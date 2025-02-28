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

#include <ani.h>
#include <array>
#include <string>

#include "api/Util.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    constexpr static const char *CLASS_NAME = "Lapi/util/UtilHelper;";
    ani_class cls;
    if (ANI_OK != env->FindClass(CLASS_NAME, &cls)) {
        auto msg = std::string("Cannot find \"") + CLASS_NAME + std::string("\" class!");
        ark::ets::stdlib::ThrowNewError(env, "Lstd/core/RuntimeException;", msg.data(), "Lstd/core/String;:V");
        return ANI_ERROR;
    }

    const auto methods = std::array {
        ani_native_function {"generateRandomUUID", "Z:Lstd/core/String;",
                             reinterpret_cast<void *>(ark::ets::sdk::util::ETSApiUtilHelperGenerateRandomUUID)}};

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << CLASS_NAME << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
