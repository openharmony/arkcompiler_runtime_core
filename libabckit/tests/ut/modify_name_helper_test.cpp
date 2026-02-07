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

#include "adapter_static/modify_name_helper.h"

using namespace testing::ext;

namespace {
constexpr std::string_view GETTER_PREFIX = "%%get-";
constexpr std::string_view SETTER_PREFIX = "%%set-";
}  // namespace

// Test implementation of AsyncReplaceFast
std::string AsyncReplaceFast(const std::string &oldFunctionKeyName, const std::string &newName)
{
    constexpr std::string_view ASYNC_MARKER = "%%async-";
    const size_t pos = oldFunctionKeyName.find(ASYNC_MARKER);
    if (pos == std::string::npos) {
        return oldFunctionKeyName;
    }
    const size_t nameStart = pos + ASYNC_MARKER.size();
    const size_t colonPos = oldFunctionKeyName.find(':', nameStart);
    if (colonPos == std::string::npos) {
        return oldFunctionKeyName;
    }
    return oldFunctionKeyName.substr(0, nameStart) + newName + oldFunctionKeyName.substr(colonPos);
}

// Test implementation of GetSetMatchAndReplace
bool GetSetMatchAndReplace(const std::string &oldFunctionKeyName, const std::string &fieldName,
                           const std::string &newName, std::string &outNewFunctionKeyName)
{
    const std::string getPrefix = std::string(GETTER_PREFIX) + fieldName + ":";
    const std::string setPrefix = std::string(SETTER_PREFIX) + fieldName + ":";
    if (oldFunctionKeyName.find(getPrefix) != std::string::npos) {
        const size_t pos = oldFunctionKeyName.find(getPrefix);
        outNewFunctionKeyName = oldFunctionKeyName.substr(0, pos) + std::string(GETTER_PREFIX) + newName + ":" +
                                oldFunctionKeyName.substr(pos + getPrefix.size());
        return true;
    }
    if (oldFunctionKeyName.find(setPrefix) != std::string::npos) {
        const size_t pos = oldFunctionKeyName.find(setPrefix);
        outNewFunctionKeyName = oldFunctionKeyName.substr(0, pos) + std::string(SETTER_PREFIX) + newName + ":" +
                                oldFunctionKeyName.substr(pos + setPrefix.size());
        return true;
    }
    return false;
}

/**
 * @tc.name: async_replace_fast_test_001
 * @tc.desc: test AsyncReplaceFast function with async prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ModifyNameHelperTest, async_replace_fast_test_001, TestSize.Level1)
{
    const std::string oldFunctionKeyName = "%%async-oldName:param1:param2:returnType";
    const std::string newName = "newName";
    const std::string expected = "%%async-newName:param1:param2:returnType";
    const std::string result = AsyncReplaceFast(oldFunctionKeyName, newName);
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: async_replace_fast_test_002
 * @tc.desc: test AsyncReplaceFast function without async prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ModifyNameHelperTest, async_replace_fast_test_002, TestSize.Level1)
{
    const std::string oldFunctionKeyName = "oldName:param1:param2:returnType";
    const std::string newName = "newName";
    const std::string expected = "oldName:param1:param2:returnType";
    const std::string result = AsyncReplaceFast(oldFunctionKeyName, newName);
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: get_set_match_and_replace_test_001
 * @tc.desc: test GetSetMatchAndReplace function with getter prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ModifyNameHelperTest, get_set_match_and_replace_test_001, TestSize.Level1)
{
    std::string outNewFunctionKeyName;
    const std::string oldFunctionKeyName = "%%get-fieldName:param1:returnType";
    const std::string fieldName = "fieldName";
    const std::string newName = "newFieldName";
    const std::string expected = "%%get-newFieldName:param1:returnType";
    const bool result = GetSetMatchAndReplace(oldFunctionKeyName, fieldName, newName, outNewFunctionKeyName);
    EXPECT_TRUE(result);
    EXPECT_EQ(outNewFunctionKeyName, expected);
}

/**
 * @tc.name: get_set_match_and_replace_test_002
 * @tc.desc: test GetSetMatchAndReplace function with setter prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ModifyNameHelperTest, get_set_match_and_replace_test_002, TestSize.Level1)
{
    std::string outNewFunctionKeyName;
    const std::string oldFunctionKeyName = "%%set-fieldName:param1:returnType";
    const std::string fieldName = "fieldName";
    const std::string newName = "newFieldName";
    const std::string expected = "%%set-newFieldName:param1:returnType";
    const bool result = GetSetMatchAndReplace(oldFunctionKeyName, fieldName, newName, outNewFunctionKeyName);
    EXPECT_TRUE(result);
    EXPECT_EQ(outNewFunctionKeyName, expected);
}

/**
 * @tc.name: get_set_match_and_replace_test_003
 * @tc.desc: test GetSetMatchAndReplace function without get/set prefix
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ModifyNameHelperTest, get_set_match_and_replace_test_003, TestSize.Level1)
{
    std::string outNewFunctionKeyName;
    const std::string oldFunctionKeyName = "fieldName:param1:returnType";
    const std::string fieldName = "fieldName";
    const std::string newName = "newFieldName";
    const bool result = GetSetMatchAndReplace(oldFunctionKeyName, fieldName, newName, outNewFunctionKeyName);
    EXPECT_FALSE(result);
}
