/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "zip_archive.h"
#include "libarkfile/file.h"
#include "libarkbase/os/file.h"
#include "libarkbase/os/mem.h"
#include "extractortool/zip_file.h"
#include "extractortool/extractor.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

#include <cstdio>
#include <cstdint>
#include <vector>
#include <sstream>
#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <securec.h>

#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

namespace ark::test {

// Test accessor class for accessing private members of ZipFile and Extractor
class ZipFileTestAccessor {
public:
    static constexpr uint32_t MISMATCH_OFFSET_INCREMENT = 100;

    static ark::extractor::ZipFile &GetZipFile(ark::extractor::Extractor &extractor)
    {
        return extractor.zipFile_;
    }

    static void SetZipFileReaderNull(ark::extractor::ZipFile &zf)
    {
        zf.zipFileReader_ = nullptr;
    }

    static void SetFileStartPos(ark::extractor::ZipFile &zf, uint64_t pos)
    {
        zf.fileStartPos_ = pos;
    }

    static void SetEntryOffset(ark::extractor::ZipFile &zf, const std::string &name, uint32_t offset)
    {
        zf.entriesMap_[name].localHeaderOffset = offset;
    }

    static void AddFakeEntry(ark::extractor::ZipFile &zf, const std::string &name, uint32_t offset)
    {
        ark::extractor::ZipEntry entry;
        entry.localHeaderOffset = offset;
        entry.fileName = name;
        zf.entriesMap_[name] = entry;
    }

    static bool HasEntry(ark::extractor::ZipFile &zf, const std::string &name)
    {
        return zf.entriesMap_.find(name) != zf.entriesMap_.end();
    }

