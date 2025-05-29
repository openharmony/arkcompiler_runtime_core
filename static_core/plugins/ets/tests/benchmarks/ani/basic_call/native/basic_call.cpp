/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ani/ani.h"
#include <iostream>
#include <array>

extern "C" {
// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_s(ani_env *env, [[maybe_unused]] ani_class self, [[maybe_unused]] ani_long a,
                        [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_long basic_call_sc([[maybe_unused]] ani_long a, [[maybe_unused]] ani_int b, [[maybe_unused]] ani_long c)
{
    return c;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_sf(ani_env *env, [[maybe_unused]] ani_class, [[maybe_unused]] ani_long a,
                         [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_v(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                        [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_vf(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                         [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_v_final(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                              [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_string basic_call_vf_final(ani_env *env, [[maybe_unused]] ani_object, [[maybe_unused]] ani_long a,
                               [[maybe_unused]] ani_string b, [[maybe_unused]] ani_object c)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ani_long basic_call_baseline(ani_env *env, [[maybe_unused]] ani_class)
{
    return 1;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cout << "Wrong version of ANI!\n";
        return ANI_ERROR;
    }
    ani_class cls;
    if (ANI_OK != env->FindClass("L$bench_name/BasicCall;", &cls)) {
        std::cout << "Class BasicCall not found!\n";
        return ANI_ERROR;
    }
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    std::array<ani_native_function, 8U> methods = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_s", "JLstd/core/String;Lstd/core/Object;:Lstd/core/String;",
                             reinterpret_cast<void *>(basic_call_s)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_sc", "JIJ:J", reinterpret_cast<void *>(basic_call_sc)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_sf", "JLstd/core/String;Lstd/core/Object;:Lstd/core/String;",
                             reinterpret_cast<void *>(basic_call_sf)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_v", "JLstd/core/String;Lstd/core/Object;:Lstd/core/String;",
                             reinterpret_cast<void *>(basic_call_v)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_vf", "JLstd/core/String;Lstd/core/Object;:Lstd/core/String;",
                             reinterpret_cast<void *>(basic_call_vf)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_v_final", "JLstd/core/String;Lstd/core/Object;:Lstd/core/String;",
                             reinterpret_cast<void *>(basic_call_v_final)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_vf_final", "JLstd/core/String;Lstd/core/Object;:Lstd/core/String;",
                             reinterpret_cast<void *>(basic_call_vf_final)},
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        ani_native_function {"basic_call_baseline", ":J", reinterpret_cast<void *>(basic_call_baseline)},
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cout << "Binding native methods failed!\n";
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}

// NOLINTEND(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)

}  // extern "C"
