/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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
constexpr ani_byte TEST_VALUE_1 = 1U;
constexpr ani_byte TEST_VALUE_2 = 2U;
constexpr ani_byte TEST_VALUE_3 = 3U;
constexpr ani_byte TEST_VALUE_4 = 4U;
constexpr ani_byte TEST_VALUE_5 = 5U;

constexpr ani_byte TEST_UPDATE_1 = 30U;
constexpr ani_byte TEST_UPDATE_2 = 40U;
constexpr ani_byte TEST_UPDATE_3 = 50U;
constexpr ani_byte TEST_UPDATE_4 = 22U;
constexpr ani_byte TEST_UPDATE_5 = 44U;
constexpr ani_byte TEST_UPDATE_6 = 33U;

constexpr ani_size TEST_ARRAY_SIZE = 5U;
constexpr uint32_t TEST_BUFFER_SIZE = 10U;

class ArraySetGetRegionByteTest : public AniTest {};

TEST_F(ArraySetGetRegionByteTest, SetByteArrayRegionErrorTests)
{
    ani_array_byte array;
    ASSERT_EQ(env_->Array_New_Byte(TEST_ARRAY_SIZE, &array), ANI_OK);
    ani_byte nativeBuffer[TEST_BUFFER_SIZE] = {0};
    const ani_size offset1 = -1;
    const ani_size len1 = 2;
    ASSERT_EQ(env_->Array_SetRegion_Byte(array, offset1, len1, nativeBuffer), ANI_OUT_OF_RANGE);

    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_SetRegion_Byte(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->Array_SetRegion_Byte(array, offset3, len3, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionByteTest, GetByteArrayRegionErrorTests)
{
    ani_array_byte array;
    ASSERT_EQ(env_->Array_New_Byte(5U, &array), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_byte nativeBuffer[bufferSize] = {0};
    const ani_size offset1 = 0;
    const ani_size len1 = 1;
    ASSERT_EQ(env_->Array_GetRegion_Byte(array, offset1, len1, nullptr), ANI_INVALID_ARGS);
    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_GetRegion_Byte(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_GetRegion_Byte(array, offset1, len1, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionByteTest, GetRegionByteTest)
{
    const auto array = static_cast<ani_array_byte>(CallEtsFunction<ani_ref>("GetArray"));

    ani_byte nativeBuffer[TEST_ARRAY_SIZE] = {0};
    const ani_size offset3 = 0;
    const ani_size len3 = TEST_ARRAY_SIZE;
    ASSERT_EQ(env_->Array_GetRegion_Byte(array, offset3, len3, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE_1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE_2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE_3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE_4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE_5);
}

TEST_F(ArraySetGetRegionByteTest, SetRegionByteTest)
{
    const auto array = static_cast<ani_array_byte>(CallEtsFunction<ani_ref>("GetArray"));
    ani_byte nativeBuffer1[TEST_ARRAY_SIZE] = {TEST_UPDATE_1, TEST_UPDATE_2, TEST_UPDATE_3};
    const ani_size offset4 = 2;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->Array_SetRegion_Byte(array, offset4, len4, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("CheckArray", array), ANI_TRUE);
}

TEST_F(ArraySetGetRegionByteTest, CheckChangeFromManagedRegionByteTest)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LArrayClass;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_array_byte>(ref);
    ani_byte nativeBuffer[TEST_ARRAY_SIZE] = {0};
    const ani_size offset5 = 0;
    const ani_size len5 = TEST_ARRAY_SIZE;

    ASSERT_EQ(env_->Array_GetRegion_Byte(array, offset5, len5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE_1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE_2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE_3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE_4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE_5);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "ChangeStaticArray", nullptr), ANI_OK);
    ASSERT_EQ(env_->Array_GetRegion_Byte(array, offset5, len5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE_1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE_2);
    ASSERT_EQ(nativeBuffer[2U], TEST_UPDATE_4);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE_4);
    ASSERT_EQ(nativeBuffer[4U], TEST_UPDATE_5);
}

TEST_F(ArraySetGetRegionByteTest, CheckChangeFromApiRegionByteTest)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("LArrayClass;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_array_byte>(ref);
    ani_byte nativeBuffer[3U] = {TEST_UPDATE_4, TEST_UPDATE_6, TEST_UPDATE_5};
    const ani_size offset6 = 2;
    const ani_size len6 = 3;

    ASSERT_EQ(env_->Array_SetRegion_Byte(array, offset6, len6, nativeBuffer), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "CheckStaticArray", nullptr, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
