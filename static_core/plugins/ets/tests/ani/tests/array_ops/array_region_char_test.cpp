/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

// Add constants at namespace scope
constexpr ani_size TEST_ARRAY_SIZE = 5U;
constexpr uint32_t TEST_BUFFER_SIZE = 10U;

class ArraySetGetRegionCharTest : public AniTest {};

TEST_F(ArraySetGetRegionCharTest, SetCharArrayRegionErrorTests)
{
    ani_array_char array;
    ASSERT_EQ(env_->Array_New_Char(TEST_ARRAY_SIZE, &array), ANI_OK);
    ani_char nativeBuffer[TEST_BUFFER_SIZE] = {0};
    const ani_size offset1 = -1;
    const ani_size len1 = 2;
    ASSERT_EQ(env_->Array_SetRegion_Char(array, offset1, len1, nativeBuffer), ANI_OUT_OF_RANGE);

    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_SetRegion_Char(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->Array_SetRegion_Char(array, offset3, len3, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionCharTest, GetCharArrayRegionErrorTests)
{
    ani_array_char array;
    ASSERT_EQ(env_->Array_New_Char(5U, &array), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_char nativeBuffer[bufferSize] = {0};
    const ani_size offset1 = 0;
    const ani_size len1 = 1;
    ASSERT_EQ(env_->Array_GetRegion_Char(array, offset1, len1, nullptr), ANI_INVALID_ARGS);
    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_GetRegion_Char(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_GetRegion_Char(array, offset1, len1, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionCharTest, GetRegionCharTest)
{
    const auto array = static_cast<ani_array_char>(CallEtsFunction<ani_ref>("getArray"));

    ani_char nativeBuffer[TEST_ARRAY_SIZE] = {0};
    const ani_size offset3 = 0;
    const ani_size len3 = TEST_ARRAY_SIZE;
    ASSERT_EQ(env_->Array_GetRegion_Char(array, offset3, len3, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], 'a');
    ASSERT_EQ(nativeBuffer[1U], 'b');
    ASSERT_EQ(nativeBuffer[2U], 'c');
    ASSERT_EQ(nativeBuffer[3U], 'd');
    ASSERT_EQ(nativeBuffer[4U], 'e');
}

TEST_F(ArraySetGetRegionCharTest, SetRegionCharTest)
{
    const auto array = static_cast<ani_array_char>(CallEtsFunction<ani_ref>("getArray"));
    ani_char nativeBuffer1[TEST_ARRAY_SIZE] = {'x', 'y', 'z'};
    const ani_size offset4 = 2;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->Array_SetRegion_Char(array, offset4, len4, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkArray", array), ANI_TRUE);
}

TEST_F(ArraySetGetRegionCharTest, SetRegionCharChangeTest)
{
    ani_array_char array;
    ASSERT_EQ(env_->Array_New_Char(5U, &array), ANI_OK);

    const auto changedArray = static_cast<ani_array_char>(CallEtsFunction<ani_ref>("changeArray", array));
    ani_char nativeBuffer[TEST_ARRAY_SIZE] = {0};
    const ani_size offset = 0;
    const ani_size len = TEST_ARRAY_SIZE;
    ASSERT_EQ(env_->Array_GetRegion_Char(changedArray, offset, len, nativeBuffer), ANI_OK);

    ASSERT_EQ(nativeBuffer[0U], 'a');
    ASSERT_EQ(nativeBuffer[1U], 'b');
    ASSERT_EQ(nativeBuffer[2U], 'x');
    ASSERT_EQ(nativeBuffer[3U], 'y');
    ASSERT_EQ(nativeBuffer[4U], 'z');
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
