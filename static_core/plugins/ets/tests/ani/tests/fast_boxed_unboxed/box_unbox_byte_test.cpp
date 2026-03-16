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
class BoxUnboxByteTest : public AniTest {
protected:
    static constexpr ani_byte TEST_VALUE = 12;
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsByte(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std.core.Byte", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxByteNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Byte(INT8_MIN, &boxedMin), ANI_OK);
    AssertIsByte(env, boxedMin);
    ani_byte unboxedMin = 0;
    ASSERT_EQ(env->Primitive_Unbox_Byte(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, INT8_MIN);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Byte(INT8_MAX, &boxedMax), ANI_OK);
    AssertIsByte(env, boxedMax);
    ani_byte unboxedMax = 0;
    ASSERT_EQ(env->Primitive_Unbox_Byte(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, INT8_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Byte(0, &boxedZero), ANI_OK);
    AssertIsByte(env, boxedZero);
    ani_byte unboxedZero = -1;
    ASSERT_EQ(env->Primitive_Unbox_Byte(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, 0);

    ani_byte value = 12;
    ani_object boxedValue = nullptr;
    ASSERT_EQ(env->Primitive_Box_Byte(value, &boxedValue), ANI_OK);
    AssertIsByte(env, boxedValue);
    ani_byte unboxedValue = 0;
    ASSERT_EQ(env->Primitive_Unbox_Byte(boxedValue, &unboxedValue), ANI_OK);
    ASSERT_EQ(unboxedValue, value);
}

static void TestUnboxByteNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_byte_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field codeField;
    ASSERT_EQ(env->Class_FindField(cls, "code_optional", &codeField), ANI_OK);

    ani_ref codeRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, codeField, &codeRef), ANI_OK);

    ani_byte code = 0;
    ASSERT_EQ(env->Primitive_Unbox_Byte(static_cast<ani_object>(codeRef), &code), ANI_OK);
    ani_byte expectCode = 12;
    ASSERT_EQ(code, expectCode);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_byte arrVal = -1;
        ASSERT_EQ(env->Primitive_Unbox_Byte(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ani_byte expectVal = 12;
        ASSERT_EQ(arrVal, expectVal);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_byte val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Byte(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsByte(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxByteTest, boxed_byte)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_byte_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testBoxByte", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxByteNative", ":", reinterpret_cast<void *>(TestBoxByteNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxByteTest, unboxed_byte)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_byte_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testUnboxByte", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxByteNative", ":", reinterpret_cast<void *>(TestUnboxByteNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxByteTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_byte_test", &module), ANI_OK);
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
        ASSERT_EQ(env_->Primitive_Box_Byte(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsByte(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxByteTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_byte_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "b:C{std.core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "b:C{std.core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsByte(env_, static_cast<ani_object>(res));
    ani_byte unboxed = 0;
    ASSERT_EQ(env_->Primitive_Unbox_Byte(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxByteTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Byte(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Byte(TEST_VALUE, &obj), ANI_OK);
    AssertIsByte(env_, obj);
    ani_byte res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Byte(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxByteTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Byte(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_byte val;
    ASSERT_EQ(env_->Primitive_Unbox_Byte(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Byte(TEST_VALUE, &obj), ANI_OK);
    AssertIsByte(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Byte(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxByteTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_byte result;
    ASSERT_EQ(env_->Primitive_Unbox_Byte(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
