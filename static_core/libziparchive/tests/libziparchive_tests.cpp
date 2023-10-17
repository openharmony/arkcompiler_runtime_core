/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "libpandafile/file.h"
#include "os/file.h"
#include "os/mem.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

#include <cstdio>
#include <cstdint>
#include <vector>
#include <gtest/gtest.h>
#include <cstddef>
#include <memory>
#include <securec.h>

#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

namespace panda::test {

#define GTEST_COUT std::cerr << "[          ] [ INFO ]"

constexpr size_t MAX_BUFFER_SIZE = 2048;
constexpr size_t MAX_DIR_SIZE = 64;

static void GenerateZipfile(const char *data, const char *archivename, int n, char *buf, char *archive_filename, int &i,
                            int &ret, std::vector<uint8_t> &pf_data, int level = Z_BEST_COMPRESSION)
{
    // Delete the test archive, so it doesn't keep growing as we run this test
    remove(archivename);

    // Create and append a directory entry for testing
    ret = CreateOrAddFileIntoZip(archivename, "directory/", nullptr, 0, APPEND_STATUS_CREATE, level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for directory failed!";
        return;
    }

    // Append a bunch of text files to the test archive
    for (i = (n - 1); i >= 0; --i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(archive_filename, MAX_DIR_SIZE, "%u.txt", i);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, MAX_BUFFER_SIZE, "%u %s %u", (n - 1) - i, data, i);
        ret =
            CreateOrAddFileIntoZip(archivename, archive_filename, buf, strlen(buf) + 1, APPEND_STATUS_ADDINZIP, level);
        if (ret != 0) {
            ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for " << i << ".txt failed!";
            return;
        }
    }

    // Append a file into directory entry for testing
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(buf, MAX_BUFFER_SIZE, "%u %s %u", n, data, n);
    ret = CreateOrAddFileIntoZip(archivename, "directory/indirectory.txt", buf, strlen(buf) + 1, APPEND_STATUS_ADDINZIP,
                                 level);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip for directory/indirectory.txt failed!";
        return;
    }

    // Add a pandafile into zip for testing
    ret = CreateOrAddFileIntoZip(archivename, "classes.abc", pf_data.data(), pf_data.size(), APPEND_STATUS_ADDINZIP,
                                 level);
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

    uint32_t uncompressed_length = entry.GetUncompressedSize();

    ASSERT_GT(entry.GetOffset(), 0);
    if (level == Z_NO_COMPRESSION) {
        ASSERT_FALSE(entry.IsCompressed());
    } else {
        ASSERT_TRUE(entry.IsCompressed());
    }

    GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressed_length
               << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
               << "entry offset: " << entry.GetOffset() << "\n";

    CloseCurrentFile(zipfile);
    CloseArchiveFile(zipfile);
    fclose(myfile);
}

static void UnzipFileCheckTxt(const char *archivename, char *filename, const char *data, int n, char *buf, int &ret,
                              int level = Z_BEST_COMPRESSION)
{
    for (int i = 0; i < n; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(filename, MAX_DIR_SIZE, "%u.txt", i);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, MAX_BUFFER_SIZE, "%u %s %u", (n - 1) - i, data, i);

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

        uint32_t uncompressed_length = entry.GetUncompressedSize();
        if (uncompressed_length == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressed_length, strlen(buf) + 1);
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressed_length, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressed_length, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }

        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressed_length
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            // Extract to mem buffer accroding to entry info.
            uint32_t page_size = os::mem::GetPageSize();
            uint32_t min_pages = uncompressed_length / page_size;
            uint32_t size_to_mmap =
                uncompressed_length % page_size == 0 ? min_pages * page_size : (min_pages + 1) * page_size;
            // we will use mem in memcmp, so donnot poision it!
            void *mem = os::mem::MapRWAnonymousRaw(size_to_mmap, false);
            if (mem == nullptr) {
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "Can't mmap anonymous!";
                return;
            }

            ret = ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), size_to_mmap);
            if (ret != 0) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "Can't extract!";
                return;
            }

            // Make sure the extraction really succeeded.
            size_t dlen = strlen(buf);
            if (uncompressed_length != (dlen + 1)) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchive(zipfile);
                ASSERT_EQ(1, 0) << "ExtractToMemory() failed!, uncompressed_length is " << uncompressed_length - 1
                                << ", original strlen is " << dlen;
                return;
            }

            if (memcmp(mem, buf, dlen) != 0) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchive(zipfile);
                ASSERT_EQ(1, 0) << "ExtractToMemory() memcmp failed!";
                return;
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressed_length << "\n";
            os::mem::UnmapRaw(mem, size_to_mmap);
        }

        CloseCurrentFile(zipfile);
        CloseArchiveFile(zipfile);
        fclose(myfile);
    }
}

