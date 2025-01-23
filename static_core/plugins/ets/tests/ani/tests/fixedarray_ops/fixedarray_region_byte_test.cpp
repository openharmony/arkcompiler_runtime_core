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

class FixedArraySetGetRegionByteTest : public AniTest {};

// ninja ani_test_byte_array_region_gtests
TEST_F(FixedArraySetGetRegionByteTest, SetByteArrayRegionErrorTests)
{
    ani_fixedarray_byte fixedarray;
    ASSERT_EQ(env_->FixedArray_New_Byte(5U, &fixedarray), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_byte nativeBuffer[bufferSize] = {0};
    ani_size offset1 = -1;
    ani_size len1 = 2;
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(fixedarray, offset1, len1, nativeBuffer), ANI_OUT_OF_RANGE);

    ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(fixedarray, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ani_size offset3 = 0;
    ani_size len3 = 5;
    // issue 22090
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(fixedarray, offset3, len3, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionByteTest, GetByteArrayRegionErrorTests)
{
    ani_fixedarray_byte fixedarray;
    ASSERT_EQ(env_->FixedArray_New_Byte(5U, &fixedarray), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_byte nativeBuffer[bufferSize] = {0};
    ani_size offset1 = 0;
    ani_size len1 = 1;
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(fixedarray, offset1, len1, nullptr), ANI_INVALID_ARGS);
    ani_size offset2 = 5;
    const ani_size len2 = 10U;
    // issue 22090
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(fixedarray, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(fixedarray, offset1, len1, nativeBuffer), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)