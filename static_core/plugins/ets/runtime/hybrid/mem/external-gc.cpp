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
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/base_runtime.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"

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

static void VisitAllRoots(const common_vm::RefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.VisitNonHeapRoots([&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common_vm::RefField<> &>(*root.GetObjectPointer()));
    });
}

static void VisitGlobalRoots(const common_vm::RefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);

    auto callback = [&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common_vm::RefField<> &>(*root.GetObjectPointer()));
    };
    rootManager.VisitAotStringRoots(callback, VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(callback);
}

static void UpdateAndSweep(const common_vm::WeakRefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);

    rootManager.UpdateAndSweep([&visitorFunc](ObjectHeader **ref) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        return visitorFunc(reinterpret_cast<common_vm::RefField<> &>(*ref)) ? ObjectStatus::ALIVE_OBJECT
                                                                            : ObjectStatus::DEAD_OBJECT;
    });
}

static void VisitConcurrentRoots(const common_vm::RefFieldVisitor &visitorFunc)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    RootManager<EtsLanguageConfig> rootManager(vm);

    auto callback = [&visitorFunc](GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitorFunc(reinterpret_cast<common_vm::RefField<> &>(*root.GetObjectPointer()));
    };
    rootManager.VisitClassRoots(callback, VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitClassLinkerContextRoots(callback);
    vm->VisitStringTable(callback, VisitGCRootFlags::ACCESS_ROOT_ALL);
}

static void ProcessFinalizationRegistryCleanup()
{
    auto *vm = static_cast<ark::ets::PandaEtsVM *>(GetPandaVM());
    auto *referenceProcessor = static_cast<mem::ets::EtsReferenceProcessor *>(vm->GetReferenceProcessor());
    auto *executionCtx = ark::ets::EtsExecutionContext::GetCurrent();
    referenceProcessor->ProcessClearedReferences();
    vm->GetFinalizationRegistryManager()->LaunchCleanupJobIfNeeded(executionCtx);
}

static void ProcessReferencesAfterCopy()
{
    auto *vm = static_cast<ark::ets::PandaEtsVM *>(GetPandaVM());
    auto *referenceProcessor = static_cast<mem::ets::EtsReferenceProcessor *>(vm->GetReferenceProcessor());
    referenceProcessor->ProcessReferencesAfterCopy();
}

// Temporary solution. Without changes under static_core directory CI 'Panda' tests group won't run
class DummyGCListener final : public common_vm::GCListener {
public:
    void OnGCStart(common_vm::GCReason reason, common_vm::GCType type) override {}
    void OnGCFinish(common_vm::GCReason reason, common_vm::GCType type) override {}
    void OnGCPhaseStart(common_vm::GCPhase phase) override {}
    void OnGCPhaseEnd(common_vm::GCPhase phase) override {}
    ~DummyGCListener()
    {
        common_vm::BaseRuntime::GetInstance()->RemoveGCListener(this);
    }
};

class StaticVMInterface : public common_vm::VMInterface {
public:
    StaticVMInterface() : dummyGCListener_(new DummyGCListener())
    {
        common_vm::BaseRuntime::GetInstance()->RegisterVM(this);
        common_vm::BaseRuntime::GetInstance()->AddGCListener(dummyGCListener_.get());
    }

    ~StaticVMInterface() override
    {
        common_vm::BaseRuntime::GetInstance()->UnregisterVM(this);
    }

    void VisitAllRoots(const common_vm::RefFieldVisitor &visitor) override
    {
        ark::mem::ets::VisitAllRoots(visitor);
    }

    void VisitGlobalRoots(const common_vm::RefFieldVisitor &visitor) override
    {
        ark::mem::ets::VisitGlobalRoots(visitor);
    }

    void VisitConcurrentRoots(const common_vm::RefFieldVisitor &visitor) override
    {
        ark::mem::ets::VisitConcurrentRoots(visitor);
    }

    void UpdateAndSweep(const common_vm::WeakRefFieldVisitor &visitor) override
    {
        ark::mem::ets::UpdateAndSweep(visitor);
    }

    void ProcessFinalizationRegistryCleanup() override
    {
        ark::mem::ets::ProcessFinalizationRegistryCleanup();
    }

    void ProcessReferencesAfterCopy() override
    {
        ark::mem::ets::ProcessReferencesAfterCopy();
    }

    void VisitPreforwardRoots([[maybe_unused]] const common_vm::RefFieldVisitor &visitor) override {}

private:
    std::unique_ptr<DummyGCListener> dummyGCListener_;
};

common_vm::VMInterface *RegisterCmcGcCallbacks()
{
    ark::mem::StaticObjectOperator::Initialize();
    return new StaticVMInterface();
}

}  // namespace ark::mem::ets
