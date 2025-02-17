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

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArraySetRefTest : public AniTest {};

// ninja ani_test_array_setref_gtests
TEST_F(ArraySetRefTest, SetRefErrorTests)
{
    ani_array_ref array = nullptr;
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("Lstd/core/String;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    const ani_size length = 3;
    ASSERT_EQ(env_->Array_New_Ref(cls, length, nullptr, &array), ANI_OK);
    ani_ref ref = nullptr;
    const ani_size index = 0;
    const ani_size invalidIndex = 5;
    ASSERT_EQ(env_->Array_Set_Ref(nullptr, index, ref), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_Set_Ref(array, invalidIndex, ref), ANI_OUT_OF_RANGE);

    auto num = static_cast<ani_ref>(CallEtsFunction<ani_ref>("GetNumber"));
    ASSERT_EQ(env_->Array_Set_Ref(array, 0, num), ANI_INVALID_TYPE);
}

TEST_F(ArraySetRefTest, SetRefOkTests)
{
    auto array = static_cast<ani_array_ref>(CallEtsFunction<ani_ref>("GetArray"));

    auto newValue1 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("GetNewString1"));
    const ani_size index1 = 0;
    ASSERT_EQ(env_->Array_Set_Ref(array, index1, newValue1), ANI_OK);

    auto newValue2 = static_cast<ani_ref>(CallEtsFunction<ani_ref>("GetNewString2"));
    const ani_size index2 = 2;
    ASSERT_EQ(env_->Array_Set_Ref(array, index2, newValue2), ANI_OK);

    ani_boolean result = static_cast<ani_boolean>(CallEtsFunction<ani_boolean>("CheckArray", array));
    ASSERT_EQ(result, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
