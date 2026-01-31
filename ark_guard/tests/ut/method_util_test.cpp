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

#include "abckit_wrapper/method.h"

using namespace testing::ext;

// Test implementation of StripSetterGetterPrefix function
std::string TestStripSetterGetterPrefix(const std::string &name)
{
    constexpr std::string_view GET_PREFIX = "%%get-";
    constexpr std::string_view SET_PREFIX = "%%set-";

    const size_t getPos = name.find(GET_PREFIX);
    if (getPos != std::string::npos) {
        return name.substr(0, getPos) + name.substr(getPos + GET_PREFIX.size());
    }
    const size_t setPos = name.find(SET_PREFIX);
    if (setPos != std::string::npos) {
        return name.substr(0, setPos) + name.substr(setPos + SET_PREFIX.size());
    }
    return name;
}

/**
 * @tc.name: strip_setter_getter_prefix_test_001
 * @tc.desc: test StripSetterGetterPrefix function with getter prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MethodUtilTest, strip_setter_getter_prefix_test_001, TestSize.Level1)
{
    const std::string getterMethod = "%%get-fieldName";
    const std::string expected = "fieldName";
    const std::string result = TestStripSetterGetterPrefix(getterMethod);
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: strip_setter_getter_prefix_test_002
 * @tc.desc: test StripSetterGetterPrefix function with setter prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MethodUtilTest, strip_setter_getter_prefix_test_002, TestSize.Level1)
{
    const std::string setterMethod = "%%set-fieldName";
    const std::string expected = "fieldName";
    const std::string result = TestStripSetterGetterPrefix(setterMethod);
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: strip_setter_getter_prefix_test_003
 * @tc.desc: test StripSetterGetterPrefix function without prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MethodUtilTest, strip_setter_getter_prefix_test_003, TestSize.Level1)
{
    const std::string normalMethod = "fieldName";
    const std::string expected = "fieldName";
    const std::string result = TestStripSetterGetterPrefix(normalMethod);
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: strip_setter_getter_prefix_test_004
 * @tc.desc: test StripSetterGetterPrefix function with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(MethodUtilTest, strip_setter_getter_prefix_test_004, TestSize.Level1)
{
    const std::string emptyString = "";
    const std::string expected = "";
    const std::string result = TestStripSetterGetterPrefix(emptyString);
    EXPECT_EQ(result, expected);
}
