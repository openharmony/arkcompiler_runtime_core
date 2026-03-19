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
class BoxUnboxIntTest : public AniTest {
protected:
    static constexpr ani_int TEST_VALUE = 12;
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsInt(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std:core.Int", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxIntNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Int(INT32_MIN, &boxedMin), ANI_OK);
    AssertIsInt(env, boxedMin);
    ani_int unboxedMin = 0;
    ASSERT_EQ(env->Primitive_Unbox_Int(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, INT32_MIN);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Int(INT32_MAX, &boxedMax), ANI_OK);
    AssertIsInt(env, boxedMax);
    ani_int unboxedMax = 0;
    ASSERT_EQ(env->Primitive_Unbox_Int(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, INT32_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Int(0, &boxedZero), ANI_OK);
    AssertIsInt(env, boxedZero);
    ani_int unboxedZero = -1;
    ASSERT_EQ(env->Primitive_Unbox_Int(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, 0);

    ani_int value = 12;
    ani_object boxedValue = nullptr;
    ASSERT_EQ(env->Primitive_Box_Int(value, &boxedValue), ANI_OK);
    AssertIsInt(env, boxedValue);
    ani_int unboxedValue = 0;
    ASSERT_EQ(env->Primitive_Unbox_Int(boxedValue, &unboxedValue), ANI_OK);
    ASSERT_EQ(unboxedValue, value);
}

static void TestUnboxIntNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_int_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field scoreField;
    ASSERT_EQ(env->Class_FindField(cls, "score_optional", &scoreField), ANI_OK);

    ani_ref scoreRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, scoreField, &scoreRef), ANI_OK);

    ani_int score = 0;
    ASSERT_EQ(env->Primitive_Unbox_Int(static_cast<ani_object>(scoreRef), &score), ANI_OK);
    ani_int expectScore = 1234;
    ASSERT_EQ(score, expectScore);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_int arrVal = -1;
        ASSERT_EQ(env->Primitive_Unbox_Int(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ani_int expectVal = 12;
        ASSERT_EQ(arrVal, expectVal);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_int val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Int(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsInt(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxIntTest, boxed_int)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_int_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testBoxInt", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxIntNative", ":", reinterpret_cast<void *>(TestBoxIntNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxIntTest, unboxed_int)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_int_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testUnboxInt", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxIntNative", ":", reinterpret_cast<void *>(TestUnboxIntNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxIntTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_int_test", &module), ANI_OK);
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
        ASSERT_EQ(env_->Primitive_Box_Int(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsInt(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxIntTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_int_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "i:C{std:core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "i:C{std:core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsInt(env_, static_cast<ani_object>(res));
    ani_int unboxed = 0;
    ASSERT_EQ(env_->Primitive_Unbox_Int(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxIntTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Int(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Int(TEST_VALUE, &obj), ANI_OK);
    AssertIsInt(env_, obj);
    ani_int res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Int(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxIntTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Int(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_int val;
    ASSERT_EQ(env_->Primitive_Unbox_Int(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Int(TEST_VALUE, &obj), ANI_OK);
    AssertIsInt(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Int(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxIntTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_int result;
    ASSERT_EQ(env_->Primitive_Unbox_Int(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
