/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "libarkbase/utils/logger.h"
#include "runtime/mem/gc/gc_root.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/heap/heap_visitor.h"

namespace common {
void SetVisitAllStaticRootsCallback(void (*callback)(const RefFieldVisitor &));
void SetVisitStaticMutatorRootsCallback(void (*callback)(const RefFieldVisitor &visitorFunc, void *mutator));
void SetVisitStaticGlobalRootsCallback(void (*callback)(const RefFieldVisitor &));
void SetVisitStaticConcurrentRootsCallback(void (*callback)(const RefFieldVisitor &));
void SetUpdateAndSweepStaticRefsCallback(void (*callback)(const WeakRefFieldVisitor &visitor));
}  // namespace common

namespace ark::mem::ets {

static ark::PandaVM *GetPandaVM()
{
    // Get PandaVM from Runtime because this function is called from GC thread
    // which is not a managed thread.
    auto *runtime = ark::Runtime::GetCurrent();
    if (runtime == nullptr) {
        return nullptr;
    }
    return runtime->GetPandaVM();
}

static void VisitAllRoots(const common::RefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.VisitNonHeapRoots([&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common::RefField<> &>(*root.GetObjectPointer()));
    });
}

static void VisitMutatorRoots(const common::RefFieldVisitor &visitorFunc, void *mutator)
{
    ASSERT(mutator != nullptr);
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);
    auto *managedThread = reinterpret_cast<ManagedThread *>(mutator);
    rootManager.VisitRootsForThread(managedThread, [&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common::RefField<> &>(*root.GetObjectPointer()));
    });
}

static void VisitGlobalRoots(const common::RefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);

    auto callback = [&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common::RefField<> &>(*root.GetObjectPointer()));
    };
    rootManager.VisitAotStringRoots(callback, VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(callback);
}

static void UpdateAndSweep(const common::WeakRefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);

    rootManager.UpdateAndSweep([&visitorFunc](ObjectHeader **ref) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        return visitorFunc(reinterpret_cast<common::RefField<> &>(*ref)) ? ObjectStatus::ALIVE_OBJECT
                                                                         : ObjectStatus::DEAD_OBJECT;
    });
}

static void VisitConcurrentRoots(const common::RefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);

    auto callback = [&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common::RefField<> &>(*root.GetObjectPointer()));
    };
    rootManager.VisitClassRoots(callback, VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitClassLinkerContextRoots(callback);
    vm->VisitStringTable(callback, VisitGCRootFlags::ACCESS_ROOT_ALL);
}

void RegisterCmcGcCallbacks()
{
    ark::mem::StaticObjectOperator::Initialize();
    common::SetVisitAllStaticRootsCallback(VisitAllRoots);
    common::SetVisitStaticMutatorRootsCallback(VisitMutatorRoots);
    common::SetVisitStaticGlobalRootsCallback(VisitGlobalRoots);
    common::SetUpdateAndSweepStaticRefsCallback(UpdateAndSweep);
    common::SetVisitStaticConcurrentRootsCallback(VisitConcurrentRoots);
}

}  // namespace ark::mem::ets
