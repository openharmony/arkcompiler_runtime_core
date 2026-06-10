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
 */

#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"
#include "plugins/ets/runtime/interop_js/tooling/internal_api.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "libarkbase/utils/time.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/mem/gc/gc.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "runtime/include/field.h"
#include "runtime/include/language_config.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mutator.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/thread_manager.h"
#include "runtime/tooling/hprof/heap_dump.h"

namespace ark::ets::interop::js {

using HeapDump = ark::tooling::hprof::HeapDump;

thread_local STSVMInterfaceImpl::XGCSyncState STSVMInterfaceImpl::xgcSyncState_ =
    STSVMInterfaceImpl::XGCSyncState::NONE;
thread_local bool STSVMInterfaceImpl::dumpManagedScopeActive_ = false;

void STSVMInterfaceImpl::MarkFromObject(void *obj)
{
    XGC::GetInstance()->MarkFromObject(obj);
}

STSVMInterfaceImpl::STSVMInterfaceImpl() : xgcBarrier_(STSVMInterface::DEFAULT_XGC_EXECUTORS_COUNT) {}

STSVMInterfaceImpl::STSVMInterfaceImpl(PandaEtsVM *vm)
    : xgcBarrier_(STSVMInterface::DEFAULT_XGC_EXECUTORS_COUNT), vm_(vm)
{
    ASSERT(vm != nullptr);
}

void STSVMInterfaceImpl::OnVMAttach()
{
    xgcBarrier_.Increment();
}

void STSVMInterfaceImpl::OnVMDetach()
{
    xgcBarrier_.Decrement();
}

bool STSVMInterfaceImpl::StartXGCBarrier(const NoWorkPred &func)
{
    ASSERT(xgcSyncState_ == XGCSyncState::NONE);
    auto res = xgcBarrier_.InitialWait(func);
    if (res) {
        xgcSyncState_ = XGCSyncState::CONCURRENT_PHASE;
    }
    return res;
}

bool STSVMInterfaceImpl::WaitForConcurrentMark(const NoWorkPred &func)
{
    ASSERT(xgcSyncState_ == XGCSyncState::CONCURRENT_PHASE);
    auto res = xgcBarrier_.Wait(func);
    if (res) {
        xgcSyncState_ = XGCSyncState::CONCURRENT_FINISHED;
    }
    return res;
}

void STSVMInterfaceImpl::RemarkStartBarrier()
{
    ASSERT(xgcSyncState_ == XGCSyncState::CONCURRENT_FINISHED);
    xgcBarrier_.Wait();
    xgcSyncState_ = XGCSyncState::REMARK_PHASE;
}

bool STSVMInterfaceImpl::WaitForRemark(const NoWorkPred &func)
{
    ASSERT(xgcSyncState_ == XGCSyncState::REMARK_PHASE);
    auto res = xgcBarrier_.Wait(func);
    if (res) {
        xgcSyncState_ = XGCSyncState::NONE;
    }
    return res;
}

void STSVMInterfaceImpl::FinishXGCBarrier()
{
    xgcBarrier_.Wait();
    xgcSyncState_ = XGCSyncState::NONE;
}

void STSVMInterfaceImpl::NotifyWaiters()
{
    xgcBarrier_.Signal();
}

bool STSVMInterfaceImpl::TriggerXGC()
{
    if (!XGC::IsEnabled()) {
        return false;
    }
    auto *gc = vm_->GetGC();
    if (gc == nullptr) {
        return false;
    }
    if (XGC::GetInstance()->IsXGcInProgress()) {
        return false;
    }
    auto task = MakePandaUnique<GCTask>(GCTaskCause::CROSSREF_CAUSE, ark::time::GetCurrentTimeInNanos());
    // isManaged=false since this is called from a JS thread, not an ETS managed thread
    return gc->AddGCTask(false, std::move(task));
}

bool STSVMInterfaceImpl::UnionStackIsEmpty(bool *isEmpty)
{
    return ark::ets::interop::js::UnionStackIsEmpty(isEmpty);
}

bool STSVMInterfaceImpl::ForEachFrameInUnionStack(
    const std::function<void(const void *frame, bool isStaticFrame)> &callback)
{
    return ark::ets::interop::js::ForEachFrameInUnionStack(callback);
}

STSVMInterfaceImpl::VMBarrier::VMBarrier(size_t vmsCount)
{
    os::memory::LockHolder lh(barrierMutex_);
    vmsCount_ = vmsCount;
    currentVMsCount_ = 0U;
    epochCount_ = 0U;
    currentWaitersCount_ = 0U;
    weakCount_ = 0U;
}

void STSVMInterfaceImpl::VMBarrier::Increment()
{
    os::memory::LockHolder lh(barrierMutex_);
    vmsCount_++;
}

void STSVMInterfaceImpl::VMBarrier::Decrement()
{
    os::memory::LockHolder lh(barrierMutex_);
    ASSERT(vmsCount_ > 1);  // after this Decrement, barrier is broken and will not wait correctly
    vmsCount_--;
    Wake();
}

bool STSVMInterfaceImpl::VMBarrier::InitialWait(const NoWorkPred &noWorkPred)
{
    return Wait(noWorkPred, true);
}

bool STSVMInterfaceImpl::VMBarrier::Wait(const NoWorkPred &noWorkPred)
{
    return Wait(noWorkPred, false);
}

bool STSVMInterfaceImpl::VMBarrier::Wait(const NoWorkPred &noWorkPred, bool isInitial)
{
    os::memory::LockHolder lh(barrierMutex_);
    // Need check if noWorkPred exist and if yes, check if it's true. This prevent situation with lost Signal.
    if (CheckNoWorkPred(noWorkPred)) {
        return false;
    }
    size_t epochCount = 0;
    do {
        // Save current epoch to look at it in future.
        epochCount = epochCount_;
        // No we can try to pass the barrier. First we increment count of waiters.
        auto currentWaitersCount = IncrementCurrentWaitersCount();
        // For Inintial barrier we use variable vmsCount_, otherwise we use currentVMsCount_ that can not be checked
        // between 2 Initial barriers.
        size_t waitersCount = 0;
        if (isInitial) {
            waitersCount = vmsCount_;
        } else {
            waitersCount = currentVMsCount_;
        }
        // Next we check if this waiter is the last, if it true, we increase epoch and go out
        if (currentWaitersCount == waitersCount) {
            // for initial barrier we also should set new currentVMsCount_;
            if (isInitial) {
                currentVMsCount_ = vmsCount_;
            }
            epochCount_++;
            Wake();
            return true;
        }
        // We go to wait, it will return true if noWorkPred() returns true
        if (WaitNextEpochOrSignal(noWorkPred)) {
            return false;
        }
    } while (epochCount == epochCount_);
    return true;
}

void STSVMInterfaceImpl::VMBarrier::Signal()
{
    os::memory::LockHolder lh(barrierMutex_);
    Wake();
}

size_t STSVMInterfaceImpl::VMBarrier::IncrementCurrentWaitersCount()
{
    return ++currentWaitersCount_;
}

bool STSVMInterfaceImpl::VMBarrier::WaitNextEpochOrSignal(const NoWorkPred &noWorkPred)
{
    size_t weakCount = weakCount_;
    do {
        condVar_.Wait(&barrierMutex_);
        // This while is needed only to avoid problems with spurious wakeups
    } while (weakCount == weakCount_);
    return CheckNoWorkPred(noWorkPred);
}

void STSVMInterfaceImpl::VMBarrier::Wake()
{
    currentWaitersCount_ = 0;
    weakCount_++;
    condVar_.SignalAll();
}

// --- Hybrid Heapdump Methods ---

bool STSVMInterfaceImpl::TriggerXGCAndWait()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (executionCtx == nullptr) {
        return false;
    }
    ani_env *env = executionCtx->GetPandaAniEnv();
    if (env == nullptr) {
        return false;
    }

