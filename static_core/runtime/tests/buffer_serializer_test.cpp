/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include "libarkbase/macros.h"
#include "runtime/include/tooling/buffer_serializer.h"

namespace ark::tooling::test {

class BufferSerializerTest : public testing::Test {
public:
    BufferSerializerTest() = default;
    ~BufferSerializerTest() override = default;

    NO_COPY_SEMANTIC(BufferSerializerTest);
    NO_MOVE_SEMANTIC(BufferSerializerTest);

protected:
    static constexpr size_t bufferSize = 256;
    std::array<uint8_t, bufferSize> buffer_ {};

    // Serialize a valid frame and return bytes written.
    size_t SerializeValidFrame(const BufferSerializer::PluginFrameData &frame)
    {
        return BufferSerializer::SerializePluginFrameData(frame, buffer_.data(), buffer_.size());
    }
};

TEST_F(BufferSerializerTest, RoundTrip)
{
    // 42, -7 : test value
    BufferSerializer::PluginFrameData in {"funcName", "moduleName", "file://app.abc", 42, -7};
    size_t written = SerializeValidFrame(in);
    ASSERT_GT(written, 0U);

    auto out = BufferSerializer::ReadPluginFrameData(buffer_.data(), written);
    EXPECT_EQ(out.functionName, "funcName");
    EXPECT_EQ(out.moduleName, "moduleName");
    EXPECT_EQ(out.url, "file://app.abc");
    // 42: test value
    EXPECT_EQ(out.lineNumber, 42);
    // -7: test value
    EXPECT_EQ(out.columnNumber, -7);
}

TEST_F(BufferSerializerTest, RoundTripEmptyStrings)
{
    BufferSerializer::PluginFrameData in {"", "", "", 0, 0};
    size_t written = SerializeValidFrame(in);
    // 3 length prefixes (4 bytes each) + 2 int32 = 20 bytes
    ASSERT_EQ(written, 20U);

    auto out = BufferSerializer::ReadPluginFrameData(buffer_.data(), written);
    EXPECT_TRUE(out.functionName.empty());
    EXPECT_TRUE(out.moduleName.empty());
    EXPECT_TRUE(out.url.empty());
    EXPECT_EQ(out.lineNumber, 0);
    EXPECT_EQ(out.columnNumber, 0);
}

TEST_F(BufferSerializerTest, SerializeIntoTooSmallBufferFails)
{
    // 1, 2 for test
    BufferSerializer::PluginFrameData in {"funcName", "moduleName", "url", 1, 2};
    // 4: test space size
    std::array<uint8_t, 4> tiny {};
    // Only room for the first length prefix, not the string data.
    size_t written = BufferSerializer::SerializePluginFrameData(in, tiny.data(), tiny.size());
    EXPECT_EQ(written, 0U);
}

TEST_F(BufferSerializerTest, SerializeExactFitTruncatesString)
{
    BufferSerializer::PluginFrameData in {"abcdef", "", "", 0, 0};
    // Room for length prefix + 3 of 6 chars only; WriteStringToBuffer truncates.
    // 7: less then chars
    std::array<uint8_t, 7> small {};
    size_t written = BufferSerializer::WriteStringToBuffer(in.functionName, small.data(), small.size());
    EXPECT_EQ(written, 7U);
    size_t offset = 0;
    auto str = BufferSerializer::ReadStringFromBuffer(small.data(), small.size(), offset);
    EXPECT_EQ(str, "abc");
}

TEST_F(BufferSerializerTest, ReadTruncatedLengthPrefix)
{
    // Fewer than sizeof(uint32_t) bytes: cannot even read the length.
    // 3 less then (sizeof(uint32_t) = 4)
    std::array<uint8_t, 3> buf {0x01, 0x00, 0x00};
    size_t offset = 0;
    auto str = BufferSerializer::ReadStringFromBuffer(buf.data(), buf.size(), offset);
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadHugeAttackerCraftedStringLength)
{
    // strLen = 0xFFFFFFFF with almost no payload: must not cause an out-of-bounds read.
    // 5 test size
    std::array<uint8_t, 5> buf {0xFF, 0xFF, 0xFF, 0xFF, 'A'};
    size_t offset = 0;
    auto str = BufferSerializer::ReadStringFromBuffer(buf.data(), buf.size(), offset);
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadStringLengthOverflowingBuffer)
{
    // strLen = 6 but only 2 payload bytes remain.
    // 0x06, 0x00, 0x00, 0x00 for test
    std::array<uint8_t, 6> buf {0x06, 0x00, 0x00, 0x00, 'A', 'B'};
    size_t offset = 0;
    auto str = BufferSerializer::ReadStringFromBuffer(buf.data(), buf.size(), offset);
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadStringExactFitBoundary)
{
    // strLen = 2 with exactly 2 payload bytes: must succeed.
    // 6 test size
    std::array<uint8_t, 6> buf {0x02, 0x00, 0x00, 0x00, 'A', 'B'};
    size_t offset = 0;
    auto str = BufferSerializer::ReadStringFromBuffer(buf.data(), buf.size(), offset);
    EXPECT_EQ(str, "AB");
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadTruncatedPODReturnsZero)
{
    // Valid string, but no room for the following int32.
    // 6 for test
    std::array<uint8_t, 6> buf {0x02, 0x00, 0x00, 0x00, 'A', 'B'};
    size_t offset = 0;
    auto str = BufferSerializer::ReadStringFromBuffer(buf.data(), buf.size(), offset);
    ASSERT_EQ(str, "AB");
    auto value = BufferSerializer::ReadPODFromBuffer<int32_t>(buf.data(), buf.size(), offset);
    EXPECT_EQ(value, 0);
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadPODWithCorruptedOffsetDoesNotWrap)
{
    // offset > bufferSize: the bounds check must not wrap around and read out of bounds.
    // 8: bufferSize
    std::array<uint8_t, 8> buf {};
    size_t offset = std::numeric_limits<size_t>::max() - 1;
    auto value = BufferSerializer::ReadPODFromBuffer<uint32_t>(buf.data(), buf.size(), offset);
    EXPECT_EQ(value, 0U);
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadStringWithCorruptedOffsetDoesNotWrap)
{
    // offset > bufferSize: reading the length prefix must fail safely.
    // 8: bufferSize
    std::array<uint8_t, 8> buf {};
    size_t offset = std::numeric_limits<size_t>::max() - 1;
    auto str = BufferSerializer::ReadStringFromBuffer(buf.data(), buf.size(), offset);
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(offset, buf.size());
}

TEST_F(BufferSerializerTest, ReadWholeFrameFromTruncatedBuffer)
{
    BufferSerializer::PluginFrameData in {"funcName", "moduleName", "file://app.abc", 42, -7};
    size_t written = SerializeValidFrame(in);
    ASSERT_GT(written, 0U);

    // Cut the buffer in the middle of the url field: later fields must degrade safely.
    // 4: test number
    size_t cutAt = 4 + std::string_view("funcName").size() + 4 + std::string_view("moduleName").size() + 2;
    auto out = BufferSerializer::ReadPluginFrameData(buffer_.data(), cutAt);
    EXPECT_EQ(out.functionName, "funcName");
    EXPECT_EQ(out.moduleName, "moduleName");
    EXPECT_TRUE(out.url.empty());
    EXPECT_EQ(out.lineNumber, 0);
    EXPECT_EQ(out.columnNumber, 0);
}

}  // namespace ark::tooling::test
