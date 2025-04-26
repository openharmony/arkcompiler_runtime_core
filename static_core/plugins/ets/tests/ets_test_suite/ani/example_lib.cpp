/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani/ani.h"
#include <gtest/gtest.h>
#include <array>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
static ani_long NativeFuncExample([[maybe_unused]] ani_env *env, ani_long param1, ani_long param2)
{
    return param1 * param2;
}

static void TestCheck1(ani_env *env)
{
    ani_double p1 = 5.0;
    ani_double p2 = 6.0;

    ani_module module;
    ASSERT_EQ(env->FindModule("Lexample_lib;", &module), ANI_OK);

    ASSERT_NE(module, nullptr);
    ani_function fn;
    ASSERT_EQ(env->Module_FindFunction(module, "exampleFunction", "DD:D", &fn), ANI_OK);

    ani_double res;
    env->Function_Call_Double(fn, &res, p1, p2);
    ASSERT_EQ(res, p1 + p2);
}

static void TestCheck2(ani_env *env)
{
    ani_long p1 = 12;
    ani_long p2 = -123;

    ani_module module;
    ASSERT_EQ(env->FindModule("Lexample_lib;", &module), ANI_OK);

    ASSERT_NE(module, nullptr);
    ani_function fn;
    ASSERT_EQ(env->Module_FindFunction(module, "nativeExampleFunction", "JJ:J", &fn), ANI_OK);

    ani_long res;
    env->Function_Call_Long(fn, &res, p1, p2);
    ASSERT_EQ(res, p1 * p2);
}

static void TestCheck3()
{
    ani_vm *vm {nullptr};
    ani_env *env {nullptr};

    // Get ANI API
    ani_size nrVMs;
    ASSERT_EQ(ANI_GetCreatedVMs(&vm, 1, &nrVMs), ANI_OK);
    ASSERT_EQ(vm->GetEnv(ANI_VERSION_1, &env), ANI_OK);
    uint32_t aniVersin;
    ASSERT_EQ(env->GetVersion(&aniVersin), ANI_OK);
    ASSERT_TRUE(aniVersin == ANI_VERSION_1) << "Incorrect ani version";

    ani_class cls;
    // Locate the class "Lexample_lib/Aaa;" in the environment
    ASSERT_EQ(env->FindClass("Lexample_lib/Aaa;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    // Emulate the allocation of instance of the class
    ani_static_method newMethod;
    ASSERT_EQ(env->Class_FindStaticMethod(cls, "new_Aaa", ":Lexample_lib/Aaa;", &newMethod), ANI_OK);
    ani_ref ref;
    ASSERT_EQ(env->Class_CallStaticMethod_Ref(cls, newMethod, &ref), ANI_OK);

    ani_method method;
    // Retrieve a method named "boolean_method" with signature "II:Z"
    ASSERT_EQ(env->Class_FindMethod(cls, "boolean_method", "II:Z", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    auto object = static_cast<ani_object>(ref);
    ani_value args[2];
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_boolean res;
    // Call the method and verify the return value
    ASSERT_EQ(env->Object_CallMethod_Boolean_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, false);
}

static ani_long NativeTestFunc([[maybe_unused]] ani_env *env, ani_long param)
{
    TestCheck1(env);
    TestCheck2(env);
    TestCheck3();
    return param;
}

static std::array g_gMethods = {
    ani_native_function {"nativeExampleFunction", "JJ:J", reinterpret_cast<void *>(NativeFuncExample)},
    ani_native_function {"nativeTestFunction", "J:J", reinterpret_cast<void *>(NativeTestFunc)},
};

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        return ANI_ERROR;
    }
    std::array methods = {
        ani_native_function {"nativeExampleFunction", "JJ:J", reinterpret_cast<void *>(NativeFuncExample)},
        ani_native_function {"nativeTestFunction", "J:J", reinterpret_cast<void *>(NativeTestFunc)},
    };

    ani_module module;
    if (env->FindModule("Lexample_lib;", &module) != ANI_OK) {
        std::cerr << "Cannot find module!" << std::endl;
        return ANI_ERROR;
    }
    if (env->Module_BindNativeFunctions(module, methods.data(), methods.size()) != ANI_OK) {
        std::cerr << "Cannot bind native functions!" << std::endl;
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
