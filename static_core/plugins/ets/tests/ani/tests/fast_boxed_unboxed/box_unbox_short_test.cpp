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
class BoxUnboxShortTest : public AniTest {
protected:
    static constexpr ani_short TEST_VALUE = 123;
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsShort(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std:core.Short", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxShortNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Short(INT16_MIN, &boxedMin), ANI_OK);
    AssertIsShort(env, boxedMin);
    ani_short unboxedMin = 0;
    ASSERT_EQ(env->Primitive_Unbox_Short(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, INT16_MIN);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Short(INT16_MAX, &boxedMax), ANI_OK);
    AssertIsShort(env, boxedMax);
    ani_short unboxedMax = 0;
    ASSERT_EQ(env->Primitive_Unbox_Short(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, INT16_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Short(0, &boxedZero), ANI_OK);
    AssertIsShort(env, boxedZero);
    ani_short unboxedZero = -1;
    ASSERT_EQ(env->Primitive_Unbox_Short(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, 0);

    ani_short value = 123;
    ani_object boxedValue = nullptr;
    ASSERT_EQ(env->Primitive_Box_Short(value, &boxedValue), ANI_OK);
    AssertIsShort(env, boxedValue);
    ani_short unboxedValue = 0;
    ASSERT_EQ(env->Primitive_Unbox_Short(boxedValue, &unboxedValue), ANI_OK);
    ASSERT_EQ(unboxedValue, value);
}

static void TestUnboxShortNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_short_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field scoreField;
    ASSERT_EQ(env->Class_FindField(cls, "score_optional", &scoreField), ANI_OK);

    ani_ref scoreRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, scoreField, &scoreRef), ANI_OK);

    ani_short score = 0;
    ASSERT_EQ(env->Primitive_Unbox_Short(static_cast<ani_object>(scoreRef), &score), ANI_OK);
    ani_short expectScore = 123;
    ASSERT_EQ(score, expectScore);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_short arrVal = -1;
        ASSERT_EQ(env->Primitive_Unbox_Short(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ani_short expectVal = 123;
        ASSERT_EQ(arrVal, expectVal);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_short val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Short(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsShort(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxShortTest, boxed_short)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_short_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testBoxShort", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxShortNative", ":", reinterpret_cast<void *>(TestBoxShortNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxShortTest, unboxed_short)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_short_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testUnboxShort", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxShortNative", ":", reinterpret_cast<void *>(TestUnboxShortNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxShortTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_short_test", &module), ANI_OK);
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
        ASSERT_EQ(env_->Primitive_Box_Short(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsShort(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxShortTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_short_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "s:C{std:core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "s:C{std:core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsShort(env_, static_cast<ani_object>(res));
    ani_short unboxed = 0;
    ASSERT_EQ(env_->Primitive_Unbox_Short(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxShortTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Short(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Short(TEST_VALUE, &obj), ANI_OK);
    AssertIsShort(env_, obj);
    ani_short res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Short(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxShortTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Short(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_short val;
    ASSERT_EQ(env_->Primitive_Unbox_Short(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Short(TEST_VALUE, &obj), ANI_OK);
    AssertIsShort(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Short(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxShortTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_short result;
    ASSERT_EQ(env_->Primitive_Unbox_Short(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
