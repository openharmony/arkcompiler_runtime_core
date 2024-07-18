/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
ETS_EXPORT ets_string ETS_CALL ETS_EtsnapiVersionHookTest_hello(EtsEnv *env, [[maybe_unused]] ets_class)
{
    return env->NewStringUTF("Hello");
}

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static EtsNativeMethod gMethods[] = {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    {"hello", ":Lstd/core/String;", (void *)ETS_EtsnapiVersionHookTest_hello},
};

static int registerNativeMethods(EtsEnv *env, const char *classname, EtsNativeMethod *methods, int countMethods)
{
    ets_class clazz = env->FindClass(classname);
    if (clazz == nullptr) {
        return ETS_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, countMethods) < 0) {
        return ETS_FALSE;
    }
    return ETS_TRUE;
}

static int registerNatives(EtsEnv *env)
{
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    if (!registerNativeMethods(env, "EtsnapiVersionHookTest", gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        return ETS_FALSE;
    }
    return ETS_TRUE;
}

ETS_EXPORT ets_int ETS_CALL EtsNapiOnLoad(EtsEnv *env)
{
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    if (!registerNatives(env)) {
        return -1;
    }
    return ETS_NAPI_VERSION_1_0;
}

// NOLINTEND(readability-named-parameter, cppcoreguidelines-pro-type-vararg, readability-identifier-naming)

}  // extern "C"
