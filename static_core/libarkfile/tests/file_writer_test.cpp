/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "libarkfile/file_writer.h"
#include "zip_archive.h"

#include <array>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#ifndef PANDA_TARGET_WINDOWS
#include <sys/stat.h>
#endif

#include <gtest/gtest.h>

namespace ark::panda_file::test {

static std::vector<uint8_t> Str2Bytes(std::string str)
{
    std::vector<uint8_t> vec;
    vec.assign(str.begin(), str.end());
    return vec;
}

static std::vector<uint8_t> ReadFile(const char *fileName)
{
    std::ifstream input(fileName, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

static uint32_t ReadU32(const std::vector<uint8_t> &data)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr uint32_t BYTE_MASK = 0xffU;
    static constexpr size_t BYTE_WIDTH = 8;  // CC-OFF(G.NAM.03-CPP) project code style
    uint32_t value = 0;
    for (size_t i = 0; i < sizeof(uint32_t); i++) {
        value |= (static_cast<uint32_t>(data[i]) & BYTE_MASK) << (i * BYTE_WIDTH);
    }
    return value;
}

#ifndef PANDA_TARGET_WINDOWS
static std::vector<uint8_t> ReadFifo(const char *fileName)
{
    std::vector<uint8_t> data;
    auto *file = std::fopen(fileName, "rbe");
    if (file == nullptr) {
        return data;
    }

    constexpr size_t FIFO_READ_BUFFER_SIZE = 256U;  // CC-OFF(G.NAM.03-CPP) project code style
    std::array<uint8_t, FIFO_READ_BUFFER_SIZE> buffer {};
    for (auto bytesRead = std::fread(buffer.data(), sizeof(uint8_t), buffer.size(), file); bytesRead > 0;
         bytesRead = std::fread(buffer.data(), sizeof(uint8_t), buffer.size(), file)) {
        data.insert(data.end(), buffer.begin(), buffer.begin() + bytesRead);
    }
    std::fclose(file);
    return data;
}
#endif

TEST(FileWriter, MemoryWriteByteWithCheck)
{
    const size_t chksumOffset = 0;
    std::vector<uint8_t> tdata = Str2Bytes("TestPanda");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    {
        MemoryWriter memWriter;
        ASSERT_TRUE(memWriter.Write<uint32_t>(0));
        memWriter.CountChecksum(true);
        for (auto item : tdata) {
            ASSERT_TRUE(memWriter.WriteByte(item));
        }
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        std::vector<uint8_t> rdata = memWriter.GetData();
        auto pRealChecksum = reinterpret_cast<uint32_t *>(rdata.data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
    {
        MemoryWriter memWriter;
        ASSERT_TRUE(memWriter.Write<uint32_t>(0));
        memWriter.CountChecksum(false);
        for (auto item : tdata) {
            ASSERT_TRUE(memWriter.WriteByte(item));
        }
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        std::vector<uint8_t> rdata = memWriter.GetData();
        auto pRealChecksum = reinterpret_cast<uint32_t *>(rdata.data());
        ASSERT_NE(expected, *pRealChecksum);
    }
}

TEST(FileWriter, MemoryWriteBytesWithCheck)
{
    const size_t chksumOffset = 0;
    std::vector<uint8_t> tdata = Str2Bytes("TestPanda");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    {
        MemoryWriter memWriter;
        memWriter.CountChecksum(false);
        ASSERT_TRUE(memWriter.Write<uint32_t>(0));
        ASSERT_TRUE(memWriter.WriteBytes(tdata));
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memWriter.GetData().data());
        ASSERT_NE(expected, *pRealChecksum);
    }
    {
        MemoryWriter memWriter;
        memWriter.Write<uint32_t>(0);
        memWriter.CountChecksum(true);
        ASSERT_TRUE(memWriter.WriteBytes(tdata));
        ASSERT_TRUE(memWriter.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memWriter.GetData().data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
}

TEST(FileWriter, MemoryAlignWithCheck)
{
    constexpr size_t CHKSUM_OFFSET = 0;  // CC-OFF(G.NAM.03-CPP) project code style
    constexpr uint8_t DATA = 0x7fU;      // CC-OFF(G.NAM.03-CPP) project code style
    constexpr size_t ALIGNMENT = 32U;    // CC-OFF(G.NAM.03-CPP) project code style
    std::vector<uint8_t> expectedData = {DATA};
    expectedData.resize(ALIGNMENT - sizeof(uint32_t), 0);
    uint32_t expected = adler32(1, expectedData.data(), expectedData.size());

    MemoryWriter memWriter;
    ASSERT_TRUE(memWriter.Write<uint32_t>(0));
    memWriter.CountChecksum(true);
    ASSERT_TRUE(memWriter.WriteByte(DATA));
    ASSERT_TRUE(memWriter.Align(ALIGNMENT));
    ASSERT_TRUE(memWriter.WriteChecksum(CHKSUM_OFFSET));

    auto pRealChecksum = reinterpret_cast<const uint32_t *>(memWriter.GetData().data());
    ASSERT_EQ(expected, *pRealChecksum);
}

TEST(FileWriter, MemoryBufferWriteBytesWithCheck)
{
    std::vector<uint8_t> tdata = Str2Bytes("TestBufferWriter");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    const size_t chksumOffset = 0;
    const auto bufSize = 64;
    std::vector<uint8_t> memBuf(bufSize);
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        ASSERT_TRUE(writer.WriteBytes(tdata));
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteBytes(tdata));
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_NE(expected, *pRealChecksum);
    }
}

TEST(FileWriter, MemoryBufferWriteWithCheck)
{
    std::vector<uint8_t> tdata = Str2Bytes("@#Test-Buffer-Writer!");
    uint32_t expected = adler32(1, tdata.data(), tdata.size());
    const size_t chksumOffset = 0;
    const auto bufSize = 64;
    std::vector<uint8_t> memBuf(bufSize);
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        for (auto item : tdata) {
            ASSERT_TRUE(writer.WriteByte(item));
        }
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_EQ(expected, *pRealChecksum);
    }
    {
        MemoryBufferWriter writer(memBuf.data(), memBuf.size());
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(false);
        for (size_t i = 0; i < tdata.size(); ++i) {
            if (i == tdata.size() / 2) {
                writer.CountChecksum(true);
            }
            ASSERT_TRUE(writer.WriteByte(tdata[i]));
        }
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        auto pRealChecksum = reinterpret_cast<const uint32_t *>(memBuf.data());
        ASSERT_NE(expected, *pRealChecksum);
    }
}

TEST(FileWriter, FileWriterRepeatedChecksumToggles)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr const char *FILE_NAME = "__file_writer_checksum_toggles__.abc";
    const size_t chksumOffset = 0;
    auto checkedData = Str2Bytes("CheckedData");
    auto ignoredData = Str2Bytes("IgnoredData");
    uint32_t expected = adler32(1, checkedData.data(), checkedData.size());

    {
        FileWriter writer(FILE_NAME);
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        ASSERT_TRUE(writer.WriteBytes(checkedData));
        writer.CountChecksum(true);
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteBytes(ignoredData));
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        ASSERT_TRUE(writer.FinishWrite());
    }

