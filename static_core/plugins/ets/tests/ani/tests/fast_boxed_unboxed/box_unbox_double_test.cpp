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
#include <cmath>
#include <cfloat>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {
class BoxUnboxDoubleTest : public AniTest {
protected:
    static constexpr ani_double TEST_VALUE = 1.23;
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsDouble(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std.core.Double", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxDoubleNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Double(-DBL_MAX, &boxedMin), ANI_OK);
    AssertIsDouble(env, boxedMin);
    ani_double unboxedMin = 0.0;
    ASSERT_EQ(env->Primitive_Unbox_Double(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, -DBL_MAX);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Double(DBL_MAX, &boxedMax), ANI_OK);
    AssertIsDouble(env, boxedMax);
    ani_double unboxedMax = 0.0;
    ASSERT_EQ(env->Primitive_Unbox_Double(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, DBL_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Double(0.0, &boxedZero), ANI_OK);
    AssertIsDouble(env, boxedZero);
    ani_double unboxedZero = -1.0;
    ASSERT_EQ(env->Primitive_Unbox_Double(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, 0.0);

    ani_double value = 1.23;
    ani_object boxedValue = nullptr;
    ASSERT_EQ(env->Primitive_Box_Double(value, &boxedValue), ANI_OK);
    AssertIsDouble(env, boxedValue);
    ani_double unboxedValue = 0.0;
    ASSERT_EQ(env->Primitive_Unbox_Double(boxedValue, &unboxedValue), ANI_OK);
    ASSERT_EQ(unboxedValue, value);
}

static void TestUnboxDoubleNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_double_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field scoreField;
    ASSERT_EQ(env->Class_FindField(cls, "score_optional", &scoreField), ANI_OK);

    ani_ref scoreRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, scoreField, &scoreRef), ANI_OK);

    ani_double score = 0.0;
    ASSERT_EQ(env->Primitive_Unbox_Double(static_cast<ani_object>(scoreRef), &score), ANI_OK);
    ani_double expectScore = 1.23;
    ASSERT_EQ(score, expectScore);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_double arrVal = -1;
        ASSERT_EQ(env->Primitive_Unbox_Double(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ani_double expectVal = 1.23;
        ASSERT_EQ(arrVal, expectVal);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_double val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Double(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsDouble(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxDoubleTest, boxed_double)
{
    ani_module ns = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_double_test", &ns), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(ns, "testBoxDouble", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxDoubleNative", ":", reinterpret_cast<void *>(TestBoxDoubleNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(ns, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxDoubleTest, unboxed_double)
{
    ani_module ns = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_double_test", &ns), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(ns, "testUnboxDouble", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxDoubleNative", ":", reinterpret_cast<void *>(TestUnboxDoubleNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(ns, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxDoubleTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_double_test", &module), ANI_OK);
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
        ASSERT_EQ(env_->Primitive_Box_Double(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsDouble(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxDoubleTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_double_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "d:C{std.core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "d:C{std.core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsDouble(env_, static_cast<ani_object>(res));
    ani_double unboxed = 0.0;
    ASSERT_EQ(env_->Primitive_Unbox_Double(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxDoubleTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Double(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Double(TEST_VALUE, &obj), ANI_OK);
    AssertIsDouble(env_, obj);
    ani_double res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Double(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxDoubleTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Double(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_double val;
    ASSERT_EQ(env_->Primitive_Unbox_Double(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Double(TEST_VALUE, &obj), ANI_OK);
    AssertIsDouble(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Double(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxDoubleTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_double result;
    ASSERT_EQ(env_->Primitive_Unbox_Double(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
