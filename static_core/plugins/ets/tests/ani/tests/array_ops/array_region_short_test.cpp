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

#include "ani_gtest_array_ops.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArraySetGetRegionShortTest : public AniGTestArrayOps {
protected:
    static constexpr ani_short TEST_VALUE1 = 1;
    static constexpr ani_short TEST_VALUE2 = 2;
    static constexpr ani_short TEST_VALUE3 = 3;
    static constexpr ani_short TEST_VALUE4 = 4;
    static constexpr ani_short TEST_VALUE5 = 5;

    static constexpr ani_short TEST_UPDATE1 = 30;
    static constexpr ani_short TEST_UPDATE2 = 40;
    static constexpr ani_short TEST_UPDATE3 = 50;

    static constexpr ani_short TEST_UPDATE4 = 222;
    static constexpr ani_short TEST_UPDATE5 = 444;
    static constexpr ani_short TEST_UPDATE6 = 333;
};

TEST_F(ArraySetGetRegionShortTest, SetShortArrayRegionErrorTests)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->Array_New_Short(LENGTH_5, &array), ANI_OK);
    ani_short nativeBuffer[LENGTH_10] = {0};
    const ani_size offset1 = -1;
    ASSERT_EQ(env_->Array_SetRegion_Short(array, offset1, LENGTH_2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionShortTest, GetShortArrayRegionErrorTests)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->Array_New_Short(LENGTH_5, &array), ANI_OK);
    ani_short nativeBuffer[LENGTH_10] = {0};
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    // Should change to ANI_OK when std lib array will work according to spec
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_1, nativeBuffer), ANI_ERROR);
}

TEST_F(ArraySetGetRegionShortTest, SetShortFixedArrayRegionErrorTests)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(LENGTH_5, &array), ANI_OK);
    ani_short nativeBuffer[LENGTH_10] = {0};
    const ani_size offset1 = -1;
    ASSERT_EQ(env_->Array_SetRegion_Short(array, offset1, LENGTH_2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionShortTest, GetShortFixedArrayRegionErrorTests)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(LENGTH_5, &array), ANI_OK);
    ani_short nativeBuffer[LENGTH_10] = {0};
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_1, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionShortTest, GetRegionShortTest)
{
    const auto array = static_cast<ani_array_short>(CallEtsFunction<ani_ref>("array_region_short_test", "GetArray"));

    ani_short nativeBuffer[LENGTH_5] = {0};
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE5);
}

TEST_F(ArraySetGetRegionShortTest, SetRegionShortTest)
{
    const auto array = static_cast<ani_array_short>(CallEtsFunction<ani_ref>("array_region_short_test", "GetArray"));
    ani_short nativeBuffer1[LENGTH_3] = {TEST_UPDATE1, TEST_UPDATE2, TEST_UPDATE3};
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_2, LENGTH_3, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("array_region_short_test", "CheckArray", array), ANI_TRUE);
}

TEST_F(ArraySetGetRegionShortTest, CheckChangeFromManagedRegionShortTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Larray_region_short_test/ArrayClass;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_array_short>(ref);
    ani_short nativeBuffer[LENGTH_5] = {0};

    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE5);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "ChangeStaticArray", nullptr), ANI_OK);
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE2);
    ASSERT_EQ(nativeBuffer[2U], TEST_UPDATE4);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE4);
    ASSERT_EQ(nativeBuffer[4U], TEST_UPDATE5);
}

TEST_F(ArraySetGetRegionShortTest, CheckChangeFromApiRegionShortTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Larray_region_short_test/ArrayClass;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_array_short>(ref);
    ani_short nativeBuffer[LENGTH_3] = {TEST_UPDATE4, TEST_UPDATE6, TEST_UPDATE5};

    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_2, LENGTH_3, nativeBuffer), ANI_OK);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "CheckStaticArray", nullptr, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(ArraySetGetRegionShortTest, GetSpecialValueToArrayTest)
{
    const auto array =
        static_cast<ani_array_short>(CallEtsFunction<ani_ref>("array_region_short_test", "GetSpecialArray"));
    std::array<ani_short, LENGTH_5> nativeBuffer = {};
    const ani_short maxShortValue = 32767;
    const ani_short minShortValue = -32768;
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], minShortValue);
    ASSERT_EQ(nativeBuffer[1U], maxShortValue);
    ASSERT_EQ(nativeBuffer[2U], 0);
}