    static uint32_t GetEntryOffset(ark::extractor::ZipFile &zf, const std::string &name)
    {
        auto it = zf.entriesMap_.find(name);
        if (it != zf.entriesMap_.end()) {
            return it->second.localHeaderOffset;
        }
        return 0;
    }
};

#define GTEST_COUT std::cerr << "[          ] [ INFO ]"

constexpr char const *FILLER_TEXT =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras feugiat et odio ac sollicitudin. Maecenas "
    "lobortis ultrices eros sed pharetra. Phasellus in tortor rhoncus, aliquam augue ac, gravida elit. Sed "
    "molestie dolor a vulputate tincidunt. Proin a tellus quam. Suspendisse id feugiat elit, non ornare lacus. "
    "Mauris arcu ex, pretium quis dolor ut, porta iaculis eros. Vestibulum sagittis placerat diam, vitae efficitur "
    "turpis ultrices sit amet. Etiam elementum bibendum congue. In sit amet dolor ultricies, suscipit arcu ac, "
    "molestie urna. Mauris ultrices volutpat massa quis ultrices. Suspendisse rutrum lectus sit amet metus "
    "laoreet, non porta sapien venenatis. Fusce ut massa et purus elementum lacinia. Sed tempus bibendum pretium.";

constexpr size_t MAX_BUFFER_SIZE = 2048;
constexpr size_t MAX_DIR_SIZE = 64;

static void GenerateZipfile(const char *data, const char *archivename, int n, std::vector<uint8_t> &pfData,
                            int level = Z_BEST_COMPRESSION)
{
    // Delete the test archive, so it doesn't keep growing as we run this test
    remove(archivename);

    // Create and append a directory entry for testing
    std::vector<uint8_t> emptyVector;
    int ret = CreateOrAddFileIntoZip(archivename, "directory/", &emptyVector, APPEND_STATUS_CREATE, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for directory failed!";
        return;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char archiveFilename[MAX_DIR_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char buf[MAX_BUFFER_SIZE];

    // Append a bunch of text files to the test archive
    for (int i = (n - 1); i >= 0; --i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(archiveFilename, sizeof(archiveFilename), "%u.txt", i);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, sizeof(buf), "%u %s %u", (n - 1) - i, data, i);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::vector<uint8_t> fillerData(buf, buf + strlen(buf) + 1);
        ret = CreateOrAddFileIntoZip(archivename, archiveFilename, &fillerData, APPEND_STATUS_ADDINZIP, level);
        if (ret != 0) {
            ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for " << i << ".txt failed!";
            return;
        }
    }

    // Append a file into directory entry for testing
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(buf, sizeof(buf), "%u %s %u", n, data, n);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::vector<uint8_t> fillerData(buf, buf + strlen(buf) + 1);
    ret = CreateOrAddFileIntoZip(archivename, "directory/indirectory.txt", &fillerData, APPEND_STATUS_ADDINZIP, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for directory/indirectory.txt failed!";
        return;
    }

    // Add a pandafile into zip for testing
    ret = CreateOrAddFileIntoZip(archivename, "classes.abc", &pfData, APPEND_STATUS_ADDINZIP, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for classes.abc failed!";
        return;
    }
}

static void UnzipFileCheckDirectory(const char *archivename, char *filename, int level = Z_BEST_COMPRESSION)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(filename, MAX_DIR_SIZE, "directory/");

    ZipArchiveHandle zipfile = nullptr;
    FILE *myfile = fopen(archivename, "rbe");

    if (OpenArchiveFile(zipfile, myfile) != 0) {
        fclose(myfile);
        ASSERT_EQ(1, 0) << "OpenArchiveFILE error.";
        return;
    }
    if (LocateFile(zipfile, filename) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        ASSERT_EQ(1, 0) << "LocateFile error.";
        return;
    }
    EntryFileStat entry = EntryFileStat();
    if (GetCurrentFileInfo(zipfile, &entry) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        ASSERT_EQ(1, 0) << "GetCurrentFileInfo test error.";
        return;
    }
    if (OpenCurrentFile(zipfile) != 0) {
        CloseCurrentFile(zipfile);
        CloseArchiveFile(zipfile);
        fclose(myfile);
        ASSERT_EQ(1, 0) << "OpenCurrentFile test error.";
        return;
    }

    GetCurrentFileOffset(zipfile, &entry);

    uint32_t uncompressedLength = entry.GetUncompressedSize();

    ASSERT_GT(entry.GetOffset(), 0);
    if (level == Z_NO_COMPRESSION) {
        ASSERT_FALSE(entry.IsCompressed());
    } else {
        ASSERT_TRUE(entry.IsCompressed());
    }

    GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
               << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
               << "entry offset: " << entry.GetOffset() << "\n";

    CloseCurrentFile(zipfile);
    CloseArchiveFile(zipfile);
    fclose(myfile);
}

static void CloseArchiveAndCurrentFile(ZipArchiveHandle zipfile, FILE *myfile)
{
    CloseCurrentFile(zipfile);
    CloseArchiveFile(zipfile);
    (void)fclose(myfile);
}

static std::string OpenArchiveAndCurrentFile(ZipArchiveHandle &zipfile, char *filename, FILE *myfile,
                                             EntryFileStat *entry)
{
    if (OpenArchiveFile(zipfile, myfile) != 0) {
        fclose(myfile);
        return "OpenArchiveFILE error.";
    }
    if (LocateFile(zipfile, filename) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        return "LocateFile error.";
    }

    if (GetCurrentFileInfo(zipfile, entry) != 0) {
        CloseArchiveFile(zipfile);
        fclose(myfile);
        return "GetCurrentFileInfo test error.";
    }
    if (OpenCurrentFile(zipfile) != 0) {
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "OpenCurrentFile test error.";
    }

    return {};
}

static std::string ExtractFile(ZipArchiveHandle &zipfile, FILE *myfile, uint32_t uncompressedLength, char *buf)
{
    // Extract to mem buffer accroding to entry info.
    uint32_t pageSize = os::mem::GetPageSize();
    ASSERT(pageSize != 0);
    uint32_t minPages = uncompressedLength / pageSize;
    uint32_t sizeToMmap = uncompressedLength % pageSize == 0 ? minPages * pageSize : (minPages + 1) * pageSize;
    // we will use mem in memcmp, so donnot poision it!
    void *mem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't mmap anonymous!";
    }

    if (ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), sizeToMmap) != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't extract!";
    }

    // Make sure the extraction really succeeded.
    size_t dlen = strlen(buf);
    if (uncompressedLength != (dlen + 1)) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        std::ostringstream out;
        out << "ExtractToMemory() failed!, uncompressed_length is " << (uncompressedLength - 1)
            << ", original strlen is " << dlen;
        return out.str();
    }

    if (memcmp(mem, buf, dlen) != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "ExtractToMemory() memcmp failed!";
    }

    os::mem::UnmapRaw(mem, sizeToMmap);

    return {};
}

