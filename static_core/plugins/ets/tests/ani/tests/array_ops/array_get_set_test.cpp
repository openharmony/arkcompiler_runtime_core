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

TEST_F(ArrayGetSetTest, GetTest)
{
    InitTest();

    ani_size arrSize = 5U;

    ani_array array {};
    ASSERT_EQ(env_->Array_New(arrSize, nullptr, &array), ANI_OK);
    ani_ref val {};
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(array, it, &val), ANI_OK);
        ani_boolean res = ANI_FALSE;
        ASSERT_EQ(env_->Reference_IsUndefined(val, &res), ANI_OK);
        ASSERT_EQ(res, ANI_TRUE);
    }

    ani_object boxedBoolean {};
    env_->Object_New(booleanClass, booleanCtor, &boxedBoolean, ANI_TRUE);
    ani_array arrayBoolean {};
    ASSERT_EQ(env_->Array_New(arrSize, boxedBoolean, &arrayBoolean), ANI_OK);
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(arrayBoolean, it, &val), ANI_OK);
        ani_boolean unboxedBoolean = ANI_FALSE;
        ASSERT_EQ(env_->Object_CallMethod_Boolean(reinterpret_cast<ani_object>(val), booleanUnbox, &unboxedBoolean),
                  ANI_OK);
        ASSERT_EQ(unboxedBoolean, ANI_TRUE);
    }

    ani_object boxedInt {};
    // NOLINTNEXTLINE(readability-magic-numbers)
    env_->Object_New(intClass, intCtor, &boxedInt, 10U);
    ani_array arrayInt {};
    ASSERT_EQ(env_->Array_New(arrSize, boxedInt, &arrayInt), ANI_OK);
    for (ani_size it = 0; it < arrSize; ++it) {
        ASSERT_EQ(env_->Array_Get(arrayInt, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, 10U);
    }
}

TEST_F(ArrayGetSetTest, SetTest)
{
    InitTest();

    ani_size arrSize = 5U;

    ani_object boxedInt {};

    ani_array array {};
    ASSERT_EQ(env_->Array_New(arrSize, nullptr, &array), ANI_OK);
    ani_ref val {};
    for (ani_size it = 0; it < arrSize; ++it) {
        env_->Object_New(intClass, intCtor, &boxedInt, it);
        ASSERT_EQ(env_->Array_Set(array, it, boxedInt), ANI_OK);
        ASSERT_EQ(env_->Array_Get(array, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }

    ani_object boxedInt2 {};
    ani_array arrayInt {};
    ASSERT_EQ(env_->Array_New(arrSize, boxedInt, &arrayInt), ANI_OK);
    for (ani_size it = 0; it < arrSize; ++it) {
        env_->Object_New(intClass, intCtor, &boxedInt2, it);
        ASSERT_EQ(env_->Array_Set(arrayInt, it, boxedInt2), ANI_OK);
        ASSERT_EQ(env_->Array_Get(arrayInt, it, &val), ANI_OK);
        ani_int unboxedInt = -1;
        ASSERT_EQ(env_->Object_CallMethod_Int(reinterpret_cast<ani_object>(val), intUnbox, &unboxedInt), ANI_OK);
        ASSERT_EQ(unboxedInt, it);
    }
}

TEST_F(ArrayGetSetTest, GetSetInvalidArgsTest)
{
    InitTest();

    ani_object boxedInt {};
    env_->Object_New(intClass, intCtor, &boxedInt, 0);
    ASSERT_EQ(env_->Array_Set(nullptr, 0, boxedInt), ANI_INVALID_ARGS);
    ani_ref res {};
    ASSERT_EQ(env_->Array_Get(nullptr, 0, &res), ANI_INVALID_ARGS);

    ani_array array {};
    ASSERT_EQ(env_->Array_New(0U, nullptr, &array), ANI_OK);
    ASSERT_EQ(env_->Array_Get(array, 0, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Array_Set(array, 0, res), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->Array_Get(array, 0, &res), ANI_OUT_OF_RANGE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
