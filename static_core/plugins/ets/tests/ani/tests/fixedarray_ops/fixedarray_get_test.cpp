/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class FixedArrayGetTest : public AniTest {};

// ninja ani_test_fixedarray_get_gtests
TEST_F(FixedArrayGetTest, GetErrorTests)
{
    auto array = static_cast<ani_fixedarray>(CallEtsFunction<ani_ref>("fixedarray_get_test", "getArray"));
    ani_ref ref = nullptr;
    const ani_size index = 0;
    const ani_size invalidIndex = 5;
    ASSERT_EQ(env_->FixedArray_Get(nullptr, index, &ref), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_Get(array, index, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_Get(array, invalidIndex, &ref), ANI_OUT_OF_RANGE);
}

TEST_F(FixedArrayGetTest, GetOkTests)
{
    auto array = static_cast<ani_fixedarray>(CallEtsFunction<ani_ref>("fixedarray_get_test", "getArray"));
    const ani_size index1 = 1;
    const ani_size index2 = 2;
    ani_ref ref1 = nullptr;
    ani_ref ref2 = nullptr;
    ani_boolean isNull;
    ASSERT_EQ(env_->FixedArray_Get(array, index1, &ref1), ANI_OK);
    ASSERT_EQ(env_->FixedArray_Get(array, index2, &ref2), ANI_OK);
    ASSERT_EQ(env_->Reference_IsNull(ref1, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_TRUE);
    ASSERT_EQ(env_->Reference_IsNull(ref2, &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