static std::string ExtractPandaFile(ZipArchiveHandle &zipfile, FILE *myfile, uint32_t uncompressedLength,
                                    std::vector<uint8_t> &pfData)
{
    // Extract to mem buffer accroding to entry info.
    uint32_t pageSize = os::mem::GetPageSize();
    ASSERT(pageSize != 0);
    uint32_t minPages = uncompressedLength / pageSize;
    uint32_t sizeToMmap = uncompressedLength % pageSize == 0 ? minPages * pageSize : (minPages + 1) * pageSize;
    // we will use mem in memcmp, so donnot poision it!
    void *mem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't mmap anonymous!";
    }

    int ret = ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), sizeToMmap);
    if (ret != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "Can't extract!";
    }

    // Make sure the extraction really succeeded.
    if (uncompressedLength != pfData.size()) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        std::ostringstream out;
        out << "ExtractToMemory() failed!, uncompressed_length is " << uncompressedLength
            << ", original pf_data size is " << pfData.size() << "\n";
        return out.str();
    }

    if (memcmp(mem, pfData.data(), pfData.size()) != 0) {
        os::mem::UnmapRaw(mem, sizeToMmap);
        CloseArchiveAndCurrentFile(zipfile, myfile);
        return "ExtractToMemory() memcmp failed!";
    }

    os::mem::UnmapRaw(mem, sizeToMmap);

    return {};
}

static void UnzipFileCheckTxt(const char *archivename, char *filename, const char *data, int n,
                              int level = Z_BEST_COMPRESSION)
{
    for (int i = 0; i < n; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(filename, MAX_DIR_SIZE, "%u.txt", i);

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        char buf[MAX_BUFFER_SIZE];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, sizeof(buf), "%u %s %u", (n - 1) - i, data, i);

        ZipArchiveHandle zipfile = nullptr;
        FILE *myfile = fopen(archivename, "rbe");
        EntryFileStat entry = EntryFileStat();

        std::string errorMessage = OpenArchiveAndCurrentFile(zipfile, filename, myfile, &entry);
        if (!errorMessage.empty()) {
            ASSERT_EQ(1, 0) << errorMessage.c_str();
        }

        GetCurrentFileOffset(zipfile, &entry);

        uint32_t uncompressedLength = entry.GetUncompressedSize();
        if (uncompressedLength == 0) {
            CloseArchiveAndCurrentFile(zipfile, myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressedLength, strlen(buf) + 1);
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressedLength, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressedLength, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }

        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            errorMessage = ExtractFile(zipfile, myfile, uncompressedLength, buf);
            if (!errorMessage.empty()) {
                ASSERT_EQ(1, 0) << errorMessage.c_str();
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressedLength << "\n";
        }

        CloseArchiveAndCurrentFile(zipfile, myfile);
    }
}

static void UnzipFileCheckPandaFile(const char *archivename, char *filename, std::vector<uint8_t> &pfData,
                                    int level = Z_BEST_COMPRESSION)
{
    {
        ZipArchiveHandle zipfile = nullptr;
        FILE *myfile = fopen(archivename, "rbe");
        EntryFileStat entry = EntryFileStat();

        std::string errorMessage = OpenArchiveAndCurrentFile(zipfile, filename, myfile, &entry);
        if (!errorMessage.empty()) {
            ASSERT_EQ(1, 0) << errorMessage.c_str();
        }

        GetCurrentFileOffset(zipfile, &entry);

        uint32_t uncompressedLength = entry.GetUncompressedSize();
        if (uncompressedLength == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressedLength, pfData.size());
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressedLength, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressedLength, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }

        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            errorMessage = ExtractPandaFile(zipfile, myfile, uncompressedLength, pfData);
            if (!errorMessage.empty()) {
                ASSERT_EQ(1, 0) << errorMessage.c_str();
            }
            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressedLength << "\n";
        }
        CloseArchiveAndCurrentFile(zipfile, myfile);
    }
}