    auto data = ReadFile(FILE_NAME);
    ASSERT_GE(data.size(), sizeof(uint32_t));
    EXPECT_EQ(expected, ReadU32(data));
    std::remove(FILE_NAME);
}

TEST(FileWriter, FileWriterMultipleChecksumRanges)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr const char *FILE_NAME = "__file_writer_checksum_ranges__.abc";
    const size_t chksumOffset = 0;
    auto firstData = Str2Bytes("FirstData");
    auto ignoredData = Str2Bytes("IgnoredData");
    auto secondData = Str2Bytes("SecondData");
    uint32_t expected = adler32(1, firstData.data(), firstData.size());
    expected = adler32(expected, secondData.data(), secondData.size());

    {
        FileWriter writer(FILE_NAME);
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        ASSERT_TRUE(writer.WriteBytes(firstData));
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteBytes(ignoredData));
        writer.CountChecksum(true);
        ASSERT_TRUE(writer.WriteBytes(secondData));
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteChecksum(chksumOffset));
        ASSERT_TRUE(writer.FinishWrite());
    }

    auto data = ReadFile(FILE_NAME);
    ASSERT_GE(data.size(), sizeof(uint32_t));
    EXPECT_EQ(expected, ReadU32(data));
    std::remove(FILE_NAME);
}

