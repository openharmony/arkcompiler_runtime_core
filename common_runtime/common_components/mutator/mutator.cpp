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

#include "common_interfaces/thread/thread_holder.h"

namespace common {
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
            if (!bySelf && !result) { // why check bySelf?
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
                HandleJSGCCallback();
            }
            return;
        }
    }
}

void Mutator::HandleJSGCCallback()
{
    void *vm = GetEcmaVMPtr();
    if (vm != nullptr) {
        JSGCCallback(vm);
    }
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
        int* countAddr = MutatorManager::Instance().GetStwFutexWord();
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

static SatbBuffer::TreapNode*& CastSatbNode(void *&satbNode)
{
    return reinterpret_cast<SatbBuffer::TreapNode*&>(satbNode);
}

void Mutator::Init()
{
    holder_ = new ThreadHolder(this);
    mutatorPhase_.store(GCPhase::GC_PHASE_IDLE);
}

Mutator::~Mutator()
{
    tid_ = 0;
    if (satbNode_ != nullptr) {
        SatbBuffer::Instance().RetireNode(CastSatbNode(satbNode_));
        satbNode_ = nullptr;
    }
    delete holder_;
}

Mutator* Mutator::NewMutator()
{
    Mutator* mutator = new (std::nothrow) Mutator();
    LOGF_CHECK(mutator != nullptr) << "new Mutator failed";
    mutator->Init();
    return mutator;
}

void Mutator::RememberObjectImpl(const BaseObject* obj)
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
}

void Mutator::InitTid()
{
    tid_ = ThreadLocal::GetThreadLocalData()->tid;
    if (tid_ == 0) {
        tid_ = static_cast<uint32_t>(GetTid());
        ThreadLocal::GetThreadLocalData()->tid = tid_;
    }
}

const void* Mutator::GetSatbBufferNode() const { return satbNode_; }

void Mutator::ClearSatbBufferNode()
{
    if (satbNode_ == nullptr) {
        return;
    }
    CastSatbNode(satbNode_)->Clear();
}

void Mutator::VisitMutatorRoots(const RootVisitor& visitor)
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

void Mutator::DumpMutator() const
{
    LOG_COMMON(ERROR) << "mutator " << this << ": inSaferegion " <<
        inSaferegion_.load(std::memory_order_relaxed) << ", tid " << tid_ <<
        ", gc phase: " << mutatorPhase_.load() << ", suspension request "<< suspensionFlag_.load();
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void Mutator::CreateCurrentGCInfo() { gcInfos_.CreateCurrentGCInfo(); }
#endif


void Mutator::VisitRawObjects(const RootVisitor& func)
{
    if (rawObject_.object != nullptr) {
        func(rawObject_);
    }
}

Mutator* Mutator::GetMutator() noexcept
{
    return ThreadLocal::GetMutator();
}

inline void CheckAndPush(BaseObject* obj, std::set<BaseObject*>& rootSet, std::stack<BaseObject*>& rootStack)
{
    auto search = rootSet.find(obj);
    if (search == rootSet.end()) {
        rootSet.insert(obj);
        if (obj->IsValidObject() && obj->HasRefField()) {
            rootStack.push(obj);
        }
    }
}

void Mutator::GcPhaseEnum(GCPhase newPhase)
{
}

// comment all
void Mutator::GCPhasePreForward(GCPhase newPhase)
{
}

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
    if (jsThread_ != nullptr) {
        // non-atomic, should update JSThread local gc state before SuspensionFlag store,
        // and SuspensionFlag load when transfer to running will guarantee the visibility of
        // the JSThread local gc state
        SynchronizeGCPhaseToJSThread(jsThread_, newPhase);
    }
    // Clear mutator's suspend request after phase transition
    ClearSuspensionFlag(SUSPENSION_FOR_GC_PHASE); // atomic seq-cst
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

void PreRunManagedCode(Mutator* mutator, int layers, ThreadLocalData* threadData)
{
    if (UNLIKELY_CC(MutatorManager::Instance().StwTriggered())) {
        mutator->SetSuspensionFlag(Mutator::SuspensionType::SUSPENSION_FOR_STW);
        mutator->EnterSaferegion(false);
    }
    mutator->LeaveSaferegion();
    mutator->SetMutatorPhase(Heap::GetHeap().GetGCPhase());
}

} // namespace common