    // JSRuntime.xgc() -> long
    ani_class jsRuntimeCls {};
    if (env->FindClass("std.interop.js.JSRuntime", &jsRuntimeCls) != ANI_OK) {
        return false;
    }
    ani_static_method xgcMethod {};
    if (env->Class_FindStaticMethod(jsRuntimeCls, "xgc", ":l", &xgcMethod) != ANI_OK) {
        return false;
    }
    ani_long taskId = 0;
    ani_value xgcArgs {};
    if (env->Class_CallStaticMethod_Long_A(jsRuntimeCls, xgcMethod, &taskId, &xgcArgs) != ANI_OK) {
        return false;
    }

    // GC.waitForFinishGC(taskId)
    ani_namespace gcNamespace {};
    if (env->FindNamespace("std.core.GC", &gcNamespace) != ANI_OK) {
        return false;
    }
    ani_function waitForFinishFunc {};
    if (env->Namespace_FindFunction(gcNamespace, "waitForFinishGC", "l:", &waitForFinishFunc) != ANI_OK) {
        return false;
    }
    ani_value waitForFinishArgs {};
    waitForFinishArgs.l = taskId;
    return env->Function_Call_Void_A(waitForFinishFunc, &waitForFinishArgs) == ANI_OK;
}

void STSVMInterfaceImpl::EtsForceFullGC()
{
    HeapDump::ForceFullGC(vm_);
}

void STSVMInterfaceImpl::SuspendEtsThreads() NO_THREAD_SAFETY_ANALYSIS
{
    ASSERT(vm_ != nullptr);

    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    if (!thread->IsManagedCode()) {
        thread->ManagedCodeBegin();
        dumpManagedScopeActive_ = true;
    }

    auto *rendezvous = vm_->GetRendezvous();
    ASSERT(rendezvous != nullptr);
    ASSERT(rendezvous->GetMutatorLock()->HasLock());
#if defined(ARK_USE_COMMON_RUNTIME)
    Mutator::GetCurrent()->StoreStatus(MutatorStatus::NATIVE);
#endif
    rendezvous->GetMutatorLock()->Unlock();
    rendezvous->SafepointBegin();
}

