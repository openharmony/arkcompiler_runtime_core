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

#include <algorithm>
#include <cstdint>
#include <stack>
#include <unistd.h>

#include "common_components/common_runtime/hooks.h"
#include "common_components/common/type_def.h"
#include "mutator/satb_buffer.h"
#if defined(_WIN64)
#define NOGDI
#include <windows.h>
#endif
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_components/common/scoped_object_access.h"
#include "common_components/mutator/mutator_manager.h"

#include "common_interfaces/thread/mutator-inl.h"
#include "common_interfaces/thread/mutator_state_transition.h"

namespace common_vm {

ThreadLocalData *GetThreadLocalData()
{
    uintptr_t tlDataAddr = reinterpret_cast<uintptr_t>(ThreadLocal::GetThreadLocalData());
#if defined(__aarch64__)
    if (Heap::GetHeap().IsGcStarted()) {
        // Since the TBI(top bit ignore) feature in Aarch64,
        // set gc phase to high 8-bit of ThreadLocalData Address for gc barrier fast path.
        // 56: make gcphase value shift left 56 bit to set the high 8-bit
        tlDataAddr = tlDataAddr | (static_cast<uint64_t>(Heap::GetHeap().GetGCPhase()) << 56);
    }
#endif
    return reinterpret_cast<ThreadLocalData *>(tlDataAddr);
}

// Ensure that mutator phase is changed only once by mutator itself or GC
bool Mutator::TransitionGCPhase(bool bySelf)
{
    // CC-OFFNXT(G.CTL.03): false positive
    do {
        GCPhaseTransitionState state = transitionState_.load();
        // If this mutator phase transition has finished, just return
        if (state == FINISH_TRANSITION) {
            bool result = mutatorPhase_.load() == Heap::GetHeap().GetGCPhase();
            if (!bySelf && !result) {  // why check bySelf?
                LOG_COMMON(FATAL) << "Unresolved fatal";
                UNREACHABLE_CC();
            }
            return result;
        }

        // If this mutator is executing phase transition by other thread, mutator should wait but GC just return
        if (state == IN_TRANSITION) {
            if (bySelf) {
                WaitForPhaseTransition();
                return true;
            } else {
                return false;
            }
        }

        if (!bySelf && state == NO_TRANSITION) {
            return true;
        }

        // Current thread set atomic variable to ensure atomicity of phase transition
        CHECK_CC(state == NEED_TRANSITION);
        if (transitionState_.compare_exchange_weak(state, IN_TRANSITION)) {
            TransitionToGCPhaseExclusive(Heap::GetHeap().GetGCPhase());
            transitionState_.store(FINISH_TRANSITION, std::memory_order_release);
            return true;
        }
    } while (true);
}

void Mutator::HandleSuspensionRequest()
{
    // CC-OFFNXT(G.CTL.03): false positive
    for (;;) {
        SetInSaferegion(SAFE_REGION_TRUE);
        if (HasSuspensionRequest(SUSPENSION_FOR_STW)) {
            SuspendForStw();
            if (HasSuspensionRequest(SUSPENSION_FOR_GC_PHASE)) {
                TransitionGCPhase(true);
            } else if (HasSuspensionRequest(SUSPENSION_FOR_CPU_PROFILE)) {
                TransitionToCpuProfile(true);
            }
        } else if (HasSuspensionRequest(SUSPENSION_FOR_GC_PHASE)) {
            TransitionGCPhase(true);
        } else if (HasSuspensionRequest(SUSPENSION_FOR_CPU_PROFILE)) {
            TransitionToCpuProfile(true);
        } else if (HasSuspensionRequest(SUSPENSION_FOR_EXIT)) {
            // CC-OFFNXT(G.CTL.03): required by program logic
            while (true) {
                sleep(INT_MAX);
            }
        } else if (HasSuspensionRequest(SUSPENSION_FOR_PENDING_CALLBACK)) {
            TryRunFlipFunction();
        } else if (HasSuspensionRequest(SUSPENSION_FOR_RUNNING_CALLBACK)) {
            WaitFlipFunctionFinish();
        }
        SetInSaferegion(SAFE_REGION_FALSE);
        // Leave saferegion if current mutator has no suspend request, otherwise try again
        if (LIKELY_CC(!HasAnySuspensionRequestExceptCallbacks())) {
            if (HasSuspensionRequest(SUSPENSION_FOR_FINALIZE)) {
                ClearFinalizeRequest();
                HandleGCCallback();
            }
            return;
        }
    }
}

void Mutator::HandleGCCallback()
{
    void *vm = GetEcmaVMPtr();
    if (vm != nullptr) {
        JSGCCallback(vm);
    }
    BaseRuntime::GetInstance()->ForEachVM([](VMInterface *vm) { vm->ProcessFinalizationRegistryCleanup(); });
}

void Mutator::SuspendForStw()
{
    ClearSuspensionFlag(SUSPENSION_FOR_STW);
    // wait until StartTheWorld
    int curCount = static_cast<int>(MutatorManager::Instance().GetStwFutexWordValue());
    // Avoid losing wake-ups
    if (curCount > 0) {
#if defined(_WIN64) || defined(__APPLE__)
        MutatorManager::Instance().MutatorWait();
#else
        int *countAddr = MutatorManager::Instance().GetStwFutexWord();
        // FUTEX_WAIT may fail when gc thread wakes up all threads before the current thread reaches this position.
        // But it is not important because there won't be data race between the current thread and the gc thread,
        // and it also won't be frozen since gc thread also modifies the value at countAddr before its waking option.
        (void)Futex(countAddr, FUTEX_WAIT, curCount);
#endif
    }
    SetInSaferegion(SAFE_REGION_FALSE);
    if (MutatorManager::Instance().StwTriggered()) {
        // entering this branch means a second request has been broadcasted, we need to reset this flag to avoid
        // missing the request. And this must be after the behaviour that set saferegion state to false, because
        // we need to make sure that the mutator can always perceive the gc request when the mutator is not in
        // safe region.
        SetSuspensionFlag(SUSPENSION_FOR_STW);
    }
}

static SatbBuffer::TreapNode *&CastSatbNode(void *&satbNode)
{
    return reinterpret_cast<SatbBuffer::TreapNode *&>(satbNode);
}

Mutator::~Mutator()
{
    tid_ = 0;
    if (satbNode_ != nullptr) {
        SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
        satbNode_ = nullptr;
    }
}

Mutator *Mutator::NewMutator()
{
    Mutator *mutator = new (std::nothrow) Mutator();
    LOGF_CHECK(mutator != nullptr) << "new Mutator failed";
    return mutator;
}

void Mutator::RememberObjectImpl(const BaseObject *obj)
{
    if (LIKELY_CC(Heap::IsHeapAddress(obj))) {
        if (SatbBuffer::ShouldEnqueue(obj)) {
            SatbBuffer::Instance().EnsureGoodNode(CastSatbNode(satbNode_));
            CastSatbNode(satbNode_)->Push(obj);
        }
    }
}

void Mutator::ResetMutator()
{
    rawObject_.object = nullptr;
    if (satbNode_ != nullptr) {
        SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
        satbNode_ = nullptr;
    }
    suspensionFlag_.store(0, std::memory_order_relaxed);
    flipFunction_ = nullptr;
    transitionState_.store(NO_TRANSITION, std::memory_order_relaxed);
}

void Mutator::InitTid()
{
    tid_ = ThreadLocal::GetThreadLocalData()->tid;
    if (tid_ == 0) {
        tid_ = static_cast<uint32_t>(GetTid());
        ThreadLocal::GetThreadLocalData()->tid = tid_;
    }
}

const void *Mutator::GetSatbBufferNode() const
{
    return satbNode_;
}

void Mutator::ClearSatbBufferNode()
{
    if (satbNode_ == nullptr) {
        return;
    }
    CastSatbNode(satbNode_)->Clear();
}

void Mutator::VisitMutatorRoots(const RefFieldVisitor &visitor)
{
}

void Mutator::UpdateReadBarrierEntrypoint(common_vm::GCPhase phase)
{
}

void Mutator::DumpMutator() const
{
    LOG_COMMON(ERROR) << "mutator " << this << ": inSaferegion " << inSaferegion_.load(std::memory_order_relaxed)
                      << ", tid " << tid_ << ", gc phase: " << mutatorPhase_.load() << ", suspension request "
                      << suspensionFlag_.load();
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void Mutator::CreateCurrentGCInfo()
{
    gcInfos_.CreateCurrentGCInfo();
}
#endif

void Mutator::VisitRawObjects(const RootVisitor &func)
{
    if (rawObject_.object != nullptr) {
        func(rawObject_);
    }
}

Mutator *Mutator::GetMutator() noexcept
{
    return ThreadLocal::GetMutator();
}

inline void CheckAndPush(BaseObject *obj, std::set<BaseObject *> &rootSet, std::stack<BaseObject *> &rootStack)
{
    auto search = rootSet.find(obj);
    if (search == rootSet.end()) {
        rootSet.insert(obj);
        if (obj->IsValidObject() && obj->HasRefField()) {
            rootStack.push(obj);
        }
    }
}

void Mutator::GcPhaseEnum(GCPhase newPhase) {}

// comment all
void Mutator::GCPhasePreForward(GCPhase newPhase) {}

void Mutator::HandleGCPhase(GCPhase newPhase)
{
    if (newPhase == GCPhase::GC_PHASE_POST_MARK) {
        if (satbNode_ != nullptr) {
            DCHECK_CC(CastSatbNode(satbNode_)->IsEmpty());
            SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
            satbNode_ = nullptr;
        }
    } else if (newPhase == GCPhase::GC_PHASE_ENUM) {
        GcPhaseEnum(newPhase);
    } else if (newPhase == GCPhase::GC_PHASE_PRECOPY) {
        GCPhasePreForward(newPhase);
    } else if (newPhase == GCPhase::GC_PHASE_REMARK_SATB || newPhase == GCPhase::GC_PHASE_FINAL_MARK) {
        if (satbNode_ != nullptr) {
            SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
            satbNode_ = nullptr;
        }
    }
}

void Mutator::TransitionToGCPhaseExclusive(GCPhase newPhase)
{
    HandleGCPhase(newPhase);
    SetSafepointActive(false);
    mutatorPhase_.store(newPhase, std::memory_order_relaxed); // handshake between mutator & mainGC thread
    BaseRuntime::GetInstance()->ForEachVM(
        [m = this, newPhase](VMInterface *vm) { m->UpdateReadBarrierEntrypoint(newPhase); });
    // Clear mutator's suspend request after phase transition
    ClearSuspensionFlag(SUSPENSION_FOR_GC_PHASE);  // atomic seq-cst
}

inline void Mutator::HandleCpuProfile()
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

void Mutator::TransitionToCpuProfileExclusive()
{
    HandleCpuProfile();
    SetSafepointActive(false);
    ClearSuspensionFlag(SUSPENSION_FOR_CPU_PROFILE);
}

void PreRunManagedCode(Mutator *mutator, int layers, ThreadLocalData *threadData)
{
    if (UNLIKELY_CC(MutatorManager::Instance().StwTriggered())) {
        mutator->SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_STW);
        mutator->EnterSaferegion(false);
    }
    mutator->LeaveSaferegion();
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
}

void Mutator::RegisterNewMutator(Mutator *mutator)
{
    auto &mutatorManager = MutatorManager::Instance();
    mutatorManager.MutatorManagementRLock();
    GCPhase phase = Heap::GetHeap().GetGCPhase();
    {
        std::lock_guard<std::mutex> guard(mutatorManager.allMutatorListLock_);
        DCHECK_CC(std::find(mutatorManager.allMutatorList_.begin(),
                            mutatorManager.allMutatorList_.end(), mutator) == mutatorManager.allMutatorList_.end());
        if (UNLIKELY_CC(mutatorManager.StwTriggered())) {
            mutator->SetSafepointActive(true);
            mutator->SetSuspensionFlag(SuspensionType::SUSPENSION_FOR_STW);
        }
        mutatorManager.allMutatorList_.push_back(mutator);
    }
    mutator->SetMutatorPhase(phase);
    // Enable read barrier for mutators created during concurrent copy/fix.
    if (phase >= GCPhase::GC_PHASE_PRECOPY) {
        mutator->TransitionToGCPhaseExclusive(phase);
    }
    mutatorManager.MutatorManagementRUnlock();
}

bool Mutator::TryBindMutator()
{
    if (ThreadLocal::IsArkProcessor()) {
        return false;
    }

    auto &mutatorManager = MutatorManager::Instance();
    mutatorManager.BindMutator(*this);

    allocBuffer_ = ThreadLocal::GetAllocBuffer();
    DCHECK_CC(allocBuffer_ != nullptr);
    return true;
}

void Mutator::BindMutator()
{
    DCHECK_CC(!IsInRunningState());
    Mutator *curr = ThreadLocal::GetMutator();
    DCHECK_CC(curr == nullptr || !curr->IsInRunningState());

    if (!TryBindMutator()) {
        LOG_COMMON(FATAL) << "BindMutator fail";
        return;
    }
}

void Mutator::UnbindMutator()
{
    DCHECK_CC(!IsInRunningState());
    allocBuffer_ = nullptr;
    auto &mutatorManager = MutatorManager::Instance();
    mutatorManager.UnbindMutator(*this);
}

void Mutator::ReleaseAllocBuffer()
{
    MutatorManagedScope scope(this);
    allocBuffer_ = nullptr;
}

void Mutator::UnregisterMutator(Mutator *mutator)
{
    DCHECK_CC(!mutator->IsInRunningState());
    mutator->ReleaseAllocBuffer();

    auto &mutatorManager = MutatorManager::Instance();
    mutatorManager.MutatorManagementRLock();
    {
        std::lock_guard<std::mutex> guard(mutatorManager.allMutatorListLock_);
        auto &list = mutatorManager.allMutatorList_;
        auto it = std::find(list.begin(), list.end(), mutator);
        if (it != list.end()) {
            list.erase(it);
        }
    }
    mutator->ResetMutator();
    mutatorManager.MutatorManagementRUnlock();
}

Mutator::TryBindMutatorScope::TryBindMutatorScope(Mutator *mutator) : mutator_(nullptr)
{
    if (mutator != nullptr && mutator->TryBindMutator()) {
        mutator_ = mutator;
    }
}

Mutator::TryBindMutatorScope::~TryBindMutatorScope()
{
    if (mutator_ != nullptr) {
        mutator_->ReleaseAllocBuffer();
        mutator_->UnbindMutator();
        mutator_ = nullptr;
    }
}

}  // namespace common_vm