static void UnzipFileCheckInDirectory(const char *archivename, char *filename, const char *data, int n,
                                      int level = Z_BEST_COMPRESSION)
{
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(filename, MAX_DIR_SIZE, "directory/indirectory.txt");

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        char buf[MAX_BUFFER_SIZE];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, sizeof(buf), "%u %s %u", n, data, n);

        // Unzip Check
        ZipArchiveHandle zipfile = nullptr;
        FILE *myfile = fopen(archivename, "rbe");
        EntryFileStat entry = EntryFileStat();

        std::string errorMessage = OpenArchiveAndCurrentFile(zipfile, filename, myfile, &entry);
        if (!errorMessage.empty()) {
            ASSERT_EQ(1, 0) << errorMessage.c_str();
        }

        GetCurrentFileOffset(zipfile, &entry);

        uint32_t uncompressedLength = entry.GetUncompressedSize();
        if (uncompressedLength == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressedLength, strlen(buf) + 1);
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressedLength, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressedLength, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }
        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressedLength
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            errorMessage = ExtractFile(zipfile, myfile, uncompressedLength, buf);
            if (!errorMessage.empty()) {
                ASSERT_EQ(1, 0) << errorMessage.c_str();
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressedLength << "\n";
        }

        CloseArchiveAndCurrentFile(zipfile, myfile);
    }
}

static void ModifyCentralDirEntryNameSize(const char *archivename, uint16_t overflowNameSize)
{
    FILE *fp = fopen(archivename, "rbe");
    ASSERT_NE(fp, nullptr);
    (void)fseek(fp, 0, SEEK_END);
    auto fileSize = ftell(fp);
    (void)fseek(fp, 0, SEEK_SET);
    std::vector<uint8_t> buffer(fileSize);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT_EQ(fread(buffer.data(), 1, fileSize, fp), static_cast<size_t>(fileSize));
    (void)fclose(fp);

    constexpr uint32_t EOCD_SIGNATURE = 0x06054b50;
    constexpr size_t EOCD_MIN_SIZE = 22U;
    constexpr size_t EOCD_OFFSET_OFFSET = 16U;
    constexpr size_t NAME_SIZE_OFFSET = 28U;

    size_t eocdPos = 0;
    size_t maxCommentLen = static_cast<size_t>(fileSize) - EOCD_MIN_SIZE;
    for (size_t offset = 0; offset <= maxCommentLen; ++offset) {
        size_t pos = static_cast<size_t>(fileSize) - EOCD_MIN_SIZE - offset;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*reinterpret_cast<uint32_t *>(buffer.data() + pos) == EOCD_SIGNATURE) {
            eocdPos = pos;
            break;
        }
    }
    ASSERT_GT(eocdPos, 0U);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t centralDirOffset = *reinterpret_cast<uint32_t *>(buffer.data() + eocdPos + EOCD_OFFSET_OFFSET);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *reinterpret_cast<uint16_t *>(buffer.data() + centralDirOffset + NAME_SIZE_OFFSET) = overflowNameSize;

    fp = fopen(archivename, "wbe");
    ASSERT_NE(fp, nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT_EQ(fwrite(buffer.data(), 1, fileSize, fp), static_cast<size_t>(fileSize));
    (void)fclose(fp);
}

