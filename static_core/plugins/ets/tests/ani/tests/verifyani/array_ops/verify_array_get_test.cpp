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

class ArrayGetTest : public VerifyAniTest {};

static void CreateInt(ani_env *env, ani_int val, ani_object *intObj)
{
    ani_class intClass;
    ASSERT_EQ(env->FindClass("std.core.Int", &intClass), ANI_OK);
    ani_method intCtor;
    ASSERT_EQ(env->Class_FindMethod(intClass, "<ctor>", "i:", &intCtor), ANI_OK);
    ASSERT_EQ(env->Object_New(intClass, intCtor, intObj, val), ANI_OK);
}

static void CreateAndInitArray(ani_env *env, ani_size length, ani_array *arr)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env->Array_New(length, undefinedRef, arr), ANI_OK);

    for (ani_size i = 0; i < length; ++i) {
        ani_object intObj;
        CreateInt(env, i, &intObj);
        ASSERT_EQ(env->Array_Set(*arr, i, intObj), ANI_OK);
    }
}

TEST_F(ArrayGetTest, wrong_env)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ani_size index = 1U;
    ani_ref res;
    ASSERT_EQ(env_->c_api->Array_Get(nullptr, arr, index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_array"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, wrong_array)
{
    ani_ref res;
    ani_size index = 2U;
    ASSERT_EQ(env_->Array_Get(nullptr, index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, null_input_array)
{
    ani_array arr {};
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&arr)), ANI_OK);

    ani_ref res;
    ani_size index = 2U;
    ASSERT_EQ(env_->Array_Get(arr, index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: null"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, undef_input_array)
{
    ani_array arr {};
    ASSERT_EQ(env_->GetUndefined(reinterpret_cast<ani_ref *>(&arr)), ANI_OK);

    ani_ref res;
    ani_size index = 2U;
    ASSERT_EQ(env_->Array_Get(arr, index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: undefined"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, err_input_array)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("escompat.Error", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &ctor), ANI_OK);
    ani_ref undefinedArgument {};
    ASSERT_EQ(env_->GetUndefined(&undefinedArgument), ANI_OK);
    ani_object errObj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &errObj, undefinedArgument, undefinedArgument), ANI_OK);

    ani_ref res;
    ani_size index = 2U;
    ASSERT_EQ(env_->Array_Get(static_cast<ani_array>(errObj), index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_error"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, obj_input_array)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &obj), ANI_OK);

    ani_ref res;
    ani_size index = 2U;
    ASSERT_EQ(env_->Array_Get(static_cast<ani_array>(obj), index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_object"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, str_input_array)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_ref res;
    ani_size index = 2U;
    ASSERT_EQ(env_->Array_Get(reinterpret_cast<ani_array>(stringObj), index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_string"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, wrong_res)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ani_size index = 2;
    ASSERT_EQ(env_->Array_Get(arr, index, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array"},
        {"index", "ani_size"},
        {"result", "ani_ref", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, wrong_all_args)
{
    ani_size index = -1U;
    ASSERT_EQ(env_->c_api->Array_Get(nullptr, nullptr, index, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_array", "wrong reference"},
        {"index", "ani_size", "no array context for index validation"},
        {"result", "ani_ref", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, throw_error)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ThrowError();

    ani_size index = 2U;
    ani_ref res;
    ASSERT_EQ(env_->Array_Get(arr, index, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"array", "ani_array"},
        {"index", "ani_size"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Get", testLines);
}

TEST_F(ArrayGetTest, index_out_of_bounds)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ani_ref res;
    ASSERT_EQ(env_->Array_Get(arr, length, &res), ANI_PENDING_ERROR);
}

TEST_F(ArrayGetTest, index_too_large)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ani_ref res;
    ASSERT_EQ(env_->Array_Get(arr, SIZE_MAX, &res), ANI_PENDING_ERROR);
}

TEST_F(ArrayGetTest, success)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    for (ani_size i = 0; i < length; ++i) {
        ani_ref res = nullptr;
        ASSERT_EQ(env_->Array_Get(arr, i, &res), ANI_OK);
        ASSERT_NE(res, nullptr);
    }
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)