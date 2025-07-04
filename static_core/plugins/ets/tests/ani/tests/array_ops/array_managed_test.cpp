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

#include <limits>
#include "ani.h"
#include "array_gtest_helper.h"
#include "macros.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class ArrayManagedTest : public ArrayHelperTest {};

TEST_F(ArrayManagedTest, GetLengthTest)
{
    InitTest();

    const auto array1 = static_cast<ani_array>(CallEtsFunction<ani_ref>("array_managed_test", "getArray"));
    ani_size res1 {};
    ASSERT_EQ(env_->Array_GetLength(array1, &res1), ANI_OK);
    ASSERT_EQ(res1, 5U);

    const auto array2 = static_cast<ani_array>(CallEtsFunction<ani_ref>("array_managed_test", "getEmptyArray"));
    ani_size res2 {};
    ASSERT_EQ(env_->Array_GetLength(array2, &res2), ANI_OK);
    ASSERT_EQ(res2, 0);
}

TEST_F(ArrayManagedTest, GetSetTest)
{
    InitTest();

    ani_size arrSize = 5U;

    const auto array1 = static_cast<ani_array>(CallEtsFunction<ani_ref>("array_managed_test", "getArray"));
    ani_size res1 {};
    ASSERT_EQ(env_->Array_GetLength(array1, &res1), ANI_OK);
    ASSERT_EQ(res1, arrSize);

    ani_ref val {};
    ani_object boxedInt {};
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(array1, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it + 1U);

        ASSERT_EQ(env_->Object_New(intClass, intCtor, &boxedInt, (it + 1) * 10U), ANI_OK);
        ASSERT_EQ(env_->Array_Set(array1, it, boxedInt), ANI_OK);
    }
    const auto res = CallEtsFunction<ani_boolean>("array_managed_test", "checkArray", array1);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ArrayManagedTest, PushTest)
{
    InitTest();

    ani_size arrSize = 5U;

    const auto array1 = static_cast<ani_array>(CallEtsFunction<ani_ref>("array_managed_test", "getEmptyArray"));
    ani_size res1 {};
    ASSERT_EQ(env_->Array_GetLength(array1, &res1), ANI_OK);
    ASSERT_EQ(res1, 0U);

    ani_object boxedInt {};
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Object_New(intClass, intCtor, &boxedInt, (it + 1) * 10U), ANI_OK);
        ASSERT_EQ(env_->Array_Push(array1, boxedInt), ANI_OK);
    }
    const auto res = CallEtsFunction<ani_boolean>("array_managed_test", "checkArray", array1);
    ASSERT_EQ(res, ANI_TRUE);
}

TEST_F(ArrayManagedTest, PopTest)
{
    InitTest();

    ani_size arrSize = 5U;

    const auto array1 = static_cast<ani_array>(CallEtsFunction<ani_ref>("array_managed_test", "getArray"));
    ani_size res1 {};
    ASSERT_EQ(env_->Array_GetLength(array1, &res1), ANI_OK);
    ASSERT_EQ(res1, 5U);

    ani_ref boxedInt {};
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Pop(array1, &boxedInt), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(boxedInt), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, 5U - it);
    }
    const auto res = CallEtsFunction<ani_boolean>("array_managed_test", "checkEmptyArray", array1);
    ASSERT_EQ(res, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
