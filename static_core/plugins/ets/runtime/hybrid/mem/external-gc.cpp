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

#include "libpandabase/utils/logger.h"
#include "runtime/mark_word.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#include "objects/ref_field.h"
#include "heap/heap_visitor.h"
#ifdef PANDA_JS_ETS_HYBRID_MODE
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#endif  // PANDA_JS_ETS_HYBRID_MODE

namespace ark::mem::ets {

void VisitVmRoots(const GCRootVisitor &visitor, PandaVM *vm)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.VisitNonHeapRoots(visitor, VisitGCRootFlags::ACCESS_ROOT_ALL);
    vm->VisitStringTable([&visitor](ObjectHeader *str) { visitor(GCRoot(RootType::STRING_TABLE, str)); },
                         VisitGCRootFlags::ACCESS_ROOT_ALL);
}

void UpdateVmRoots(const GCRootUpdater &updater, PandaVM *vm)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.UpdateRefsToMovedObjects(updater);
}

}  // namespace ark::mem::ets

namespace common {

static ark::PandaVM *GetPandaVM()
{
    auto *runtime = ark::Runtime::GetCurrent();
    if (runtime == nullptr) {
        return nullptr;
    }
    return runtime->GetPandaVM();
}

void VisitStaticRoots(const RefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }

    ark::GCRootVisitor rootVisitor = [&visitor](const ark::mem::GCRoot &gcRoot) {
        common::RefField<> refField {reinterpret_cast<common::BaseObject *>(gcRoot.GetObjectHeader())};
        visitor(refField);
    };
    ark::mem::ets::VisitVmRoots(rootVisitor, vm);
}

void UpdateStaticRoots(const RefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }

    ark::GCRootUpdater rootUpdater = [&visitor](ark::ObjectHeader **object) {
        ark::ObjectHeader *prev = *object;
        visitor(reinterpret_cast<RefField<> &>(*reinterpret_cast<BaseObject **>(object)));
        return prev != *object;
    };
    ark::mem::ets::UpdateVmRoots(rootUpdater, vm);
}

void SweepStaticRoots(const WeakRefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    ark::GCObjectVisitor visitorWrap = [&visitor](ark::ObjectHeader *object) -> ark::ObjectStatus {
        // Provide the reference to a local variable.
        // The object must be forwarded during UpdateStaticRoots.
        // Here we need to know is the object alive or not.
        bool alive = visitor(reinterpret_cast<RefField<> &>(*reinterpret_cast<BaseObject **>(&object)));
        return alive ? ark::ObjectStatus::ALIVE_OBJECT : ark::ObjectStatus::DEAD_OBJECT;
    };
    vm->SweepVmRefs(visitorWrap);
#ifdef PANDA_JS_ETS_HYBRID_MODE
    static_cast<ark::mem::ets::EtsReferenceProcessor *>(vm->GetReferenceProcessor())->ClearDeadReference(visitorWrap);
#endif
}

#ifdef PANDA_JS_ETS_HYBRID_MODE
void UnmarkAllXRefsImpl()
{
    ark::ets::interop::js::XGC::GetInstance()->UnmarkAllXRefs();
}

void SweepUnmarkedXRefsImpl()
{
    ark::ets::interop::js::XGC::GetInstance()->SweepUnmarkedXRefs();
}

void AddXRefToStaticRoots()
{
    ark::ets::interop::js::XGC::GetInstance()->AddXRefToStaticRoots();
}

void RemoveXRefFromStaticRoots()
{
    ark::ets::interop::js::XGC::GetInstance()->RemoveXRefFromStaticRoots();
}

bool TryTransferCurrentCoroutineToNative(common::ThreadHolder *current)
{
    ark::Thread *thread = ark::Thread::GetCurrent();
    if (thread == nullptr) {
        return false;
    }
    ark::Coroutine *co = ark::Coroutine::GetCurrent();
    if (co == nullptr) {
        return false;
    }
    common::ThreadHolder *holder = co->GetThreadHolder();
    if (holder == current) {
        return false;
    }
    return holder->TransferToNativeIfInRunning();
}

bool TryTransferCurrentCoroutineToRunning(common::ThreadHolder *current)
{
    ark::Thread *thread = ark::Thread::GetCurrent();
    if (thread == nullptr) {
        return false;
    }
    ark::Coroutine *co = ark::Coroutine::GetCurrent();
    if (co == nullptr) {
        return false;
    }
    common::ThreadHolder *holder = co->GetThreadHolder();
    if (holder == current) {
        return false;
    }
    return holder->TransferToRunningIfInNative();
}
#endif  // PANDA_JS_ETS_HYBRID_MODE

void RegisterStaticRootsProcessFunc()
{
    RegisterVisitStaticRootsHook(VisitStaticRoots);
    RegisterUpdateStaticRootsHook(UpdateStaticRoots);
    RegisterSweepStaticRootsHook(SweepStaticRoots);
#ifdef PANDA_JS_ETS_HYBRID_MODE
    RegisterUnmarkAllXRefsHook(UnmarkAllXRefsImpl);
    RegisterSweepUnmarkedXRefsHook(SweepUnmarkedXRefsImpl);
    RegisterAddXRefToStaticRootsHook(AddXRefToStaticRoots);
    RegisterRemoveXRefFromStaticRootsHook(RemoveXRefFromStaticRoots);
    RegisterInterOpCoroutineToNativeHook(TryTransferCurrentCoroutineToNative);
    RegisterInterOpCoroutineToRunningHook(TryTransferCurrentCoroutineToRunning);
#endif
}

}  // namespace common
