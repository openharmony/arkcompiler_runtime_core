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

#include <gtest/gtest.h>

#include <cstdint>
#include <iomanip>
#include <limits>
#include <string_view>

#include "runtime/on_stack_string.h"

namespace ark::test {
namespace {

TEST(OnStackStringTest, EmptyByDefault)
{
    OnStackString<16> text;

    ASSERT_EQ(text.size(), 0);
    ASSERT_STREQ(text.c_str(), "");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

TEST(OnStackStringTest, AppendAotEscapeStyleLine)
{
    OnStackString<64> text;
    uintptr_t pc = 1234;
    std::string_view isInAot = "true\n";

    text << "Signal: " << 6 << ", PC: " << pc << ", isInAot: " << isInAot;

    ASSERT_STREQ(text.c_str(), "Signal: 6, PC: 1234, isInAot: true\n");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

TEST(OnStackStringTest, AppendSignedAndUnsignedNumbers)
{
    OnStackString<96> text;

    text << std::numeric_limits<int64_t>::min() << "," << std::numeric_limits<uint64_t>::max();

    ASSERT_STREQ(text.c_str(), "-9223372036854775808,18446744073709551615");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

TEST(OnStackStringTest, AppendEmptyStringViewDoesNotChangeState)
{
    OnStackString<16> text;
    std::string_view empty {};

    text << "a" << empty << "b";

    ASSERT_STREQ(text.c_str(), "ab");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

TEST(OnStackStringTest, TruncateWhenFull)
{
    OnStackString<8> text;

    text << "12345678";
    text << "abcdef";

    ASSERT_STREQ(text.c_str(), "12345678");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

TEST(OnStackStringTest, ClearResetsState)
{
    OnStackString<8> text;

    text << "12345678";
    text.clear();

    ASSERT_STREQ(text.c_str(), "");
    ASSERT_EQ(text.data()[text.size()], '\0');
    ASSERT_EQ(text.size(), 0);
}

TEST(OnStackStringTest, SupportsHexAndDecManipulators)
{
    OnStackString<64> text;

    text << "pc: " << std::hex << static_cast<uintptr_t>(0x12ab) << ", sig: " << std::dec << 6;

    ASSERT_STREQ(text.c_str(), "pc: 12ab, sig: 6");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

TEST(OnStackStringTest, SupportsPointerFormatting)
{
    OnStackString<64> text;
    auto ptr = reinterpret_cast<void *>(static_cast<uintptr_t>(0x1234));

    text << "handler=" << ptr;

    ASSERT_STREQ(text.c_str(), "handler=0x1234");
    ASSERT_EQ(text.data()[text.size()], '\0');
}

}  // namespace
}  // namespace ark::test
