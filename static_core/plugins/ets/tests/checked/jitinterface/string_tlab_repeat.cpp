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

#include "runtime/compiler.h"
#include "runtime/include/thread.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

extern "C" {
namespace ark::ets::ani {
// NOLINTNEXTLINE(readability-identifier-naming)
ANI_EXPORT ani_int CompileMethodImpl([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_string name)
{
    if (!Runtime::GetCurrent()->IsJitEnabled()) {
        return 1;
    }
    ScopedManagedCodeFix s(env);
    auto str = s.ToInternalType(name);
    return ark::CompileMethodImpl(str->GetCoreType(), panda_file::SourceLang::ETS);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *moduleName = "Lstring_tlab_repeat;";
    ani_module md;
    if (ANI_OK != env->FindModule(moduleName, &md)) {
        auto msg = std::string("Cannot find \"") + moduleName + std::string("\" module!");
        ark::ets::stdlib::ThrowNewError(env, "Lstd/core/RuntimeException;", msg.data(), "Lstd/core/String;:V");
        return ANI_ERROR;
    }

    const auto functions = std::array {
        ani_native_function {"compileMethod", "Lstd/core/String;:I", reinterpret_cast<void *>(CompileMethodImpl)}};

    if (ANI_OK != env->Module_BindNativeFunctions(md, functions.data(), functions.size())) {
        std::cerr << "Cannot bind native methods to '" << moduleName << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

}  // namespace ark::ets::ani

}  // extern "C"
#endif
