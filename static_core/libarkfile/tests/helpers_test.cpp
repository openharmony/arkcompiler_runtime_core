/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "libarkfile/helpers.h"

#include <cstdint>

#include <functional>
#include <vector>

#include <gtest/gtest.h>

namespace ark::panda_file::test {

TEST(Helpers, ReadValues)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    Span<const uint8_t> sp(data.data(), data.size());

    EXPECT_EQ(helpers::Read<1>(&sp), 0x01U);
    EXPECT_EQ(sp.Size(), 6U);

    EXPECT_EQ(helpers::Read<sizeof(uint16_t)>(&sp), 0x0302U);
    EXPECT_EQ(sp.Size(), 4U);

    EXPECT_EQ(helpers::Read<sizeof(uint32_t)>(&sp), 0x07060504U);
    EXPECT_EQ(sp.Size(), 0U);
}

TEST(Helpers, ReadFromValueSpan)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data = {0x11, 0x22, 0x33, 0x44};
    Span<const uint8_t> sp(data.data(), data.size());

    EXPECT_EQ(helpers::Read<sizeof(uint32_t)>(sp), 0x44332211U);
}

TEST(Helpers, ReadOutOfBoundsShouldFatal)
{
    {
        // 1 byte requested, empty span
        Span<const uint8_t> sp;
        EXPECT_DEATH({ (void)helpers::Read<1>(&sp); }, ".*");
    }
    {
        // 2 bytes requested, 1 byte available
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0x01};
        Span<const uint8_t> sp(data.data(), data.size());
        EXPECT_DEATH({ (void)helpers::Read<sizeof(uint16_t)>(&sp); }, ".*");
    }
    {
        // 4 bytes requested, 3 bytes available
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::vector<uint8_t> data = {0x01, 0x02, 0x03};
        Span<const uint8_t> sp(data.data(), data.size());
        EXPECT_DEATH({ (void)helpers::Read<sizeof(uint32_t)>(&sp); }, ".*");
    }
}

TEST(Helpers, GetOptionalTaggedValue)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data = {0x01, 0xAA, 0xBB, 0x02};
    Span<const uint8_t> sp(data.data(), data.size());

    // Tag matches: value is read and span is advanced
    Span<const uint8_t> next;
    auto res = helpers::GetOptionalTaggedValue<uint16_t>(sp, 0x01, &next);
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(*res, 0xBBAAU);
    EXPECT_EQ(next.Size(), 1U);
    EXPECT_EQ(next[0], 0x02U);

    // Tag does not match: no value, span stays unchanged
    Span<const uint8_t> nextUnchanged;
    res = helpers::GetOptionalTaggedValue<uint16_t>(sp, 0x02, &nextUnchanged);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(nextUnchanged.Size(), sp.Size());
    EXPECT_EQ(nextUnchanged.data(), sp.data());
}

TEST(Helpers, GetOptionalTaggedValueEmptySpan)
{
    // Empty span: no value is returned instead of out-of-bounds access
    Span<const uint8_t> sp;
    Span<const uint8_t> next;
    auto res = helpers::GetOptionalTaggedValue<uint16_t>(sp, 0x01, &next);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(next.Size(), 0U);
}

TEST(Helpers, GetOptionalTaggedValueTruncatedShouldFatal)
{
    // Tag matches but there is not enough data for the value
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data = {0x01};
    Span<const uint8_t> sp(data.data(), data.size());
    Span<const uint8_t> next;
    EXPECT_DEATH({ (void)helpers::GetOptionalTaggedValue<uint32_t>(sp, 0x01, &next); }, ".*");
}

TEST(Helpers, EnumerateTaggedValues)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data = {0x01, 0xAA, 0x00, 0x01, 0xBB, 0x00, 0x02};
    Span<const uint8_t> sp(data.data(), data.size());

    std::vector<uint16_t> values;
    Span<const uint8_t> next;
    helpers::EnumerateTaggedValues<uint16_t>(
        sp, 0x01, [&values](uint16_t value) { values.push_back(value); }, &next);

    ASSERT_EQ(values.size(), 2U);
    EXPECT_EQ(values[0], 0x00AAU);
    EXPECT_EQ(values[1], 0x00BBU);
    // Iteration stops on the first byte which is not the tag
    EXPECT_EQ(next.Size(), 1U);
    EXPECT_EQ(next[0], 0x02U);
}

TEST(Helpers, EnumerateTaggedValuesEmptySpan)
{
    // Empty span: no callbacks are invoked instead of out-of-bounds access
    Span<const uint8_t> sp;
    Span<const uint8_t> next;
    size_t count = 0;
    helpers::EnumerateTaggedValues<uint16_t>(
        sp, 0x01, [&count](uint16_t) { count++; }, &next);
    EXPECT_EQ(count, 0U);
    EXPECT_EQ(next.Size(), 0U);
}

TEST(Helpers, EnumerateTaggedValuesWithEarlyStop)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::vector<uint8_t> data = {0x01, 0xAA, 0x00, 0x01, 0xBB, 0x00};
    Span<const uint8_t> sp(data.data(), data.size());

    // Callback stops the enumeration
    size_t count = 0;
    auto stopped =
        helpers::EnumerateTaggedValuesWithEarlyStop<uint16_t>(sp, 0x01, [&count](uint16_t) { return ++count == 1U; });
    EXPECT_TRUE(stopped);
    EXPECT_EQ(count, 1U);

    // Enumeration exhausts all tagged values
    count = 0;
    stopped = helpers::EnumerateTaggedValuesWithEarlyStop<uint16_t>(sp, 0x01, [&count](uint16_t) {
        count++;
        return false;
    });
    EXPECT_FALSE(stopped);
    EXPECT_EQ(count, 2U);
}

TEST(Helpers, EnumerateTaggedValuesWithEarlyStopEmptySpan)
{
    // Empty span: no callbacks are invoked instead of out-of-bounds access
    Span<const uint8_t> sp;
    size_t count = 0;
    auto stopped =
        helpers::EnumerateTaggedValuesWithEarlyStop<uint16_t>(sp, 0x01, [&count](uint16_t) { return ++count == 1U; });
    EXPECT_FALSE(stopped);
    EXPECT_EQ(count, 0U);
}

}  // namespace ark::panda_file::test
