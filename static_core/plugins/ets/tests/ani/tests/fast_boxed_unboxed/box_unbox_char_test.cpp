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
#include <climits>
#include <cstdint>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {
class BoxUnboxCharTest : public AniTest {
protected:
    static constexpr ani_char TEST_VALUE = 'A';
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsChar(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std:core.Char", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxCharNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Char(0, &boxedMin), ANI_OK);
    AssertIsChar(env, boxedMin);
    ani_char unboxedMin = 1;
    ASSERT_EQ(env->Primitive_Unbox_Char(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, 0);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Char(UINT16_MAX, &boxedMax), ANI_OK);
    AssertIsChar(env, boxedMax);
    ani_char unboxedMax = 0;
    ASSERT_EQ(env->Primitive_Unbox_Char(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, UINT16_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Char('\0', &boxedZero), ANI_OK);
    AssertIsChar(env, boxedZero);
    ani_char unboxedZero = 1;
    ASSERT_EQ(env->Primitive_Unbox_Char(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, '\0');

    ani_object boxedChar = nullptr;
    ASSERT_EQ(env->Primitive_Box_Char('A', &boxedChar), ANI_OK);
    AssertIsChar(env, boxedChar);
    ani_char unboxedChar = '\0';
    ASSERT_EQ(env->Primitive_Unbox_Char(boxedChar, &unboxedChar), ANI_OK);
    ASSERT_EQ(unboxedChar, 'A');
}

static void TestUnboxCharNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_char_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field msgField;
    ASSERT_EQ(env->Class_FindField(cls, "msg_optional", &msgField), ANI_OK);

    ani_ref msgRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, msgField, &msgRef), ANI_OK);

    ani_char msg = '0';
    ASSERT_EQ(env->Primitive_Unbox_Char(static_cast<ani_object>(msgRef), &msg), ANI_OK);
    ASSERT_EQ(msg, 'z');
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_char arrVal = '0';
        ASSERT_EQ(env->Primitive_Unbox_Char(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ASSERT_EQ(arrVal, 'A');
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_char val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Char(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsChar(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxCharTest, boxed_char)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_char_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testBoxChar", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxCharNative", ":", reinterpret_cast<void *>(TestBoxCharNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxCharTest, unboxed_char)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_char_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testUnboxChar", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxCharNative", ":", reinterpret_cast<void *>(TestUnboxCharNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxCharTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_char_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testPostArray", "C{std:core.Array}:", &etsFunc), ANI_OK);

    ani_native_function fn {"testPostArrayNative", "C{std:core.Array}:", reinterpret_cast<void *>(TestPostArrayNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_array arr;
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->Array_New(ARRAY_LENGTH, undefinedRef, &arr), ANI_OK);
    for (ani_size i = 0; i < ARRAY_LENGTH; ++i) {
        ani_object arrElement;
        ASSERT_EQ(env_->Primitive_Box_Char(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsChar(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxCharTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_char_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "c:C{std:core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "c:C{std:core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsChar(env_, static_cast<ani_object>(res));
    ani_char unboxed = '\0';
    ASSERT_EQ(env_->Primitive_Unbox_Char(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxCharTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Char(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Char(TEST_VALUE, &obj), ANI_OK);
    AssertIsChar(env_, obj);
    ani_char res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Char(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxCharTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Char(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_char val;
    ASSERT_EQ(env_->Primitive_Unbox_Char(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Char(TEST_VALUE, &obj), ANI_OK);
    AssertIsChar(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Char(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxCharTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_char result;
    ASSERT_EQ(env_->Primitive_Unbox_Char(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
