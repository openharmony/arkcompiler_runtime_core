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

class FixedArrayGetTest : public VerifyAniTest {};

static void CreateString(ani_env *env, ani_string *str)
{
    ASSERT_EQ(env->String_NewUTF8("test", sizeof("test") - 1U, str), ANI_OK);
}

static void CreateStringFixedArray(ani_env *env, ani_fixedarray *array)
{
    ani_class stringClass {};
    ASSERT_EQ(env->FindClass("std.core.String", &stringClass), ANI_OK);
    ani_string initialElement {};
    CreateString(env, &initialElement);
    ASSERT_EQ(env->FixedArray_New(stringClass, LENGTH, initialElement, array), ANI_OK);
}

TEST_F(FixedArrayGetTest, wrong_env)
{
    ani_fixedarray array {};
    CreateStringFixedArray(env_, &array);

    ani_ref result {};
    ASSERT_EQ(env_->c_api->FixedArray_Get(nullptr, array, 0U, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"array", "ani_fixedarray"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get", testLines);
}

TEST_F(FixedArrayGetTest, wrong_array)
{
    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get(nullptr, 0U, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_fixedarray", "reference is nullptr [ERROR]"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get", testLines);
}

TEST_F(FixedArrayGetTest, wrong_array_type)
{
    ani_valuearray_int array {};
    ASSERT_EQ(env_->ValueArray_New_Int(LENGTH, &array), ANI_OK);

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get(reinterpret_cast<ani_fixedarray>(array), 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_fixedarray", "wrong reference type: ani_valuearray_int, expected: ani_fixedarray [FATAL]"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get", testLines);
}

TEST_F(FixedArrayGetTest, wrong_result)
{
    ani_fixedarray array {};
    CreateStringFixedArray(env_, &array);

    ASSERT_EQ(env_->FixedArray_Get(array, 0U, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_fixedarray"},
        {"index", "ani_size"},
        {"result", "ani_ref", "nullptr for storing 'ani_ref' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get", testLines);
}

TEST_F(FixedArrayGetTest, out_of_range)
{
    ani_fixedarray array {};
    CreateStringFixedArray(env_, &array);

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get(array, LENGTH, &result), ANI_OUT_OF_RANGE);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(FixedArrayGetTest, throw_error)
{
    ani_fixedarray array {};
    CreateStringFixedArray(env_, &array);

    ThrowError();

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get(array, 0U, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"array", "ani_fixedarray"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_Get", testLines);
}

TEST_F(FixedArrayGetTest, success)
{
    ani_fixedarray array {};
    CreateStringFixedArray(env_, &array);

    ani_ref result {};
    ASSERT_EQ(env_->FixedArray_Get(array, 0U, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
