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

#include <ani.h>

#include <array>

static ani_ref Leak(ani_env *env)
{
    ani_ref res {};
    env->GetUndefined(&res);
    return res;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        return ANI_ERROR;
    }

    std::array functions = {
        ani_native_function {"leakString", ":C{std.core.String}", reinterpret_cast<void *>(Leak)},
        ani_native_function {"leakClass", ":C{std.core.Class}", reinterpret_cast<void *>(Leak)},
        ani_native_function {"leakLinker", ":C{std.core.RuntimeLinker}", reinterpret_cast<void *>(Leak)},
        ani_native_function {"leakClassArray", ":A{C{std.core.Class}}", reinterpret_cast<void *>(Leak)},
        ani_native_function {"leakByteArray", ":A{b}", reinterpret_cast<void *>(Leak)},
    };

    ani_namespace ns {};
    if (env->FindNamespace("NullSafetyTest.unsafe", &ns) != ANI_OK) {
        return ANI_ERROR;
    }
    if (env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()) != ANI_OK) {
        return ANI_ERROR;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}
