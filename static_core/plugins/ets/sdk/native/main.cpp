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

#include "api/Util.h"
#include <array>
#include <cstddef>
#include <string>
#include "ets_napi.h"
#include "libpandabase/macros.h"

extern "C" {
ETS_EXPORT ets_int ETS_CALL EtsNapiOnLoad(EtsEnv *env)
{
    ASSERT(env != nullptr);
    const auto methods = std::array {
        EtsNativeMethod {"generateRandomUUID", nullptr,
                         reinterpret_cast<void *>(ark::ets::sdk::util::ETSApiUtilHelperGenerateRandomUUID)}};

    auto hlpCls = std::string("api/util/UtilHelper");
    ets_class cls = env->FindClass(hlpCls.data());
    if (UNLIKELY(!cls)) {
        auto msg = std::string("Cannot find \"") + hlpCls + std::string("\" class!");
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), msg.data());
        return ETS_ERR;
    }
    auto res = env->RegisterNatives(cls, methods.data(), methods.size());
    return res == ETS_OK ? ETS_NAPI_VERSION_1_0 : ETS_ERR;
}
}
