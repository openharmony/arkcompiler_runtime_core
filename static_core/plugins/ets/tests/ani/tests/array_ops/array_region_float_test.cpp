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

class ArraySetGetRegionFloatTest : public AniTest {
protected:
    static constexpr float TEST_VALUE_1 = 1.0F;
    static constexpr float TEST_VALUE_2 = 2.0F;
    static constexpr float TEST_VALUE_3 = 3.0F;
    static constexpr float TEST_VALUE_4 = 4.0F;
    static constexpr float TEST_VALUE_5 = 5.0F;
    static constexpr float TEST_UPDATE_1 = 30.0F;
    static constexpr float TEST_UPDATE_2 = 40.0F;
    static constexpr float TEST_UPDATE_3 = 50.0F;
};

TEST_F(ArraySetGetRegionFloatTest, SetFloatArrayRegionErrorTests)
{
    ani_array_float array;
    ASSERT_EQ(env_->Array_New_Float(5U, &array), ANI_OK);
    const uint32_t bufferSize = 5U;
    ani_float nativeBuffer[bufferSize] = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4, TEST_VALUE_5};
    const ani_size offset1 = -1;
    const ani_size len1 = 2;
    ASSERT_EQ(env_->Array_SetRegion_Float(array, offset1, len1, nativeBuffer), ANI_OUT_OF_RANGE);

    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_SetRegion_Float(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->Array_SetRegion_Float(array, offset3, len3, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionFloatTest, GetFloatArrayRegionErrorTests)
{
    ani_array_float array;
    ASSERT_EQ(env_->Array_New_Float(5U, &array), ANI_OK);
    const uint32_t bufferSize = 5U;
    ani_float nativeBuffer[bufferSize] = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4, TEST_VALUE_5};
    const ani_size offset1 = 0;
    const ani_size len1 = 1;
    ASSERT_EQ(env_->Array_GetRegion_Float(array, offset1, len1, nullptr), ANI_INVALID_ARGS);
    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_GetRegion_Float(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_GetRegion_Float(array, offset1, len1, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionFloatTest, GetRegionFloatTest)
{
    const auto array = static_cast<ani_array_float>(CallEtsFunction<ani_ref>("GetArray"));

    ani_float nativeBuffer[5U] = {0.0F};
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    const float epsilon = 1e-6;  // Define acceptable tolerance
    ASSERT_EQ(env_->Array_GetRegion_Float(array, offset3, len3, nativeBuffer), ANI_OK);
    ASSERT_NEAR(nativeBuffer[0U], TEST_VALUE_1, epsilon);
    ASSERT_NEAR(nativeBuffer[1U], TEST_VALUE_2, epsilon);
    ASSERT_NEAR(nativeBuffer[2U], TEST_VALUE_3, epsilon);
    ASSERT_NEAR(nativeBuffer[3U], TEST_VALUE_4, epsilon);
    ASSERT_NEAR(nativeBuffer[4U], TEST_VALUE_5, epsilon);
}

TEST_F(ArraySetGetRegionFloatTest, SetRegionFloatTest)
{
    const auto array = static_cast<ani_array_float>(CallEtsFunction<ani_ref>("GetArray"));
    ani_float nativeBuffer1[5U] = {TEST_UPDATE_1, TEST_UPDATE_2, TEST_UPDATE_3};
    const ani_size offset4 = 2;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->Array_SetRegion_Float(array, offset4, len4, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("CheckArray", array), ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
