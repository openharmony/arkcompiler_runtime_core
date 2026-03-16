/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_BASE_RUNTIME_HOOKS_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_BASE_RUNTIME_HOOKS_H

#include <cstdint>

#include "common_interfaces/base/runtime_param.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/thread/mutator.h"

// Visitor that iterate all `RefField`s in a TaggedObject and add them to
// `LocalMarkStack` Should be moved to BaseRT and panda namespace later
namespace common_vm {
PUBLIC_API void InvokeSharedNativePointerCallbacks();

PUBLIC_API void AddXRefToDynamicRoots();
PUBLIC_API void RemoveXRefFromDynamicRoots();

PUBLIC_API void SynchronizeGCPhaseToJSThread(void *jsThread, GCPhase gcPhase);

// CMC-GC dependent interface
PUBLIC_API void SetBaseAddress(uintptr_t base);
PUBLIC_API void JSGCCallback(void *ecmaVM);
PUBLIC_API bool IsPostForked();

// Jit interfaces
PUBLIC_API void SweepThreadLocalJitFort();
PUBLIC_API bool IsMachineCodeObject(uintptr_t obj);
PUBLIC_API void JitFortUnProt(size_t size, void *base);
PUBLIC_API void MarkThreadLocalJitFortInstalled(void *mutator, void *machineCode);

// Used for init/fini BaseRuntime from static
PUBLIC_API void CheckAndInitBaseRuntime(const RuntimeParam &param);
PUBLIC_API void CheckAndFiniBaseRuntime();
}  // namespace common_vm
#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_BASE_RUNTIME_HOOKS_H
