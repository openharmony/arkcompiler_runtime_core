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

#include "fixedarray_gtest_ops.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FixedArrayGetLengthTest : public AniGTestFixedArrayOps {};

TEST_F(FixedArrayGetLengthTest, GetLengthErrorTests)
{
    ani_size length = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(nullptr, &length), ANI_INVALID_ARGS);

    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_fixedarray array = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_5, undefinedRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FixedArrayGetLengthTest, GetLengthOkTests)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_fixedarray array = nullptr;
    ASSERT_EQ(env_->FixedArray_New(cls, LENGTH_5, undefinedRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);

    ani_size length = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(array, &length), ANI_OK);
    ASSERT_EQ(length, LENGTH_5);
}

TEST_F(FixedArrayGetLengthTest, GetLengthReferenceTypesTests)
{
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_class stringClass = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &stringClass), ANI_OK);
    ASSERT_NE(stringClass, nullptr);
    ani_fixedarray stringArray = nullptr;
    ASSERT_EQ(env_->FixedArray_New(stringClass, LENGTH_5, undefinedRef, &stringArray), ANI_OK);
    ASSERT_NE(stringArray, nullptr);
    ani_size length = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(stringArray, &length), ANI_OK);
    ASSERT_EQ(length, LENGTH_5);

    ani_class objectClass = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectClass), ANI_OK);
    ASSERT_NE(objectClass, nullptr);
    ani_fixedarray objectArray = nullptr;
    ASSERT_EQ(env_->FixedArray_New(objectClass, LENGTH_6, undefinedRef, &objectArray), ANI_OK);
    ASSERT_NE(objectArray, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(objectArray, &length), ANI_OK);
    ASSERT_EQ(length, LENGTH_6);

    ani_class errorClass = nullptr;
    ASSERT_EQ(env_->FindClass("escompat.Error", &errorClass), ANI_OK);
    ASSERT_NE(errorClass, nullptr);
    ani_fixedarray errorArray = nullptr;
    ASSERT_EQ(env_->FixedArray_New(errorClass, LENGTH_10, undefinedRef, &errorArray), ANI_OK);
    ASSERT_NE(errorArray, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(errorArray, &length), ANI_OK);
    ASSERT_EQ(length, LENGTH_10);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
