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

#include "plugins/ets/runtime/napi/ets_napi.h"
#include <ostream>

extern "C" {
// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_string ETS_CALL basic_call_s(EtsEnv *env, [[maybe_unused]] ets_class self, [[maybe_unused]] ets_int a,
                                            [[maybe_unused]] ets_string b, [[maybe_unused]] ets_long c,
                                            [[maybe_unused]] ets_object d)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_long ETS_CALL basic_call_sc([[maybe_unused]] ets_int a, [[maybe_unused]] ets_int b,
                                           [[maybe_unused]] ets_long c, [[maybe_unused]] ets_long d)
{
    return d;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_string ETS_CALL basic_call_sf(EtsEnv *env, [[maybe_unused]] ets_class, [[maybe_unused]] ets_int a,
                                             [[maybe_unused]] ets_string b, [[maybe_unused]] ets_long c,
                                             [[maybe_unused]] ets_object d)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_string ETS_CALL basic_call_v(EtsEnv *env, [[maybe_unused]] ets_object, [[maybe_unused]] ets_int a,
                                            [[maybe_unused]] ets_string b, [[maybe_unused]] ets_long c,
                                            [[maybe_unused]] ets_object d)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_string ETS_CALL basic_call_vf(EtsEnv *env, [[maybe_unused]] ets_object, [[maybe_unused]] ets_int a,
                                             [[maybe_unused]] ets_string b, [[maybe_unused]] ets_long c,
                                             [[maybe_unused]] ets_object d)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_string ETS_CALL basic_call_v_final(EtsEnv *env, [[maybe_unused]] ets_object, [[maybe_unused]] ets_int a,
                                                  [[maybe_unused]] ets_string b, [[maybe_unused]] ets_long c,
                                                  [[maybe_unused]] ets_object d)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_string ETS_CALL basic_call_vf_final(EtsEnv *env, [[maybe_unused]] ets_object, [[maybe_unused]] ets_int a,
                                                   [[maybe_unused]] ets_string b, [[maybe_unused]] ets_long c,
                                                   [[maybe_unused]] ets_object d)
{
    return b;
}

// NOLINTBEGIN(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)
ETS_EXPORT ets_long ETS_CALL basic_call_baseline(EtsEnv *env, [[maybe_unused]] ets_class)
{
    return 1;
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static EtsNativeMethod gMethods[] = {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_s", "ILstd/core/String;JLstd/core/Object;:Lstd/core/String;", reinterpret_cast<void *>(basic_call_s)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_sc", "IIJJ:J", reinterpret_cast<void *>(basic_call_sc)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_sf", "ILstd/core/String;JLstd/core/Object;:Lstd/core/String;",
     reinterpret_cast<void *>(basic_call_sf)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_v", "ILstd/core/String;JLstd/core/Object;:Lstd/core/String;", reinterpret_cast<void *>(basic_call_v)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_vf", "ILstd/core/String;JLstd/core/Object;:Lstd/core/String;",
     reinterpret_cast<void *>(basic_call_vf)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_v_final", "ILstd/core/String;JLstd/core/Object;:Lstd/core/String;",
     reinterpret_cast<void *>(basic_call_v_final)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_vf_final", "ILstd/core/String;JLstd/core/Object;:Lstd/core/String;",
     reinterpret_cast<void *>(basic_call_vf_final)},
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"basic_call_baseline", ":J", reinterpret_cast<void *>(basic_call_baseline)},
};

ETS_EXPORT ets_int ETS_CALL EtsNapiOnLoad(EtsEnv *env)
{
    ets_class clazz = env->FindClass("BasicCall");
    if (clazz == nullptr) {
        return -1;
    }
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    if (env->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        return -1;
    }
    return ETS_NAPI_VERSION_1_0;
}

// NOLINTEND(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)

}  // extern "C"
