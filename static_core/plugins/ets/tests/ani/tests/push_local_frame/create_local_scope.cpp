/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class CreateLocalScopeTest : public AniTest {};

// Define constants to replace magic numbers
constexpr ani_size SPECIFIED_CAPACITY = 60;
constexpr ani_size MAX_CAPACITY = 32000000;
constexpr ani_size MIN_CAPACITY = 1;
constexpr size_t TEST_STRING_LENGTH = 4;

TEST_F(CreateLocalScopeTest, create_local_scope_test)
{
    // Passing 0 as capacity should return ANI_INVALID_ARGS
    ASSERT_EQ(env_->CreateLocalScope(0), ANI_INVALID_ARGS);

    // Passing SPECIFIED_CAPACITY as capacity should succeed and return ANI_OK
    ASSERT_EQ(env_->CreateLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string string = nullptr;

    // Create SPECIFIED_CAPACITY strings in the newly created local scope
    for (ani_size i = 1; i <= SPECIFIED_CAPACITY; ++i) {
        // Construct a unique stringName for each iteration
        std::string stringName = "String_NewUTF8_" + std::to_string(i) + ";";

        // Attempt to create a new UTF8 string and check the result
        ASSERT_EQ(env_->String_NewUTF8(stringName.c_str(), stringName.size(), &string), ANI_OK);
        ASSERT_NE(string, nullptr);
    }

    // Create another string to confirm that string creation still works
    ani_status res = env_->String_NewUTF8("test", TEST_STRING_LENGTH, &string);
    ASSERT_EQ(res, ANI_OK);

    // Destroy the local scope after string creation
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    /*
     * Attempt to create a local scope with very large capacity (MAX_CAPACITY).
     * The comment below indicates that the free size of local reference storage is
     * smaller than the requested capacity, thus should return ANI_OUT_OF_MEMORY.
     */
    // Free size of local reference storage is less than capacity: MAX_CAPACITY
    // blocks_count_: 3 need_blocks: 533334 blocks_free: 524285
    ASSERT_EQ(env_->CreateLocalScope(MAX_CAPACITY), ANI_OUT_OF_MEMORY);
}

TEST_F(CreateLocalScopeTest, create_escape_local_scope)
{
    // Passing 0 as capacity should return ANI_INVALID_ARGS
    ASSERT_EQ(env_->CreateEscapeLocalScope(0), ANI_INVALID_ARGS);

    // Passing SPECIFIED_CAPACITY as capacity should succeed
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string string = nullptr;
    // Create multiple strings to test scope behavior
    for (ani_size i = 1; i <= SPECIFIED_CAPACITY; ++i) {
        std::string stringName = "String_NewUTF8_" + std::to_string(i) + ";";

        ASSERT_EQ(env_->String_NewUTF8(stringName.c_str(), stringName.size(), &string), ANI_OK);
        ASSERT_NE(string, nullptr);
    }

    // Create another string to confirm it still works
    ani_status res = env_->String_NewUTF8("test", TEST_STRING_LENGTH, &string);
    ASSERT_EQ(res, ANI_OK);

    ani_ref result;
    // Destroy the escape local scope and retrieve the final reference
    ASSERT_EQ(env_->DestroyEscapeLocalScope(string, &result), ANI_OK);

    /*
     * Trying to create an escape local scope with a large capacity
     * should fail due to insufficient memory (ANI_OUT_OF_MEMORY).
     */
    // Free size of local reference storage is less than capacity: MAX_CAPACITY
    // blocks_count_: 3 need_blocks: 533334 blocks_free: 524285
    ASSERT_EQ(env_->CreateEscapeLocalScope(MAX_CAPACITY), ANI_OUT_OF_MEMORY);
}

TEST_F(CreateLocalScopeTest, destroy_local_scope)
{
    // Create a local scope with capacity of MIN_CAPACITY
    ASSERT_EQ(env_->CreateLocalScope(MIN_CAPACITY), ANI_OK);

    ani_string string = nullptr;
    // Attempt to create a new string
    ASSERT_EQ(env_->String_NewUTF8("test", TEST_STRING_LENGTH, &string), ANI_OK);

    // Destroy the local scope after string creation
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope)
{
    // Passing SPECIFIED_CAPACITY as capacity should succeed
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);

    ani_string string = nullptr;
    // Create a string to test with
    ASSERT_EQ(env_->String_NewUTF8("test", TEST_STRING_LENGTH, &string), ANI_OK);

    ani_ref result;
    // Destroy the escape local scope and retrieve the final reference
    ASSERT_EQ(env_->DestroyEscapeLocalScope(string, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_null)
{
    // test for null
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ani_ref nullRef;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_ref result;
    ASSERT_EQ(env_->DestroyEscapeLocalScope(nullRef, &result), ANI_OK);
    ani_boolean isNull = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsNull(result, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
}

TEST_F(CreateLocalScopeTest, destroy_escape_local_scope_undefinde)
{
    // test for undefined
    ASSERT_EQ(env_->CreateEscapeLocalScope(SPECIFIED_CAPACITY), ANI_OK);
    ani_ref undefined;
    ASSERT_EQ(env_->GetUndefined(&undefined), ANI_OK);
    ani_ref result;
    ASSERT_EQ(env_->DestroyEscapeLocalScope(undefined, &result), ANI_OK);
    ani_boolean isUndefined = ANI_FALSE;
    ASSERT_EQ(env_->Reference_IsUndefined(result, &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing
