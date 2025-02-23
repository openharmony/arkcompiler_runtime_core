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

class ArraySetGetRegionIntTest : public AniTest {
protected:
    static constexpr ani_int TEST_VALUE1 = 1;
    static constexpr ani_int TEST_VALUE2 = 2;
    static constexpr ani_int TEST_VALUE3 = 3;
    static constexpr ani_int TEST_VALUE4 = 4;
    static constexpr ani_int TEST_VALUE5 = 5;

    static constexpr ani_int TEST_UPDATE1 = 30;
    static constexpr ani_int TEST_UPDATE2 = 40;
    static constexpr ani_int TEST_UPDATE3 = 50;
};

TEST_F(ArraySetGetRegionIntTest, SetIntArrayRegionErrorTests)
{
    ani_array_int array;
    ASSERT_EQ(env_->Array_New_Int(5U, &array), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_int nativeBuffer[bufferSize] = {0};
    const ani_size offset1 = -1;
    const ani_size len1 = 2;
    ASSERT_EQ(env_->Array_SetRegion_Int(array, offset1, len1, nativeBuffer), ANI_OUT_OF_RANGE);

    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_SetRegion_Int(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->Array_SetRegion_Int(array, offset3, len3, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionIntTest, GetIntArrayRegionErrorTests)
{
    ani_array_int array;
    ASSERT_EQ(env_->Array_New_Int(5U, &array), ANI_OK);
    const uint32_t bufferSize = 10U;
    ani_int nativeBuffer[bufferSize] = {0};
    const ani_size offset1 = 0;
    const ani_size len1 = 1;
    ASSERT_EQ(env_->Array_GetRegion_Int(array, offset1, len1, nullptr), ANI_INVALID_ARGS);
    const ani_size offset2 = 5;
    const ani_size len2 = 10U;
    ASSERT_EQ(env_->Array_GetRegion_Int(array, offset2, len2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_GetRegion_Int(array, offset1, len1, nativeBuffer), ANI_OK);
}

TEST_F(ArraySetGetRegionIntTest, GetRegionIntTest)
{
    const auto array = static_cast<ani_array_int>(CallEtsFunction<ani_ref>("GetArray"));

    ani_int nativeBuffer[5U] = {0};
    const ani_size offset3 = 0;
    const ani_size len3 = 5;
    ASSERT_EQ(env_->Array_GetRegion_Int(array, offset3, len3, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE5);
}

TEST_F(ArraySetGetRegionIntTest, SetRegionIntTest)
{
    const auto array = static_cast<ani_array_int>(CallEtsFunction<ani_ref>("GetArray"));
    ani_int nativeBuffer1[5U] = {TEST_UPDATE1, TEST_UPDATE2, TEST_UPDATE3};
    const ani_size offset4 = 2;
    const ani_size len4 = 3;
    ASSERT_EQ(env_->Array_SetRegion_Int(array, offset4, len4, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("CheckArray", array), ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
