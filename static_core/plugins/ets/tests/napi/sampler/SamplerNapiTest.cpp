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

#include "SamplerNapiTest.h"
#include "libpandabase/utils/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTBEGIN(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

ETS_EXPORT ets_int ETS_CALL ETS_ETSGLOBAL_NativeSlowFunction([[maybe_unused]] EtsEnv *, [[maybe_unused]] ets_class,
                                                             ets_int iterations)
{
    ets_int res = 0;
    for (ets_int i = 0; i < iterations; i++) {
        ++res;
    }
    for (size_t k = 0; k < 4U; ++k) {
        for (ets_int i = 0; i < iterations; i++) {
            res = res ^ i;
        }
    }
    // res = 20'000'000-200'000'000
    return res;
}

ETS_EXPORT ets_int ETS_CALL ETS_ETSGLOBAL_NativeFastFunction([[maybe_unused]] EtsEnv *, [[maybe_unused]] ets_class)
{
    ets_int res = 1;
    return res;
}

ETS_EXPORT ets_int ETS_CALL ETS_ETSGLOBAL_NativeNAPISlowFunction(EtsEnv *env, ets_class cls, ets_int iterations)
{
    ets_method method = env->GetStaticp_method(cls, "SlowETSFunction", "I:I");
    ets_int res = env->CallStaticIntMethod(cls, method, iterations);
    return res;
}

ETS_EXPORT ets_int ETS_CALL ETS_ETSGLOBAL_NativeNAPIFastFunction(EtsEnv *env, ets_class cls, ets_int iterations)
{
    ets_method method = env->GetStaticp_method(cls, "FastETSFunction", ":I");
    ets_int res = 0;
    for (int i = 0; i < iterations; ++i) {
        res += env->CallStaticIntMethod(cls, method);
    }
    return res;
}

// NOLINTEND(readability-magic-numbers, readability-named-parameter, cppcoreguidelines-pro-type-vararg,
// hicpp-signed-bitwice)

#ifdef __cplusplus
}
#endif
