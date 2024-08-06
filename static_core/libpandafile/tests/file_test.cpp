/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "file-inl.h"
#include "file_items.h"
#include "file_item_container.h"
#include "file_reader.h"
#include "utils/string_helpers.h"
#include "zip_archive.h"
#include "file.h"

#include "assembly-emitter.h"
#include "assembly-parser.h"

#include <cstdint>
#ifdef PANDA_TARGET_MOBILE
#include <unistd.h>
#endif

#include <vector>

#include <gtest/gtest.h>

namespace ark::panda_file::test {

static std::unique_ptr<const File> GetPandaFile(std::vector<uint8_t> *data)
{
    os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data->data()), data->size(),
                              [](std::byte *, size_t) noexcept {});
    return File::OpenFromMemory(std::move(ptr));
}

static std::vector<uint8_t> GetEmptyPandaFileBytes()
{
    pandasm::Parser p;

    auto source = R"()";

    std::string srcFilename = "src.pa";
    auto res = p.Parse(source, srcFilename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(pf != nullptr);

    std::vector<uint8_t> data {};
    const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    data.assign(headerPtr, headerPtr + sizeof(File::Header));

    ASSERT(data.size() == sizeof(File::Header));

    return data;
}

int CreateOrAddZipPandaFile(std::vector<uint8_t> *data, const char *zipArchiveName, const char *filename, int append,
                            int level)
{
    return CreateOrAddFileIntoZip(zipArchiveName, filename, data, append, level);
}

bool CheckAnonMemoryName([[maybe_unused]] const char *zipArchiveName)
{
    // check if [annon:panda-classes.abc extracted in memory from /xx/__OpenPandaFileFromZip__.zip]
#ifdef PANDA_TARGET_MOBILE
    bool result = false;
    const char *prefix = "[anon:panda-";
    int pid = getpid();
    std::stringstream ss;
    ss << "/proc/" << pid << "/maps";
    std::ifstream f;
    f.open(ss.str(), std::ios::in);
    EXPECT_TRUE(f.is_open());
    for (std::string line; std::getline(f, line);) {
        if (line.find(prefix) != std::string::npos && line.find(zipArchiveName) != std::string::npos) {
            result = true;
        }
    }
    f.close();
    return result;
#else
    return true;
#endif
}

TEST(File, OpenMemory)
{
    {
        auto data = GetEmptyPandaFileBytes();
        auto ptr = GetPandaFile(&data);
        EXPECT_NE(ptr, nullptr);
    }

    {
        auto data = GetEmptyPandaFileBytes();
        data[0] = 0x0;  // Corrupt magic

        auto ptr = GetPandaFile(&data);
        EXPECT_EQ(ptr, nullptr);
    }
}

TEST(File, GetClassByName)
{
    ItemContainer container;

    std::vector<std::string> names = {"C", "B", "A"};
    std::vector<ClassItem *> classes;
    classes.reserve(names.size());

    for (auto &name : names) {
        classes.push_back(container.GetOrCreateClassItem(name));
    }

    MemoryWriter memWriter;

    ASSERT_TRUE(container.Write(&memWriter));

    // Read panda file from memory

    auto data = memWriter.GetData();
    auto pandaFile = GetPandaFile(&data);
    ASSERT_NE(pandaFile, nullptr);

    for (size_t i = 0; i < names.size(); i++) {
        EXPECT_EQ(pandaFile->GetClassId(reinterpret_cast<const uint8_t *>(names[i].c_str())).GetOffset(),
                  classes[i]->GetOffset());
    }
}

TEST(File, OpenPandaFile)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zipFilename = "__OpenPandaFile__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "classses2.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename2, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFile(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    remove(zipFilename);
}

TEST(File, OpenPandaFileFromZipNameAnonMem)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zipFilename = "__OpenPandaFileFromZipNameAnonMem__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFile(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    ASSERT_TRUE(CheckAnonMemoryName(zipFilename));
    remove(zipFilename);
}

TEST(File, OpenPandaFileOrZip)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    int ret;
    const char *zipFilename = "__OpenPandaFileOrZip__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "classes2.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename2, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFileOrZip(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    remove(zipFilename);
}

TEST(File, OpenPandaFileUncompressed)
{
    // Create ZIP
    auto data = GetEmptyPandaFileBytes();
    std::cout << "pandafile size = " << data.size() << std::endl;
    int ret;
    const char *zipFilename = "__OpenPandaFileUncompressed__.zip";
    const char *filename1 = ARCHIVE_FILENAME;
    const char *filename2 = "class.abc";  // just for testing.
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename2, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);
    ret = CreateOrAddZipPandaFile(&data, zipFilename, filename1, APPEND_STATUS_ADDINZIP, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open from ZIP
    auto pf = OpenPandaFileOrZip(zipFilename);
    EXPECT_NE(pf, nullptr);
    EXPECT_STREQ((pf->GetFilename()).c_str(), zipFilename);
    remove(zipFilename);
}

TEST(File, LineNumberProgramDeduplication)
{
    pandasm::Parser p;

    auto source = R"delim(
        .function void foo() {
            return.void
        }

        .function void bar() {
            return.void
        }
    )delim";

    std::string srcFilename = "src.pa";
    auto res = p.Parse(source, srcFilename);
    ASSERT(p.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);

    ASSERT_EQ(res.Value().functionTable.size(), 2);
    for (auto &a : res.Value().functionTable) {
        ASSERT_TRUE(a.second.HasDebugInfo());
    }

    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(pf != nullptr);

    auto reader = FileReader(std::move(pf));

    ASSERT_TRUE(reader.ReadContainer());

    int lnpCnt = 0;

    ASSERT_NE(reader.GetItems()->size(), 0);

    for (const auto &a : *reader.GetItems()) {
        const auto typ = a.second->GetItemType();
        if (typ == ItemTypes::LINE_NUMBER_PROGRAM_ITEM) {
            lnpCnt++;
        }
    }

    reader.GetContainerPtr()->ComputeLayout();
    reader.GetContainerPtr()->DeduplicateCodeAndDebugInfo();

    ASSERT_EQ(lnpCnt, 1);
}

}  // namespace ark::panda_file::test
