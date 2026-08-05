/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "valuearray_gtest_ops.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ValueArrayNewTest : public AniGTestValueArrayOps {
protected:
    static constexpr ani_size ARRAYSIZE_10K = 10240U;
    static constexpr ani_size ARRAYSIZE_100K = 102400U;
};

TEST_F(ValueArrayNewTest, NewLargeArrayTypesTest)
{
    ani_valuearray_boolean array = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Boolean(ARRAYSIZE_10K, &array), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Boolean(ARRAYSIZE_100K, &array), ANI_OK);
    ani_valuearray_char array2 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Char(ARRAYSIZE_10K, &array2), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Char(ARRAYSIZE_100K, &array2), ANI_OK);
    ani_valuearray_byte array3 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Byte(ARRAYSIZE_10K, &array3), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Byte(ARRAYSIZE_100K, &array3), ANI_OK);
    ani_valuearray_short array4 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Short(ARRAYSIZE_10K, &array4), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Short(ARRAYSIZE_100K, &array4), ANI_OK);
    ani_valuearray_int array5 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Int(ARRAYSIZE_10K, &array5), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Int(ARRAYSIZE_100K, &array5), ANI_OK);
    ani_valuearray_long array6 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Long(ARRAYSIZE_10K, &array6), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Long(ARRAYSIZE_100K, &array6), ANI_OK);
    ani_valuearray_float array7 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Float(ARRAYSIZE_10K, &array7), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Float(ARRAYSIZE_100K, &array7), ANI_OK);
    ani_valuearray_double array8 = nullptr;
    ASSERT_EQ(env_->ValueArray_New_Double(ARRAYSIZE_10K, &array8), ANI_OK);
    ASSERT_EQ(env_->ValueArray_New_Double(ARRAYSIZE_100K, &array8), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