static void UnzipFileCheckPandaFile(const char *archivename, char *filename, std::vector<uint8_t> &pf_data, int &ret,
                                    int level = Z_BEST_COMPRESSION)
{
    {
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

        uint32_t uncompressed_length = entry.GetUncompressedSize();
        if (uncompressed_length == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressed_length, pf_data.size());
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressed_length, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressed_length, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }

        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressed_length
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            // Extract to mem buffer accroding to entry info.
            uint32_t page_size = os::mem::GetPageSize();
            uint32_t min_pages = uncompressed_length / page_size;
            uint32_t size_to_mmap =
                uncompressed_length % page_size == 0 ? min_pages * page_size : (min_pages + 1) * page_size;
            // we will use mem in memcmp, so donnot poision it!
            void *mem = os::mem::MapRWAnonymousRaw(size_to_mmap, false);
            if (mem == nullptr) {
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "Can't mmap anonymous!";
                return;
            }

            ret = ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), size_to_mmap);
            if (ret != 0) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "Can't extract!";
                return;
            }

            // Make sure the extraction really succeeded.
            if (uncompressed_length != pf_data.size()) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "ExtractToMemory() failed!, uncompressed_length is " << uncompressed_length
                                << ", original pf_data size is " << pf_data.size() << "\n";
                return;
            }

            if (memcmp(mem, pf_data.data(), pf_data.size()) != 0) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "ExtractToMemory() memcmp failed!";
                return;
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressed_length << "\n";

            os::mem::UnmapRaw(mem, size_to_mmap);
        }
        CloseCurrentFile(zipfile);
        CloseArchiveFile(zipfile);
        fclose(myfile);
    }
}

static void UnzipFileCheckInDirectory(const char *archivename, char *filename, const char *data, int n, char *buf,
                                      int &ret, int level = Z_BEST_COMPRESSION)
{
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(filename, MAX_DIR_SIZE, "directory/indirectory.txt");
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        (void)sprintf_s(buf, MAX_BUFFER_SIZE, "%u %s %u", n, data, n);

        // Unzip Check
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

        uint32_t uncompressed_length = entry.GetUncompressedSize();
        if (uncompressed_length == 0) {
            CloseCurrentFile(zipfile);
            CloseArchiveFile(zipfile);
            fclose(myfile);
            ASSERT_EQ(1, 0) << "Entry file has zero length! Readed bad data!";
            return;
        }
        ASSERT_GT(entry.GetOffset(), 0);
        ASSERT_EQ(uncompressed_length, strlen(buf) + 1);
        if (level == Z_NO_COMPRESSION) {
            ASSERT_EQ(uncompressed_length, entry.GetCompressedSize());
            ASSERT_FALSE(entry.IsCompressed());
        } else {
            ASSERT_GE(uncompressed_length, entry.GetCompressedSize());
            ASSERT_TRUE(entry.IsCompressed());
        }
        GTEST_COUT << "Filename: " << filename << ", Uncompressed size: " << uncompressed_length
                   << "Compressed size: " << entry.GetCompressedSize() << "Compressed(): " << entry.IsCompressed()
                   << "entry offset: " << entry.GetOffset() << "\n";

        {
            // Extract to mem buffer accroding to entry info.
            uint32_t page_size = os::mem::GetPageSize();
            uint32_t min_pages = uncompressed_length / page_size;
            uint32_t size_to_mmap =
                uncompressed_length % page_size == 0 ? min_pages * page_size : (min_pages + 1) * page_size;
            // we will use mem in memcmp, so donnot poision it!
            void *mem = os::mem::MapRWAnonymousRaw(size_to_mmap, false);
            if (mem == nullptr) {
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "Can't mmap anonymous!";
                return;
            }

            ret = ExtractToMemory(zipfile, reinterpret_cast<uint8_t *>(mem), size_to_mmap);
            if (ret != 0) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchiveFile(zipfile);
                fclose(myfile);
                ASSERT_EQ(1, 0) << "Can't extract!";
                return;
            }

            // Make sure the extraction really succeeded.
            size_t dlen = strlen(buf);
            if (uncompressed_length != (dlen + 1)) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchive(zipfile);
                ASSERT_EQ(1, 0) << "ExtractToMemory() failed!, uncompressed_length is " << uncompressed_length - 1
                                << ", original strlen is " << dlen;
                return;
            }

            if (memcmp(mem, buf, dlen) != 0) {
                os::mem::UnmapRaw(mem, size_to_mmap);
                CloseCurrentFile(zipfile);
                CloseArchive(zipfile);
                ASSERT_EQ(1, 0) << "ExtractToMemory() memcmp failed!";
                return;
            }

            GTEST_COUT << "Successfully extracted file " << filename << " from " << archivename << ", size is "
                       << uncompressed_length << "\n";

            os::mem::UnmapRaw(mem, size_to_mmap);
        }

        CloseCurrentFile(zipfile);
        CloseArchiveFile(zipfile);
        fclose(myfile);
    }
}

