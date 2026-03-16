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
class BoxUnboxLongTest : public AniTest {
protected:
    static constexpr ani_long TEST_VALUE = 123456789;
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsLong(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std.core.Long", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxLongNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Long(INT64_MIN, &boxedMin), ANI_OK);
    AssertIsLong(env, boxedMin);
    ani_long unboxedMin = 0;
    ASSERT_EQ(env->Primitive_Unbox_Long(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, INT64_MIN);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Long(INT64_MAX, &boxedMax), ANI_OK);
    AssertIsLong(env, boxedMax);
    ani_long unboxedMax = 0;
    ASSERT_EQ(env->Primitive_Unbox_Long(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, INT64_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Long(0, &boxedZero), ANI_OK);
    AssertIsLong(env, boxedZero);
    ani_long unboxedZero = -1;
    ASSERT_EQ(env->Primitive_Unbox_Long(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, 0);

    ani_long value = 123456789;
    ani_object boxedValue = nullptr;
    ASSERT_EQ(env->Primitive_Box_Long(value, &boxedValue), ANI_OK);
    AssertIsLong(env, boxedValue);
    ani_long unboxedValue = 0;
    ASSERT_EQ(env->Primitive_Unbox_Long(boxedValue, &unboxedValue), ANI_OK);
    ASSERT_EQ(unboxedValue, value);
}

static void TestUnboxLongNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_long_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field scoreField;
    ASSERT_EQ(env->Class_FindField(cls, "score_optional", &scoreField), ANI_OK);

    ani_ref scoreRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, scoreField, &scoreRef), ANI_OK);

    ani_long score = 0;
    ASSERT_EQ(env->Primitive_Unbox_Long(static_cast<ani_object>(scoreRef), &score), ANI_OK);
    ani_long expectScore = 123456789;
    ASSERT_EQ(score, expectScore);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_long arrVal = -1;
        ASSERT_EQ(env->Primitive_Unbox_Long(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ani_long expectVal = 123456789;
        ASSERT_EQ(arrVal, expectVal);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_long val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Long(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsLong(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxLongTest, boxed_long)
{
    ani_module ns = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_long_test", &ns), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(ns, "testBoxLong", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxLongNative", ":", reinterpret_cast<void *>(TestBoxLongNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(ns, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxLongTest, unboxed_long)
{
    ani_module ns = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_long_test", &ns), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(ns, "testUnboxLong", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxLongNative", ":", reinterpret_cast<void *>(TestUnboxLongNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(ns, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxLongTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_long_test", &module), ANI_OK);
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
        ASSERT_EQ(env_->Primitive_Box_Long(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsLong(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxLongTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_long_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "l:C{std.core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "l:C{std.core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsLong(env_, static_cast<ani_object>(res));
    ani_long unboxed = 0;
    ASSERT_EQ(env_->Primitive_Unbox_Long(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxLongTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Long(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Long(TEST_VALUE, &obj), ANI_OK);
    AssertIsLong(env_, obj);
    ani_long res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Long(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxLongTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Long(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_long val;
    ASSERT_EQ(env_->Primitive_Unbox_Long(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Long(TEST_VALUE, &obj), ANI_OK);
    AssertIsLong(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Long(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxLongTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_long result;
    ASSERT_EQ(env_->Primitive_Unbox_Long(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
