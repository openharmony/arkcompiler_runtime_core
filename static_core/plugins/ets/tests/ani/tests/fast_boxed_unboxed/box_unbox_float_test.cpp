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
class BoxUnboxFloatTest : public AniTest {
protected:
    static constexpr ani_float TEST_VALUE = 1.1F;
    static constexpr ani_size ARRAY_LENGTH = 5;
};

static void AssertIsFloat(ani_env *env, ani_object boxed)
{
    ani_class typeClass = nullptr;
    ASSERT_EQ(env->FindClass("std.core.Float", &typeClass), ANI_OK);

    ani_boolean isExpectedType = ANI_FALSE;
    ASSERT_EQ(env->Object_InstanceOf(boxed, typeClass, &isExpectedType), ANI_OK);
    ASSERT_EQ(isExpectedType, ANI_TRUE);
}

static void TestBoxFloatNative(ani_env *env)
{
    ani_object boxedMin = nullptr;
    ASSERT_EQ(env->Primitive_Box_Float(-FLT_MAX, &boxedMin), ANI_OK);
    AssertIsFloat(env, boxedMin);
    ani_float unboxedMin = 0.0F;
    ASSERT_EQ(env->Primitive_Unbox_Float(boxedMin, &unboxedMin), ANI_OK);
    ASSERT_EQ(unboxedMin, -FLT_MAX);

    ani_object boxedMax = nullptr;
    ASSERT_EQ(env->Primitive_Box_Float(FLT_MAX, &boxedMax), ANI_OK);
    AssertIsFloat(env, boxedMax);
    ani_float unboxedMax = 0.0F;
    ASSERT_EQ(env->Primitive_Unbox_Float(boxedMax, &unboxedMax), ANI_OK);
    ASSERT_EQ(unboxedMax, FLT_MAX);

    ani_object boxedZero = nullptr;
    ASSERT_EQ(env->Primitive_Box_Float(0.0F, &boxedZero), ANI_OK);
    AssertIsFloat(env, boxedZero);
    ani_float unboxedZero = -1.0F;
    ASSERT_EQ(env->Primitive_Unbox_Float(boxedZero, &unboxedZero), ANI_OK);
    ASSERT_EQ(unboxedZero, 0.0F);

    ani_object boxedValue = nullptr;
    ASSERT_EQ(env->Primitive_Box_Float(1.1F, &boxedValue), ANI_OK);
    AssertIsFloat(env, boxedValue);
    ani_float unboxedValue = 0.0F;
    ASSERT_EQ(env->Primitive_Unbox_Float(boxedValue, &unboxedValue), ANI_OK);
    ASSERT_EQ(unboxedValue, 1.1F);
}

static void TestUnboxFloatNative(ani_env *env)
{
    ani_class cls;
    ASSERT_EQ(env->FindClass("box_unbox_float_test.Message", &cls), ANI_OK);
    ani_method messageCtor;
    ASSERT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &messageCtor), ANI_OK);
    ani_object messageObj;
    ASSERT_EQ(env->Object_New(cls, messageCtor, &messageObj), ANI_OK);

    ani_field scoreField;
    ASSERT_EQ(env->Class_FindField(cls, "score_optional", &scoreField), ANI_OK);

    ani_ref scoreRef;
    ASSERT_EQ(env->Object_GetField_Ref(messageObj, scoreField, &scoreRef), ANI_OK);

    ani_float score = 0.0F;
    ASSERT_EQ(env->Primitive_Unbox_Float(static_cast<ani_object>(scoreRef), &score), ANI_OK);
    ASSERT_EQ(score, 1.2F);
}

static void TestPostArrayNative(ani_env *env, ani_array arr)
{
    ani_size len;
    ASSERT_EQ(env->Array_GetLength(arr, &len), ANI_OK);
    for (ani_size i = 0; i < len; ++i) {
        ani_ref arrElement;
        ASSERT_EQ(env->Array_Get(arr, i, &arrElement), ANI_OK);
        ani_float arrVal = 0.0F;
        ASSERT_EQ(env->Primitive_Unbox_Float(static_cast<ani_object>(arrElement), &arrVal), ANI_OK);
        ASSERT_EQ(arrVal, 1.1F);
    }
}

