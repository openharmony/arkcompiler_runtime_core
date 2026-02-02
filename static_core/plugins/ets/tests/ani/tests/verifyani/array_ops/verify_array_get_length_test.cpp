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

class ArrayGetLengthTest : public VerifyAniTest {};

static void CreateArray(ani_env *env, ani_size length, ani_array *arr)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env->Array_New(length, undefinedRef, arr), ANI_OK);
}

TEST_F(ArrayGetLengthTest, wrong_env)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateArray(env_, length, &arr);

    ani_size res = -1;
    ASSERT_EQ(env_->c_api->Array_GetLength(nullptr, arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_array"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, wrong_result)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateArray(env_, length, &arr);

    ASSERT_EQ(env_->Array_GetLength(arr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, wrong_all_args)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateArray(env_, length, &arr);

    ASSERT_EQ(env_->c_api->Array_GetLength(nullptr, arr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_array"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, throw_error)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateArray(env_, length, &arr);

    ThrowError();

    ani_size res = -1;
    ASSERT_EQ(env_->Array_GetLength(arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"array", "ani_array"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, null_input_array)
{
    ani_array arr {};
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&arr)), ANI_OK);

    ani_size res;
    ASSERT_EQ(env_->Array_GetLength(arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: null"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, undef_input_array)
{
    ani_array arr {};
    ASSERT_EQ(env_->GetUndefined(reinterpret_cast<ani_ref *>(&arr)), ANI_OK);

    ani_size res;
    ASSERT_EQ(env_->Array_GetLength(arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: undefined"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, err_input_array)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("escompat.Error", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object errObj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &errObj), ANI_OK);

    ani_size res;
    ASSERT_EQ(env_->Array_GetLength(static_cast<ani_array>(errObj), &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_error"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, obj_input_array)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &obj), ANI_OK);

    ani_size res;
    ASSERT_EQ(env_->Array_GetLength(static_cast<ani_array>(obj), &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_object"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, str_input_array)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_size res;
    ASSERT_EQ(env_->Array_GetLength(reinterpret_cast<ani_array>(stringObj), &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_string"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_GetLength", testLines);
}

TEST_F(ArrayGetLengthTest, success)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateArray(env_, length, &arr);

    ani_size res = -1;
    ASSERT_EQ(env_->Array_GetLength(arr, &res), ANI_OK);
    ASSERT_EQ(res, length);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)