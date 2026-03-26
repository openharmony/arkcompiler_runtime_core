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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {
class BoxUnboxBooleanTest : public AniTest {
protected:
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsBoolean(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std.core.Boolean", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxBooleanNative(ani_env *env)
{
    ani_object boxedBool = nullptr;
    ASSERT_EQ(env->Primitive_Box_Boolean(ANI_TRUE, &boxedBool), ANI_OK);
    AssertIsBoolean(env, boxedBool);
    ani_boolean unboxedBool = ANI_FALSE;
    ASSERT_EQ(env->Primitive_Unbox_Boolean(boxedBool, &unboxedBool), ANI_OK);
    ASSERT_EQ(unboxedBool, ANI_TRUE);
}

static void TestUnboxBooleanNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_boolean_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field genderField;
    ASSERT_EQ(env->Class_FindField(cls, "gender_optional", &genderField), ANI_OK);

    ani_ref genderRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, genderField, &genderRef), ANI_OK);

    ani_boolean gender = ANI_TRUE;
    ASSERT_EQ(env->Primitive_Unbox_Boolean(static_cast<ani_object>(genderRef), &gender), ANI_OK);
    ASSERT_EQ(gender, ANI_FALSE);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_boolean arrVal = ANI_FALSE;
        ASSERT_EQ(env->Primitive_Unbox_Boolean(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ASSERT_EQ(arrVal, ANI_TRUE);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_boolean val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Boolean(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsBoolean(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxBooleanTest, boxed_boolean)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("box_unbox_boolean_test", &module), ANI_OK);
    ani_function etsFunc {};
    ASSERT_EQ(env_->Module_FindFunction(module, "testBoxBoolean", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxBooleanNative", ":", reinterpret_cast<void *>(TestBoxBooleanNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxBooleanTest, unboxed_boolean)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("box_unbox_boolean_test", &module), ANI_OK);
    ani_function etsFunc {};
    ASSERT_EQ(env_->Module_FindFunction(module, "testUnboxBoolean", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxBooleanNative", ":", reinterpret_cast<void *>(TestUnboxBooleanNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxBooleanTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_boolean_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testPostArray", "C{std.core.Array}:", &etsFunc), ANI_OK);

    ani_native_function fn {"testPostArrayNative", "C{std.core.Array}:", reinterpret_cast<void *>(TestPostArrayNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_array arr;
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->Array_New(ARRAY_LENGTH, undefinedRef, &arr), ANI_OK);
    for (ani_size i = 0; i < ARRAY_LENGTH; ++i) {
        ani_object arrElement;
        ASSERT_EQ(env_->Primitive_Box_Boolean(ANI_TRUE, &arrElement), ANI_OK);
        AssertIsBoolean(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxBooleanTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_boolean_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "z:C{std.core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "z:C{std.core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, ANI_TRUE), ANI_OK);
    AssertIsBoolean(env_, static_cast<ani_object>(res));
    ani_boolean unboxed = ANI_FALSE;
    ASSERT_EQ(env_->Primitive_Unbox_Boolean(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, ANI_TRUE);
}

TEST_F(BoxUnboxBooleanTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Boolean(nullptr, ANI_TRUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Boolean(ANI_TRUE, &obj), ANI_OK);
    AssertIsBoolean(env_, obj);
    ani_boolean res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Boolean(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxBooleanTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Boolean(ANI_TRUE, nullptr), ANI_INVALID_ARGS);

    ani_boolean val;
    ASSERT_EQ(env_->Primitive_Unbox_Boolean(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Boolean(ANI_TRUE, &obj), ANI_OK);
    AssertIsBoolean(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Boolean(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxBooleanTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_boolean result;
    ASSERT_EQ(env_->Primitive_Unbox_Boolean(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