TEST(LIBZIPARCHIVE, ZipFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    static const char *archivename = "__LIBZIPARCHIVE__ZipFile__.zip";
    const int n = 3;

    GenerateZipfile(FILLER_TEXT, archivename, n, pfData);

    // Quick Check
    ZipArchiveHandle zipfile = nullptr;
    if (OpenArchive(zipfile, archivename) != 0) {
        ASSERT_EQ(1, 0) << "OpenArchive error.";
        return;
    }

    GlobalStat gi = GlobalStat();
    if (GetGlobalFileInfo(zipfile, &gi) != 0) {
        ASSERT_EQ(1, 0) << "GetGlobalFileInfo error.";
        return;
    }
    for (int i = 0; i < static_cast<int>(gi.GetNumberOfEntry()); ++i) {
        EntryFileStat fileStat {};
        if (GetCurrentFileInfo(zipfile, &fileStat) != 0) {
            CloseArchive(zipfile);
            ASSERT_EQ(1, 0) << "GetCurrentFileInfo error. Current index i = " << i;
            return;
        }
        GTEST_COUT << "Index:  " << i << ", Uncompressed size: " << fileStat.GetUncompressedSize()
                   << "Compressed size: " << fileStat.GetCompressedSize() << "Compressed(): " << fileStat.IsCompressed()
                   << "entry offset: " << fileStat.GetOffset() << "\n";
        if ((i + 1) < static_cast<int>(gi.GetNumberOfEntry())) {
            if (GoToNextFile(zipfile) != 0) {
                CloseArchive(zipfile);
                ASSERT_EQ(1, 0) << "GoToNextFile error. Current index i = " << i;
                return;
            }
        }
    }

    CloseArchive(zipfile);
    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipFile__.zip";
    const int n = 3;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];

    GenerateZipfile(FILLER_TEXT, archivename, n, pfData);

    UnzipFileCheckDirectory(archivename, filename);

    UnzipFileCheckTxt(archivename, filename, FILLER_TEXT, n);

    UnzipFileCheckInDirectory(archivename, filename, FILLER_TEXT, n);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipUncompressedFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipUncompressedFile__.zip";
    const int n = 3;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];

    GenerateZipfile(FILLER_TEXT, archivename, n, pfData, Z_NO_COMPRESSION);

    UnzipFileCheckDirectory(archivename, filename, Z_NO_COMPRESSION);

    UnzipFileCheckTxt(archivename, filename, FILLER_TEXT, n, Z_NO_COMPRESSION);

    UnzipFileCheckInDirectory(archivename, filename, FILLER_TEXT, n, Z_NO_COMPRESSION);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData, Z_NO_COMPRESSION);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipUncompressedPandaFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipUncompressedPandaFile__.zip";
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];

    // Delete the test archive, so it doesn't keep growing as we run this test
    remove(archivename);

    // Add pandafile into zip for testing
    int ret = CreateOrAddFileIntoZip(archivename, "class.abc", &pfData, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }
    ret = CreateOrAddFileIntoZip(archivename, "classes.abc", &pfData, APPEND_STATUS_ADDINZIP, Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "class.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData, Z_NO_COMPRESSION);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pfData, Z_NO_COMPRESSION);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, IllegalPathTest)
{
    static const char *archivename = "__LIBZIPARCHIVE__ILLEGALPATHTEST__.zip";
    int ret =
        CreateOrAddFileIntoZip(archivename, "illegal_path.abc", nullptr, APPEND_STATUS_ADDINZIP, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, ZIPARCHIVE_ERR);
    ZipArchiveHandle zipfile = nullptr;
    ASSERT_EQ(OpenArchive(zipfile, archivename), ZIPARCHIVE_ERR);
    GlobalStat gi = GlobalStat();
    ASSERT_EQ(GetGlobalFileInfo(zipfile, &gi), ZIPARCHIVE_ERR);
    ASSERT_EQ(GoToNextFile(zipfile), ZIPARCHIVE_ERR);
    ASSERT_EQ(LocateFile(zipfile, "illegal_path.abc"), ZIPARCHIVE_ERR);
    EntryFileStat entry = EntryFileStat();
    ASSERT_EQ(GetCurrentFileInfo(zipfile, &entry), ZIPARCHIVE_ERR);
    ASSERT_EQ(CloseArchive(zipfile), ZIPARCHIVE_ERR);

    ASSERT_EQ(OpenArchiveFile(zipfile, nullptr), ZIPARCHIVE_ERR);
    ASSERT_EQ(OpenCurrentFile(zipfile), ZIPARCHIVE_ERR);
    ASSERT_EQ(CloseCurrentFile(zipfile), ZIPARCHIVE_ERR);
    ASSERT_EQ(CloseArchiveFile(zipfile), ZIPARCHIVE_ERR);
}

TEST(LIBZIPARCHIVE, ZipFileParseAllEntriesOverflowTest)
{
    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;
        auto res = p.Parse(R"()", "src.pa");
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    static const char *archivename = "__LIBZIPARCHIVE__ZipFileParseOverflowTest__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 1, pfData, Z_NO_COMPRESSION);
    ModifyCentralDirEntryNameSize(archivename, UINT16_MAX);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_FALSE(zipFile.Open()) << "ZipFile::Open should fail for corrupted central directory entry";
    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: IsEntryDataConsistent
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test IsEntryDataConsistent with entry not in entriesMap
 */
TEST(LIBZIPARCHIVE, IsEntryDataConsistent_001)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__CONSISTENCY_TEST_001__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    auto result = zipFile.IsEntryDataConsistent("not_exist_entry");
    EXPECT_EQ(result, ark::extractor::ConsistencyResult::READ_ERROR);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: IsEntryDataConsistent
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test IsEntryDataConsistent with zipFileReader nullptr
 */
TEST(LIBZIPARCHIVE, IsEntryDataConsistent_002)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__CONSISTENCY_TEST_002__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    ZipFileTestAccessor::SetZipFileReaderNull(zipFile);
    auto result = zipFile.IsEntryDataConsistent("classes.abc");
    EXPECT_EQ(result, ark::extractor::ConsistencyResult::READ_ERROR);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: IsEntryDataConsistent
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test IsEntryDataConsistent with real entry and offset set to 0
 */
TEST(LIBZIPARCHIVE, IsEntryDataConsistent_003)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__CONSISTENCY_TEST_003__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    ZipFileTestAccessor::SetEntryOffset(zipFile, "classes.abc", 0);
    auto result = zipFile.IsEntryDataConsistent("classes.abc");
    EXPECT_EQ(result, ark::extractor::ConsistencyResult::CONSISTENT);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: IsEntryDataConsistent
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test IsEntryDataConsistent with fake entry name using real offset
 */
TEST(LIBZIPARCHIVE, IsEntryDataConsistent_004)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__CONSISTENCY_TEST_004__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    // Get the offset of classes.abc
    uint32_t realOffset = ZipFileTestAccessor::GetEntryOffset(zipFile, "classes.abc");
    // Add fake entry with same offset
    ZipFileTestAccessor::AddFakeEntry(zipFile, "fake_entry_name.abc", realOffset);
    auto result = zipFile.IsEntryDataConsistent("fake_entry_name.abc");
    EXPECT_EQ(result, ark::extractor::ConsistencyResult::ENTRY_NOT_FOUND);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: IsEntryDataConsistent
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test IsEntryDataConsistent with real entry but modified offset
 */
TEST(LIBZIPARCHIVE, IsEntryDataConsistent_005)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__CONSISTENCY_TEST_005__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    // Modify offset to cause mismatch
    uint32_t mismatchOffset =
        ZipFileTestAccessor::GetEntryOffset(zipFile, "classes.abc") + ZipFileTestAccessor::MISMATCH_OFFSET_INCREMENT;
    ZipFileTestAccessor::SetEntryOffset(zipFile, "classes.abc", mismatchOffset);
    auto result = zipFile.IsEntryDataConsistent("classes.abc");
    EXPECT_EQ(result, ark::extractor::ConsistencyResult::OFFSET_MISMATCH);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: IsEntryDataConsistent
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test IsEntryDataConsistent with real entry and correct offset
 */
TEST(LIBZIPARCHIVE, IsEntryDataConsistent_006)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__CONSISTENCY_TEST_006__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    auto result = zipFile.IsEntryDataConsistent("classes.abc");
    EXPECT_EQ(result, ark::extractor::ConsistencyResult::CONSISTENT);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: ReadLocalHeaderName
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test ReadLocalHeaderName with fileStartPos overflow
 */
TEST(LIBZIPARCHIVE, ReadLocalHeaderName_001)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__READLOCALHEADER_TEST_001__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    // Set fileStartPos to UINT64_MAX to cause overflow when adding offset
    ZipFileTestAccessor::SetFileStartPos(zipFile, UINT64_MAX);
    // Test with fileStartPos overflow
    ark::extractor::LocalHeader header;
    std::string name;
    bool result = zipFile.ReadLocalHeaderName(0, header, name);
    EXPECT_FALSE(result);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: ReadLocalHeaderName
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test ReadLocalHeaderName with zipFileReader nullptr
 */
TEST(LIBZIPARCHIVE, ReadLocalHeaderName_002)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__READLOCALHEADER_TEST_002__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    ZipFileTestAccessor::SetZipFileReaderNull(zipFile);
    ark::extractor::LocalHeader header;
    std::string name;
    bool result = zipFile.ReadLocalHeaderName(0, header, name);
    EXPECT_FALSE(result);

    (void)remove(archivename);
}

/*
 * Feature: ZipFile
 * Function: ReadLocalHeaderName
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test ReadLocalHeaderName with valid entry at offset 0
 */
TEST(LIBZIPARCHIVE, ReadLocalHeaderName_003)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__READLOCALHEADER_TEST_003__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::ZipFile zipFile(archivename);
    ASSERT_TRUE(zipFile.Open());
    ark::extractor::LocalHeader header;
    std::string name;
    bool result = zipFile.ReadLocalHeaderName(0, header, name);
    EXPECT_TRUE(result);
    // First entry should be "directory/" based on GenerateZipfile
    EXPECT_EQ(name, "directory/");

    (void)remove(archivename);
}

