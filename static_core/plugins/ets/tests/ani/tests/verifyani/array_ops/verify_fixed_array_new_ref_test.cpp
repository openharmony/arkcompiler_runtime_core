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

class FixedArrayNewRefTest : public VerifyAniTest {
protected:
    static constexpr ani_size LENGTH = 3U;
};

static void GetObjectClass(ani_env *env, ani_class *cls)
{
    ASSERT_EQ(env->FindClass("std.core.Object", cls), ANI_OK);
}

static void GetStringClass(ani_env *env, ani_class *cls)
{
    ASSERT_EQ(env->FindClass("std.core.String", cls), ANI_OK);
}

static void CreateObject(ani_env *env, ani_object *object)
{
    ani_class cls {};
    GetObjectClass(env, &cls);
    ani_method ctor {};
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ASSERT_EQ(env->Object_New(cls, ctor, object), ANI_OK);
}

static void CreateString(ani_env *env, ani_string *str)
{
    ASSERT_EQ(env->String_NewUTF8("test", sizeof("test") - 1U, str), ANI_OK);
}

TEST_F(FixedArrayNewRefTest, wrong_env)
{
    ani_class arrayType {};
    GetObjectClass(env_, &arrayType);
    ani_ref initialElement {};
    ASSERT_EQ(env_->GetUndefined(&initialElement), ANI_OK);

    ani_fixedarray_ref result {};
    ASSERT_EQ(env_->c_api->FixedArray_New_Ref(nullptr, arrayType, LENGTH, initialElement, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope [ERROR]"},
        {"type", "ani_ref"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_fixedarray_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Ref", testLines);
}

TEST_F(FixedArrayNewRefTest, wrong_type)
{
    ani_string initialElement {};
    CreateString(env_, &initialElement);

    ani_fixedarray_ref result {};
    ASSERT_EQ(env_->FixedArray_New_Ref(nullptr, LENGTH, initialElement, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref", "reference is nullptr [ERROR]"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_fixedarray_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Ref", testLines);
}

TEST_F(FixedArrayNewRefTest, wrong_initial_element)
{
    ani_class arrayType {};
    GetObjectClass(env_, &arrayType);

    ani_fixedarray_ref result {};
    ASSERT_EQ(env_->FixedArray_New_Ref(arrayType, LENGTH, nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref", "initial element is null [ERROR]"},
        {"result", "ani_fixedarray_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Ref", testLines);
}

TEST_F(FixedArrayNewRefTest, wrong_result)
{
    ani_class arrayType {};
    GetObjectClass(env_, &arrayType);
    ani_ref initialElement {};
    ASSERT_EQ(env_->GetUndefined(&initialElement), ANI_OK);

    ASSERT_EQ(env_->FixedArray_New_Ref(arrayType, LENGTH, initialElement, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_fixedarray_ref *", "nullptr for storing 'ani_fixedarray_ref' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Ref", testLines);
}

TEST_F(FixedArrayNewRefTest, wrong_initial_element_type)
{
    ani_class arrayType {};
    GetStringClass(env_, &arrayType);
    ani_object object {};
    CreateObject(env_, &object);

    ani_fixedarray_ref result {};
    ASSERT_EQ(env_->FixedArray_New_Ref(arrayType, LENGTH, object, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref",
         "wrong reference type: ani_object, expected: fixed array component type [FATAL]"},
        {"result", "ani_fixedarray_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Ref", testLines);
}

TEST_F(FixedArrayNewRefTest, throw_error)
{
    ani_class arrayType {};
    GetObjectClass(env_, &arrayType);
    ani_ref initialElement {};
    ASSERT_EQ(env_->GetUndefined(&initialElement), ANI_OK);

    ThrowError();

    ani_fixedarray_ref result {};
    ASSERT_EQ(env_->FixedArray_New_Ref(arrayType, LENGTH, initialElement, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error [ERROR]"},
        {"type", "ani_ref"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_fixedarray_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FixedArray_New_Ref", testLines);
}

TEST_F(FixedArrayNewRefTest, success)
{
    ani_class arrayType {};
    GetStringClass(env_, &arrayType);
    ani_string initialElement {};
    CreateString(env_, &initialElement);

    ani_fixedarray_ref result {};
    ASSERT_EQ(env_->FixedArray_New_Ref(arrayType, LENGTH, initialElement, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