TEST(FileWriter, FileWriterChecksumAcrossFlushBoundary)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr const char *FILE_NAME = "__file_writer_checksum_large__.abc";
    constexpr size_t CHECKSUM_OFFSET = 0;                         // CC-OFF(G.NAM.03-CPP) project code style
    constexpr size_t LARGE_DATA_SIZE = 2U * 1024U * 1024U + 17U;  // CC-OFF(G.NAM.03-CPP) project code style
    constexpr size_t IGNORED_DATA_SIZE = 1024U;                   // CC-OFF(G.NAM.03-CPP) project code style
    constexpr uint8_t IGNORED_DATA_BYTE = 0xA5U;                  // CC-OFF(G.NAM.03-CPP) project code style

    std::vector<uint8_t> checkedData(LARGE_DATA_SIZE);
    for (size_t i = 0; i < checkedData.size(); i++) {
        checkedData[i] = static_cast<uint8_t>(i);
    }
    std::vector<uint8_t> ignoredData(IGNORED_DATA_SIZE, IGNORED_DATA_BYTE);
    auto expected = adler32(1, checkedData.data(), checkedData.size());

    {
        FileWriter writer(FILE_NAME);
        ASSERT_TRUE(writer.Write<uint32_t>(0));
        writer.CountChecksum(true);
        ASSERT_TRUE(writer.WriteBytes(checkedData));
        writer.CountChecksum(false);
        ASSERT_TRUE(writer.WriteBytes(ignoredData));
        ASSERT_TRUE(writer.WriteChecksum(CHECKSUM_OFFSET));
        ASSERT_TRUE(writer.FinishWrite());
    }

    auto data = ReadFile(FILE_NAME);
    ASSERT_EQ(sizeof(uint32_t) + checkedData.size() + ignoredData.size(), data.size());
    EXPECT_EQ(expected, ReadU32(data));
    std::remove(FILE_NAME);
}

#ifndef PANDA_TARGET_WINDOWS
TEST(FileWriter, FileWriterChecksumToNonSeekableOutput)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr const char *FILE_NAME = "__file_writer_checksum_fifo__.abc";
    constexpr size_t CHECKSUM_OFFSET = 0;  // CC-OFF(G.NAM.03-CPP) project code style
    auto checkedData = Str2Bytes("CheckedData");
    auto ignoredData = Str2Bytes("IgnoredData");
    auto expected = adler32(1, checkedData.data(), checkedData.size());

    std::remove(FILE_NAME);
    ASSERT_EQ(mkfifo(FILE_NAME, S_IRUSR | S_IWUSR), 0);

    std::vector<uint8_t> data;
    std::thread reader([&data]() { data = ReadFifo(FILE_NAME); });

    bool writeOk = true;
    {
        FileWriter writer(FILE_NAME);
        writeOk = writeOk && static_cast<bool>(writer);
        writeOk = writeOk && writer.Write<uint32_t>(0);
        writer.CountChecksum(true);
        writeOk = writeOk && writer.WriteBytes(checkedData);
        writer.CountChecksum(false);
        writeOk = writeOk && writer.WriteBytes(ignoredData);
        writeOk = writeOk && writer.WriteChecksum(CHECKSUM_OFFSET);
        writeOk = writeOk && writer.FinishWrite();
    }

    reader.join();
    std::remove(FILE_NAME);

    ASSERT_TRUE(writeOk);
    ASSERT_EQ(sizeof(uint32_t) + checkedData.size() + ignoredData.size(), data.size());
    EXPECT_EQ(expected, ReadU32(data));
}
#endif

}  // namespace ark::panda_file::test