TEST_F(ArraySetGetRegionShortTest, SetSpecialValueToArrayTest)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(LENGTH_5, &array), ANI_OK);
    const ani_short maxShortValue = 32767;
    const ani_short minShortValue = -32768;
    const std::array<ani_short, LENGTH_5> nativeBuffer = {minShortValue, maxShortValue, 0, -1, 1};
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);

    std::array<ani_short, LENGTH_5> nativeBuffer2 = {};
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer2[0U], minShortValue);
    ASSERT_EQ(nativeBuffer2[1U], maxShortValue);
    ASSERT_EQ(nativeBuffer2[2U], 0);
    ASSERT_EQ(nativeBuffer2[3U], -1);
}

TEST_F(ArraySetGetRegionShortTest, SetGetUnionToArrayTest)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(LENGTH_5, &array), ANI_OK);

    std::array<ani_short, LENGTH_5> nativeBuffer = {TEST_VALUE1, TEST_VALUE2, TEST_VALUE3, TEST_VALUE4, TEST_VALUE5};
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);

    std::array<ani_short, LENGTH_5> nativeBuffer2 = {};
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    std::array<ani_short, LENGTH_5> nativeBuffer3 = {TEST_VALUE1, TEST_VALUE3, TEST_VALUE5, TEST_VALUE2, TEST_VALUE4};
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer2, nativeBuffer3);

    ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

TEST_F(ArraySetGetRegionShortTest, SetGetStabilityToArrayTest)
{
    ani_array_short array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(LENGTH_5, &array), ANI_OK);

    std::array<ani_short, LENGTH_5> nativeBuffer = {TEST_VALUE1, TEST_VALUE2, TEST_VALUE3, TEST_VALUE4, TEST_VALUE5};
    std::array<ani_short, LENGTH_5> nativeBuffer2 = {};
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->Array_SetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
        ASSERT_EQ(env_->Array_GetRegion_Short(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

TEST_F(ArraySetGetRegionShortTest, EscompatGetRegionShortTest)
{
    const auto array =
        static_cast<ani_array_short>(CallEtsFunction<ani_ref>("array_region_short_test", "GetEscompatArray"));

    ani_short nativeBuffer[5U] = {0};
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->Array_GetRegion_Short(array, offset3, len3, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE5);
}

TEST_F(ArraySetGetRegionShortTest, EscompatSetRegionShortTest)
{
    const auto array =
        static_cast<ani_array_short>(CallEtsFunction<ani_ref>("array_region_short_test", "GetEscompatArray"));
    ani_short nativeBuffer1[5U] = {TEST_UPDATE1, TEST_UPDATE2, TEST_UPDATE3};
    const ani_size offset4 = 2;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->Array_SetRegion_Short(array, offset4, len4, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("array_region_short_test", "CheckEscompatArray", array), ANI_TRUE);
}

TEST_F(ArraySetGetRegionShortTest, EscompatInvalidShortTest)
{
    const auto array =
        static_cast<ani_array_short>(CallEtsFunction<ani_ref>("array_region_short_test", "GetEscompatArray"));
    ani_short nativeBuffer1[5U] = {TEST_UPDATE1, TEST_UPDATE2, TEST_UPDATE3};
    const ani_size offset4 = 3;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->Array_SetRegion_Short(array, offset4, len4, nativeBuffer1), ANI_OUT_OF_RANGE);
    ani_short nativeBuffer[5U] = {0};
    ASSERT_EQ(env_->Array_GetRegion_Short(array, offset4, len4, nativeBuffer), ANI_OUT_OF_RANGE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