TEST(LIBZIPARCHIVE, ZipFile)
{
    static const char *data =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras feugiat et odio ac sollicitudin. Maecenas "
        "lobortis ultrices eros sed pharetra. Phasellus in tortor rhoncus, aliquam augue ac, gravida elit. Sed "
        "molestie dolor a vulputate tincidunt. Proin a tellus quam. Suspendisse id feugiat elit, non ornare lacus. "
        "Mauris arcu ex, pretium quis dolor ut, porta iaculis eros. Vestibulum sagittis placerat diam, vitae efficitur "
        "turpis ultrices sit amet. Etiam elementum bibendum congue. In sit amet dolor ultricies, suscipit arcu ac, "
        "molestie urna. Mauris ultrices volutpat massa quis ultrices. Suspendisse rutrum lectus sit amet metus "
        "laoreet, non porta sapien venenatis. Fusce ut massa et purus elementum lacinia. Sed tempus bibendum pretium.";

    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pf_data {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string src_filename = "src.pa";
        auto res = p.Parse(source, src_filename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto header_ptr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pf_data.assign(header_ptr, header_ptr + sizeof(panda_file::File::Header));
    }

    static const char *archivename = "__LIBZIPARCHIVE__ZipFile__.zip";
    const int n = 3;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char buf[MAX_BUFFER_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char archive_filename[MAX_DIR_SIZE];
    int i = 0;
    int ret = 0;

    GenerateZipfile(data, archivename, n, buf, archive_filename, i, ret, pf_data);

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
    for (i = 0; i < (int)gi.GetNumberOfEntry(); ++i) {
        EntryFileStat file_stat {};
        if (GetCurrentFileInfo(zipfile, &file_stat) != 0) {
            CloseArchive(zipfile);
            ASSERT_EQ(1, 0) << "GetCurrentFileInfo error. Current index i = " << i;
            return;
        }
        GTEST_COUT << "Index:  " << i << ", Uncompressed size: " << file_stat.GetUncompressedSize()
                   << "Compressed size: " << file_stat.GetCompressedSize()
                   << "Compressed(): " << file_stat.IsCompressed() << "entry offset: " << file_stat.GetOffset() << "\n";
        if ((i + 1) < (int)gi.GetNumberOfEntry()) {
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
    static const char *data =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras feugiat et odio ac sollicitudin. Maecenas "
        "lobortis ultrices eros sed pharetra. Phasellus in tortor rhoncus, aliquam augue ac, gravida elit. Sed "
        "molestie dolor a vulputate tincidunt. Proin a tellus quam. Suspendisse id feugiat elit, non ornare lacus. "
        "Mauris arcu ex, pretium quis dolor ut, porta iaculis eros. Vestibulum sagittis placerat diam, vitae efficitur "
        "turpis ultrices sit amet. Etiam elementum bibendum congue. In sit amet dolor ultricies, suscipit arcu ac, "
        "molestie urna. Mauris ultrices volutpat massa quis ultrices. Suspendisse rutrum lectus sit amet metus "
        "laoreet, non porta sapien venenatis. Fusce ut massa et purus elementum lacinia. Sed tempus bibendum pretium.";

    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pf_data {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string src_filename = "src.pa";
        auto res = p.Parse(source, src_filename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto header_ptr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pf_data.assign(header_ptr, header_ptr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipFile__.zip";
    const int n = 3;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char buf[MAX_BUFFER_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char archive_filename[MAX_DIR_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];
    int i = 0;
    int ret = 0;

    GenerateZipfile(data, archivename, n, buf, archive_filename, i, ret, pf_data);

    UnzipFileCheckDirectory(archivename, filename);

    UnzipFileCheckTxt(archivename, filename, data, n, buf, ret);

    UnzipFileCheckInDirectory(archivename, filename, data, n, buf, ret);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pf_data, ret);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipUncompressedFile)
{
    static const char *data =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras feugiat et odio ac sollicitudin. Maecenas "
        "lobortis ultrices eros sed pharetra. Phasellus in tortor rhoncus, aliquam augue ac, gravida elit. Sed "
        "molestie dolor a vulputate tincidunt. Proin a tellus quam. Suspendisse id feugiat elit, non ornare lacus. "
        "Mauris arcu ex, pretium quis dolor ut, porta iaculis eros. Vestibulum sagittis placerat diam, vitae efficitur "
        "turpis ultrices sit amet. Etiam elementum bibendum congue. In sit amet dolor ultricies, suscipit arcu ac, "
        "molestie urna. Mauris ultrices volutpat massa quis ultrices. Suspendisse rutrum lectus sit amet metus "
        "laoreet, non porta sapien venenatis. Fusce ut massa et purus elementum lacinia. Sed tempus bibendum pretium.";

    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pf_data {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string src_filename = "src.pa";
        auto res = p.Parse(source, src_filename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto header_ptr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pf_data.assign(header_ptr, header_ptr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipUncompressedFile__.zip";
    const int n = 3;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char buf[MAX_BUFFER_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char archive_filename[MAX_DIR_SIZE];
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];
    int i = 0;
    int ret = 0;

    GenerateZipfile(data, archivename, n, buf, archive_filename, i, ret, pf_data, Z_NO_COMPRESSION);

    UnzipFileCheckDirectory(archivename, filename, Z_NO_COMPRESSION);

    UnzipFileCheckTxt(archivename, filename, data, n, buf, ret, Z_NO_COMPRESSION);

    UnzipFileCheckInDirectory(archivename, filename, data, n, buf, ret, Z_NO_COMPRESSION);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pf_data, ret, Z_NO_COMPRESSION);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}

TEST(LIBZIPARCHIVE, UnZipUncompressedPandaFile)
{
    /*
     * creating an empty pandafile
     */
    std::vector<uint8_t> pf_data {};
    {
        pandasm::Parser p;

        auto source = R"()";

        std::string src_filename = "src.pa";
        auto res = p.Parse(source, src_filename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto header_ptr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pf_data.assign(header_ptr, header_ptr + sizeof(panda_file::File::Header));
    }

    // The zip filename
    static const char *archivename = "__LIBZIPARCHIVE__UnZipUncompressedPandaFile__.zip";
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char filename[MAX_DIR_SIZE];
    int ret = 0;

    // Delete the test archive, so it doesn't keep growing as we run this test
    remove(archivename);

    // Add pandafile into zip for testing
    ret = CreateOrAddFileIntoZip(archivename, "class.abc", pf_data.data(), pf_data.size(), APPEND_STATUS_CREATE,
                                 Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }
    ret = CreateOrAddFileIntoZip(archivename, "classes.abc", pf_data.data(), pf_data.size(), APPEND_STATUS_ADDINZIP,
                                 Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "class.abc");
    UnzipFileCheckPandaFile(archivename, filename, pf_data, ret, Z_NO_COMPRESSION);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    sprintf_s(filename, MAX_DIR_SIZE, "classes.abc");
    UnzipFileCheckPandaFile(archivename, filename, pf_data, ret, Z_NO_COMPRESSION);

    remove(archivename);
    GTEST_COUT << "Success.\n";
}
}  // namespace panda::test
