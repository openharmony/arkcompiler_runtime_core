/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "verify_ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
namespace ark::ets::ani::verify::testing {

static constexpr ani_size LENGTH = 3U;

class FixedArrayGetRefTest : public VerifyAniTest {};

static void CreateString(ani_env *env, ani_string *str)
{
    ASSERT_EQ(env->String_NewUTF8("test", sizeof("test") - 1U, str), ANI_OK);
}

static void CreateStringFixedArray(ani_env *env, ani_fixedarray_ref *array)
{
    ani_class stringClass {};
    ASSERT_EQ(env->FindClass("std.core.String", &stringClass), ANI_OK);
    ani_string initialElement {};
    CreateString(env, &initialElement);
    ASSERT_EQ(env->FixedArray_New_Ref(stringClass, LENGTH, initialElement, array), ANI_OK);
}

TEST_F(FixedArrayGetRefTest, wrong_env)
{
    ani_fixedarray_ref array {};
    CreateStringFixedArray(env_, &array);

    ani_ref result {};
    ASSERT_EQ(env_->c_api->FixedArray_Get_Ref(nullptr, array, 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_fixedarray_ref"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get_Ref", testLines);
}

TEST_F(FixedArrayGetRefTest, wrong_array)
{
    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get_Ref(nullptr, 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_fixedarray_ref", "wrong reference"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get_Ref", testLines);
}

TEST_F(FixedArrayGetRefTest, wrong_result)
{
    ani_fixedarray_ref array {};
    CreateStringFixedArray(env_, &array);

    ASSERT_EQ(env_->FixedArray_Get_Ref(array, 0U, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_fixedarray_ref"},
        {"index", "ani_size"},
        {"result", "ani_ref", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get_Ref", testLines);
}

TEST_F(FixedArrayGetRefTest, out_of_range)
{
    ani_fixedarray_ref array {};
    CreateStringFixedArray(env_, &array);

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get_Ref(array, LENGTH, &result), ANI_OUT_OF_RANGE);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(FixedArrayGetRefTest, throw_error)
{
    ani_fixedarray_ref array {};
    CreateStringFixedArray(env_, &array);

    ThrowError();

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get_Ref(array, 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"array", "ani_fixedarray_ref"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get_Ref", testLines);
}

TEST_F(FixedArrayGetRefTest, success)
{
    ani_fixedarray_ref array {};
    CreateStringFixedArray(env_, &array);

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get_Ref(array, 0U, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
