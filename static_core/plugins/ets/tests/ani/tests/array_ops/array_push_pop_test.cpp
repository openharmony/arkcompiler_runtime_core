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

class ArrayGetSetTest : public ArrayHelperTest {};

TEST_F(ArrayGetSetTest, PushTest)
{
    InitTest();

    ani_size arrSize = 5U;
    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);

    for (ani_size it = 0; it < arrSize; ++it) {
        ani_object boxedInt {};
        env_->Object_New(intClass, intCtor, &boxedInt, it);
        ASSERT_EQ(env_->Array_Push(array, boxedInt), ANI_OK);
        ani_size len = -1;
        ASSERT_EQ(env_->Array_GetLength(array, &len), ANI_OK);
        ASSERT_EQ(len, it + 1U);

        ani_ref val {};
        ASSERT_EQ(env_->Array_Get(array, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }
}

TEST_F(ArrayGetSetTest, PopTest)
{
    InitTest();

    ani_size arrSize = 5U;
    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);

    for (ani_size it = 0; it < arrSize; ++it) {
        ani_object boxedInt {};
        env_->Object_New(intClass, intCtor, &boxedInt, it);
        ASSERT_EQ(env_->Array_Push(array, boxedInt), ANI_OK);
    }
    ani_size len = -1;
    ASSERT_EQ(env_->Array_GetLength(array, &len), ANI_OK);
    ASSERT_EQ(len, arrSize);

    for (ani_int it = arrSize - 1; it >= 0; --it) {
        ani_ref boxedInt {};
        ASSERT_EQ(env_->Array_Pop(array, &boxedInt), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(boxedInt), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }
    ASSERT_EQ(env_->Array_GetLength(array, &len), ANI_OK);
    ASSERT_EQ(len, 0);
}

TEST_F(ArrayGetSetTest, PushPopInvalidArgsTest)
{
    InitTest();

    ani_object boxedInt {};
    env_->Object_New(intClass, intCtor, &boxedInt, 0);
    ASSERT_EQ(env_->Array_Push(nullptr, boxedInt), ANI_INVALID_ARGS);
    ani_ref res {};
    ASSERT_EQ(env_->Array_Pop(nullptr, &res), ANI_INVALID_ARGS);

    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);
    ASSERT_EQ(env_->Array_Pop(array, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