void STSVMInterfaceImpl::ResumeEtsThreads() NO_THREAD_SAFETY_ANALYSIS
{
    ASSERT(vm_ != nullptr);

    auto *rendezvous = vm_->GetRendezvous();
    ASSERT(rendezvous != nullptr);
    rendezvous->SafepointEnd();
    rendezvous->GetMutatorLock()->ReadLock();
#if defined(ARK_USE_COMMON_RUNTIME)
    Mutator::GetCurrent()->StoreStatus(MutatorStatus::RUNNING);
#endif

    if (dumpManagedScopeActive_) {
        auto *thread = ManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
        thread->ManagedCodeEnd();
        dumpManagedScopeActive_ = false;
    }
}

std::vector<arkplatform::NodeInfo> STSVMInterfaceImpl::GetEtsVMRoots()
{
    std::vector<arkplatform::NodeInfo> roots;
    auto collectRoot = [&roots](const mem::GCRoot &gcRoot) {
        ObjectHeader *rootObject = gcRoot.GetObjectHeader();
        if (rootObject != nullptr) {
            roots.push_back(HeapDump::ObjectToNodeInfo(rootObject));
        }
    };
    mem::RootManager<EtsLanguageConfig> rootManager(vm_);
    rootManager.VisitNonHeapRoots(collectRoot, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    vm_->VisitStringTable(collectRoot, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    return roots;
}

std::vector<arkplatform::NodeInfo> STSVMInterfaceImpl::GetAllEtsObjects()
{
    return HeapDump::GetAllEtsObjects(vm_);
}

void STSVMInterfaceImpl::IterateEtsObjects(const std::function<void(uint64_t)> &callback)
{
    HeapDump::IterateAllObjects(vm_, callback);
}

void STSVMInterfaceImpl::GetEtsNodeEdges(uint64_t etsAddr, std::vector<arkplatform::EdgeInfo> &edges)
{
    auto checker = [](ObjectHeader *obj, const Field &field) -> bool {
        return EtsObject::FromCoreType(obj)->GetClass()->IsWeakReference() &&
               field.GetOffset() == EtsWeakReference::GetReferentOffset();
    };
    HeapDump::DumpReferences(etsAddr, edges, checker);
}

arkplatform::NodeInfo STSVMInterfaceImpl::GetEtsNodeInfo(uint64_t etsAddr)
{
    auto *object = reinterpret_cast<ObjectHeader *>(etsAddr);
    return HeapDump::ObjectToNodeInfo(object);
}

void STSVMInterfaceImpl::GetXRefMaps(uintptr_t ecmaVM, std::unordered_map<uint64_t, uint64_t> &jsToEts,
                                     std::unordered_map<uint64_t, uint64_t> &etsToJs)
{
    auto *storage = ets_proxy::SharedReferenceStorage::GetCurrent();
    if (storage == nullptr) {
        return;
    }
    auto collectXRef = [&](ets_proxy::SharedReference *ref) {
        napi_ref jsRef = ref->GetJsRef();
        EtsObject *etsObj = ref->GetEtsObject();
        if (jsRef == nullptr || etsObj == nullptr) {
            return;
        }
        uintptr_t refVm = 0;
        if (napi_ref_get_vm(jsRef, refVm) != napi_ok || refVm != ecmaVM) {
            return;
        }
        uintptr_t jsAddr = 0;
        if (napi_ref_get_value(jsRef, jsAddr) != napi_ok || jsAddr == 0) {
            return;
        }
        auto etsAddr = reinterpret_cast<uint64_t>(etsObj->GetCoreType());
        if (ref->HasETSFlag()) {
            jsToEts[jsAddr] = etsAddr;
        }
        if (ref->HasJSFlag()) {
            etsToJs[etsAddr] = jsAddr;
        }
    };
    storage->VisitAllRefs(collectXRef);
}

bool STSVMInterfaceImpl::AttachCurrentThread()
{
    auto *etsVm = static_cast<PandaEtsVM *>(Runtime::GetCurrent()->GetPandaVM());
    if (etsVm == nullptr) {
        return false;
    }
    ani_env *env {nullptr};
    auto status = etsVm->AttachCurrentThread(nullptr, ANI_VERSION_1, &env);
    return status == ANI_OK;
}

bool STSVMInterfaceImpl::DetachCurrentThread()
{
    auto *etsVm = static_cast<PandaEtsVM *>(Runtime::GetCurrent()->GetPandaVM());
    if (etsVm == nullptr) {
        return false;
    }
    auto status = etsVm->DetachCurrentThread();
    return status == ANI_OK;
}

bool STSVMInterfaceImpl::IsCurrentThreadAttached()
{
    return Mutator::GetCurrent() != nullptr;
}

}  // namespace ark::ets::interop::js
