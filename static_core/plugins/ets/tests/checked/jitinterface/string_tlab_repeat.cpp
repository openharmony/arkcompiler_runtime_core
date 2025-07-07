/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PRODUCT_BUILD

#include <array>

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "plugins/ets/tests/checked/jitinterface/compile_method.h"

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *moduleName = "string_tlab_repeat";
    ani_module md;
    if (ANI_OK != env->FindModule(moduleName, &md)) {
        auto msg = std::string("Cannot find \"") + moduleName + std::string("\" module!");
        ark::ets::stdlib::ThrowNewError(env, "std.core.RuntimeException", msg.data(), "C{std.core.String}:");
        return ANI_ERROR;
    }

    const auto functions = std::array {ani_native_function {"compileMethod", "C{std.core.String}:i",
                                                            reinterpret_cast<void *>(ark::ets::ani::CompileMethod)}};

    if (ANI_OK != env->Module_BindNativeFunctions(md, functions.data(), functions.size())) {
        std::cerr << "Cannot bind native methods to '" << moduleName << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

#endif  // PANDA_PRODUCT_BUILD
