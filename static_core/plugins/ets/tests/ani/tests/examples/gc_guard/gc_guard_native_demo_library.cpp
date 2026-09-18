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

#include <array>
#include <iostream>

#include "ani.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/libani_helpers/gc_guard.h"
#include "runtime/include/gc_task.h"

namespace {

constexpr const char *CLASS_NAME = "gc_guard_native_demo.GcGuardNativeDemo";

void NativeFunc([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class cls)
{
    // The guard lasts until the end of this native function.  Any GC entered
    // in the scope must terminate the process with LOG(FATAL).
    arkts::GCGuard gcGuard;

    // This explicitly drives the real GC path:
    // GCTask::Run -> GC::WaitForGC -> GC::RunPhases -> DCHECK_ALLOW_GARBAGE_COLLECTION.
    ark::GCTask task(ark::GCTaskCause::OOM_CAUSE);
    task.Run(*ark::ets::PandaEtsVM::GetCurrent()->GetGC());
}

}  // namespace

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    ani_class cls = nullptr;
    if (env->FindClass(CLASS_NAME, &cls) != ANI_OK) {
        std::cerr << "Class not found: " << CLASS_NAME << std::endl;
        return ANI_ERROR;
    }

    const std::array methods = {
        ani_native_function {"native_func", ":", reinterpret_cast<void *>(NativeFunc)},
    };
    if (env->Class_BindStaticNativeMethods(cls, methods.data(), methods.size()) != ANI_OK) {
        std::cerr << "Cannot bind native_func" << std::endl;
        return ANI_ERROR;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}