static ani_object TestWithArgsNative(ani_env *env, ani_float val)
{
    ani_object boxedValue;
    ani_status status = env->Primitive_Box_Float(val, &boxedValue);
    if (status != ANI_OK) {
        return nullptr;
    }
    AssertIsFloat(env, boxedValue);
    return boxedValue;
}

TEST_F(BoxUnboxFloatTest, boxed_float)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_float_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testBoxFloat", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testBoxFloatNative", ":", reinterpret_cast<void *>(TestBoxFloatNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxFloatTest, unboxed_float)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_float_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testUnboxFloat", ":", &etsFunc), ANI_OK);

    ani_native_function fn {"testUnboxFloatNative", ":", reinterpret_cast<void *>(TestUnboxFloatNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);
    ASSERT_EQ(env_->Function_Call_Void(etsFunc), ANI_OK);
}

TEST_F(BoxUnboxFloatTest, post_array)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_float_test", &module), ANI_OK);
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
        ASSERT_EQ(env_->Primitive_Box_Float(TEST_VALUE, &arrElement), ANI_OK);
        AssertIsFloat(env_, arrElement);
        ASSERT_EQ(env_->Array_Set(arr, i, arrElement), ANI_OK);
    }
    ASSERT_EQ(env_->Function_Call_Void(etsFunc, arr), ANI_OK);
}

TEST_F(BoxUnboxFloatTest, boxed_post_args)
{
    ani_module module = nullptr;
    ASSERT_EQ(env_->FindModule("box_unbox_float_test", &module), ANI_OK);
    ani_function etsFunc = nullptr;
    ASSERT_EQ(env_->Module_FindFunction(module, "testWithArgs", "f:C{std.core.Object}", &etsFunc), ANI_OK);

    ani_native_function fn {"testWithArgsNative", "f:C{std.core.Object}", reinterpret_cast<void *>(TestWithArgsNative)};
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, &fn, 1), ANI_OK);

    ani_ref res = nullptr;
    ASSERT_EQ(env_->Function_Call_Ref(etsFunc, &res, TEST_VALUE), ANI_OK);
    AssertIsFloat(env_, static_cast<ani_object>(res));
    ani_float unboxed = 0.0F;
    ASSERT_EQ(env_->Primitive_Unbox_Float(static_cast<ani_object>(res), &unboxed), ANI_OK);
    ASSERT_EQ(unboxed, TEST_VALUE);
}

TEST_F(BoxUnboxFloatTest, invalid_env)
{
    ani_object obj;
    ASSERT_EQ(env_->c_api->Primitive_Box_Float(nullptr, TEST_VALUE, &obj), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Primitive_Box_Float(TEST_VALUE, &obj), ANI_OK);
    AssertIsFloat(env_, obj);
    ani_float res;
    ASSERT_EQ(env_->c_api->Primitive_Unbox_Float(nullptr, obj, &res), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxFloatTest, invalid_args)
{
    ASSERT_EQ(env_->Primitive_Box_Float(TEST_VALUE, nullptr), ANI_INVALID_ARGS);

    ani_float val;
    ASSERT_EQ(env_->Primitive_Unbox_Float(nullptr, &val), ANI_INVALID_ARGS);

    ani_object obj;
    ASSERT_EQ(env_->Primitive_Box_Float(TEST_VALUE, &obj), ANI_OK);
    AssertIsFloat(env_, obj);
    ASSERT_EQ(env_->Primitive_Unbox_Float(obj, nullptr), ANI_INVALID_ARGS);
}

TEST_F(BoxUnboxFloatTest, invalid_types)
{
    ani_string stringObj;
    std::string str = "test";
    ASSERT_EQ(env_->String_NewUTF8(str.c_str(), str.size(), &stringObj), ANI_OK);

    ani_float result;
    ASSERT_EQ(env_->Primitive_Unbox_Float(stringObj, &result), ANI_INVALID_TYPE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
