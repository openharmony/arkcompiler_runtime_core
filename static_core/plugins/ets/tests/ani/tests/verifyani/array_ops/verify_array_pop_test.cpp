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

class ArrayPopTest : public VerifyAniTest {};

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

    for (ani_size i = 0U; i < length; ++i) {
        ani_object intObj;
        CreateInt(env, i, &intObj);
        ASSERT_EQ(env->Array_Set(*arr, i, intObj), ANI_OK);
    }
}

TEST_F(ArrayPopTest, wrong_env)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->c_api->Array_Pop(nullptr, arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_array"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, wrong_array)
{
    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(nullptr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, null_input_array)
{
    ani_array arr {};
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&arr)), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: null"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, undef_input_array)
{
    ani_array arr {};
    ASSERT_EQ(env_->GetUndefined(reinterpret_cast<ani_ref *>(&arr)), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: undefined"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, err_input_array)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("escompat.Error", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &ctor), ANI_OK);
    ani_ref undefinedArgument {};
    ASSERT_EQ(env_->GetUndefined(&undefinedArgument), ANI_OK);
    ani_object errObj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &errObj, undefinedArgument, undefinedArgument), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(static_cast<ani_array>(errObj), &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_error"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, obj_input_array)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &obj), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(static_cast<ani_array>(obj), &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_object"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, str_input_array)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(reinterpret_cast<ani_array>(stringObj), &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array", "wrong reference type: ani_string"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, wrong_res)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ASSERT_EQ(env_->Array_Pop(arr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"array", "ani_array"},
        {"result", "ani_ref", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Array_Pop(nullptr, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"array", "ani_array", "wrong reference"},
        {"result", "ani_ref", "wrong pointer for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, throw_error)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ThrowError();

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(arr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"array", "ani_array"},
        {"result", "ani_ref"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_Pop", testLines);
}

TEST_F(ArrayPopTest, success)
{
    ani_array arr {};
    ani_size length = 3U;
    CreateAndInitArray(env_, length, &arr);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Array_Pop(arr, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)