/*
 * Feature: Extractor
 * Function: GetSafeData
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test GetSafeData with non-abc file extension
 */
TEST(LIBZIPARCHIVE, GetSafeData_001)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__GETSAFEDATA_TEST_001__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::Extractor extractor(archivename);
    ASSERT_TRUE(extractor.Init());
    auto result = extractor.GetSafeData("module.json");
    EXPECT_EQ(result, nullptr);

    (void)remove(archivename);
}

/*
 * Feature: Extractor
 * Function: GetSafeData
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test GetSafeData with abc file but entry not in map
 */
TEST(LIBZIPARCHIVE, GetSafeData_002)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__GETSAFEDATA_TEST_002__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::Extractor extractor(archivename);
    ASSERT_TRUE(extractor.Init());
    auto result = extractor.GetSafeData("not_exist.abc");
    EXPECT_EQ(result, nullptr);

    (void)remove(archivename);
}

/*
 * Feature: Extractor
 * Function: GetSafeData
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test GetSafeData with abc file but zipFileReader nullptr
 */
TEST(LIBZIPARCHIVE, GetSafeData_003)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__GETSAFEDATA_TEST_003__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::Extractor extractor(archivename);
    ASSERT_TRUE(extractor.Init());
    auto &zipFile = ZipFileTestAccessor::GetZipFile(extractor);
    ZipFileTestAccessor::SetZipFileReaderNull(zipFile);
    auto result = extractor.GetSafeData("/data/storage/el1/bundle/com.example/classes.abc");
    EXPECT_EQ(result, nullptr);

    (void)remove(archivename);
}

