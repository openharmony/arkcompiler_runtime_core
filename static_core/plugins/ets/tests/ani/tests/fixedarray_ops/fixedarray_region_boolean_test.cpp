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

class FixedArraySetGetRegionBooleanTest : public AniTest {};

// ninja ani_test_boolean_array_region_gtests
TEST_F(FixedArraySetGetRegionBooleanTest, SetBooleanArrayRegionErrorTests)
{
    ani_fixedarray_boolean fixedarray;
    ASSERT_EQ(env_->FixedArray_New_Boolean(5U, &fixedarray), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_boolean nativeBuffer[bufferSize] = {ANI_FALSE};
    const ani_size offset1 = -1;
    const ani_size len1 = 2;
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(fixedarray, offset1, len1, nativeBuffer), ANI_OUT_OF_RANGE);

    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(fixedarray, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(fixedarray, offset3, len3, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionBooleanTest, GetBooleanArrayRegionErrorTests)
{
    ani_fixedarray_boolean fixedarray;
    ASSERT_EQ(env_->FixedArray_New_Boolean(5U, &fixedarray), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_boolean nativeBuffer[bufferSize] = {ANI_TRUE};
    const ani_size offset1 = 0;
    const ani_size len1 = 1;
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(fixedarray, offset1, len1, nullptr), ANI_INVALID_ARGS);
    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(fixedarray, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(fixedarray, offset1, len1, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionBooleanTest, GetRegionBooleanTest)
{
    const auto array = static_cast<ani_fixedarray_boolean>(CallEtsFunction<ani_ref>("GetArray"));

    ani_boolean nativeBuffer[5U] = {ANI_FALSE};
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, offset3, len3, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[1U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[2U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[3U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[4U], ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionBooleanTest, SetRegionBooleanTest)
{
    const auto array = static_cast<ani_fixedarray_boolean>(CallEtsFunction<ani_ref>("GetArray"));
    const ani_boolean nativeBuffer1[5U] = {ANI_TRUE, ANI_FALSE, ANI_TRUE};
    const ani_size offset4 = 2;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, offset4, len4, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("CheckArray", array), ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
