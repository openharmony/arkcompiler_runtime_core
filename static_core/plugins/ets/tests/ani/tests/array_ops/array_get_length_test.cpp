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

class ArrayGetLengthTest : public AniTest {};

TEST_F(ArrayGetLengthTest, GetLengthErrorTests)
{
    ani_array_byte array;
    ASSERT_EQ(env_->Array_New_Byte(5U, &array), ANI_OK);
    ani_size length = 0;
    ASSERT_EQ(env_->Array_GetLength(nullptr, &length), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Array_GetLength(array, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ArrayGetLengthTest, GetLengthOkTests)
{
    ani_array_byte array;
    const ani_size arraySize = 5U;
    ASSERT_EQ(env_->Array_New_Byte(arraySize, &array), ANI_OK);
    ani_size length = 0;
    ASSERT_EQ(env_->Array_GetLength(array, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