/*
 * Feature: Extractor
 * Function: GetSafeData
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test GetSafeData with abc file and offset mismatch
 */
TEST(LIBZIPARCHIVE, GetSafeData_004)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__GETSAFEDATA_TEST_004__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::Extractor extractor(archivename);
    ASSERT_TRUE(extractor.Init());
    auto &zipFile = ZipFileTestAccessor::GetZipFile(extractor);
    uint32_t mismatchOffset =
        ZipFileTestAccessor::GetEntryOffset(zipFile, "classes.abc") + ZipFileTestAccessor::MISMATCH_OFFSET_INCREMENT;
    ZipFileTestAccessor::SetEntryOffset(zipFile, "classes.abc", mismatchOffset);
    auto result = extractor.GetSafeData("/data/storage/el1/bundle/com.example/classes.abc");
    EXPECT_EQ(result, nullptr);

    (void)remove(archivename);
}

/*
 * Feature: Extractor
 * Function: GetSafeData
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test GetSafeData with abc file and consistent result
 */
TEST(LIBZIPARCHIVE, GetSafeData_005)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__GETSAFEDATA_TEST_005__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::Extractor extractor(archivename);
    ASSERT_TRUE(extractor.Init());
    auto result = extractor.GetSafeData("/data/storage/el1/bundle/com.example/classes.abc");
    EXPECT_NE(result, nullptr);

    (void)remove(archivename);
}

/*
 * Feature: Extractor
 * Function: GetSafeData
 * SubFunction: NA
 * EnvConditions: NA
 * CaseDescription: Test GetSafeData with fake entry name using real offset
 */
TEST(LIBZIPARCHIVE, GetSafeData_006)
{
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser p;
        auto source = R"()";
        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + sizeof(panda_file::File::Header));
    }

    const char *archivename = "__GETSAFEDATA_TEST_006__.zip";
    GenerateZipfile(FILLER_TEXT, archivename, 3, pfData);

    ark::extractor::Extractor extractor(archivename);
    ASSERT_TRUE(extractor.Init());
    auto &zipFile = ZipFileTestAccessor::GetZipFile(extractor);
    // Get the offset of classes.abc using accessor
    uint32_t realOffset = ZipFileTestAccessor::GetEntryOffset(zipFile, "classes.abc");
    // Add fake entry with same offset
    ZipFileTestAccessor::AddFakeEntry(zipFile, "fake_entry_name.abc", realOffset);
    auto result = extractor.GetSafeData("/data/storage/el1/bundle/com.example/fake_entry_name.abc");
    EXPECT_EQ(result, nullptr);

    (void)remove(archivename);
}
}  // namespace ark::test
