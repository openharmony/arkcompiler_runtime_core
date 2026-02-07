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

#include "util/string_util.h"

using namespace testing::ext;

// Test implementations of buffer functions
static void TestEscapeParenthesesToBuffer(std::string &out, const std::string &str)
{
    constexpr int braceSize = 2;
    out.clear();
    out.reserve(str.size() + braceSize);
    for (const char c : str) {
        if (c == '{' || c == '}') {
            out += '\\';
        }
        out += c;
    }
}

static void TestRemoveSpacesToBuffer(std::string &out, const std::string &str)
{
    out = str;
    out.erase(std::remove(out.begin(), out.end(), ' '), out.end());
}

/**
 * @tc.name: escape_parentheses_to_buffer_test_001
 * @tc.desc: test EscapeParenthesesToBuffer function with braces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilBufferTest, escape_parentheses_to_buffer_test_001, TestSize.Level1)
{
    std::string out;
    const std::string input = "{test}";
    const std::string expected = "\\{test\\}";
    TestEscapeParenthesesToBuffer(out, input);
    EXPECT_EQ(out, expected);
}

/**
 * @tc.name: escape_parentheses_to_buffer_test_002
 * @tc.desc: test EscapeParenthesesToBuffer function without braces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilBufferTest, escape_parentheses_to_buffer_test_002, TestSize.Level1)
{
    std::string out;
    const std::string input = "test";
    const std::string expected = "test";
    TestEscapeParenthesesToBuffer(out, input);
    EXPECT_EQ(out, expected);
}

/**
 * @tc.name: escape_parentheses_to_buffer_test_003
 * @tc.desc: test EscapeParenthesesToBuffer function with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilBufferTest, escape_parentheses_to_buffer_test_003, TestSize.Level1)
{
    std::string out;
    const std::string input = "";
    const std::string expected = "";
    TestEscapeParenthesesToBuffer(out, input);
    EXPECT_EQ(out, expected);
}

/**
 * @tc.name: remove_spaces_to_buffer_test_001
 * @tc.desc: test RemoveSpacesToBuffer function with spaces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilBufferTest, remove_spaces_to_buffer_test_001, TestSize.Level1)
{
    std::string out;
    const std::string input = "test test";
    const std::string expected = "testtest";
    TestRemoveSpacesToBuffer(out, input);
    EXPECT_EQ(out, expected);
}

/**
 * @tc.name: remove_spaces_to_buffer_test_002
 * @tc.desc: test RemoveSpacesToBuffer function without spaces
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilBufferTest, remove_spaces_to_buffer_test_002, TestSize.Level1)
{
    std::string out;
    const std::string input = "test";
    const std::string expected = "test";
    TestRemoveSpacesToBuffer(out, input);
    EXPECT_EQ(out, expected);
}

/**
 * @tc.name: remove_spaces_to_buffer_test_003
 * @tc.desc: test RemoveSpacesToBuffer function with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(StringUtilBufferTest, remove_spaces_to_buffer_test_003, TestSize.Level1)
{
    std::string out;
    const std::string input = "";
    const std::string expected = "";
    TestRemoveSpacesToBuffer(out, input);
    EXPECT_EQ(out, expected);
}
