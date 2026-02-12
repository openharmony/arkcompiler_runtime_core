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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             cppcoreguidelines-pro-type-union-access, modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_THREAD_MUTATOR_H
#define COMMON_INTERFACES_THREAD_MUTATOR_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include "common_interfaces/base/common.h"
#include "common_interfaces/objects/base_object.h"

namespace common {
class Mutator;
class ThreadHolder;
struct ThreadLocalData;

// GCPhase describes phases for stw/concurrent gc.
enum GCPhase : uint8_t {
    GC_PHASE_UNDEF = 0,
    GC_PHASE_IDLE = 1,
    GC_PHASE_START = 8,

    // only gc phase after GC_PHASE_START ( enum value > GC_PHASE_START) needs barrier.
    GC_PHASE_ENUM = 9,
    GC_PHASE_MARK = 10,
    GC_PHASE_REMARK_SATB = 11,
    GC_PHASE_FINAL_MARK = 12,
    GC_PHASE_POST_MARK = 13,
    GC_PHASE_PRECOPY = 14,
    GC_PHASE_COPY = 15,
    GC_PHASE_FIX = 16,
};

class Mutator;
using FlipFunction = std::function<void(Mutator&)>;
class PUBLIC_API Mutator {
public:
    // flag which indicates the reason why mutator should suspend. flag is set by some external thread.
    enum SuspensionType : uint32_t {
        SUSPENSION_FOR_GC_PHASE = 1 << 0,
        SUSPENSION_FOR_STW = 1 << 1,
        SUSPENSION_FOR_EXIT = 1 << 2,
        SUSPENSION_FOR_CPU_PROFILE = 1 << 3,
        SUSPENSION_FOR_PENDING_CALLBACK = 1 << 4,
        SUSPENSION_FOR_RUNNING_CALLBACK = 1 << 5,
        /**
         * The below ones are not actually suspension request, it's just some callbacks need to process
         * at the beginning of transfering to RUNNING
         * So This is equivalent to:
         * ````    __attribute__((always_inline)) inline void DoLeaveSaferegion()
         * ````    {
         * ````        SetInSaferegion(SAFE_REGION_FALSE);
         * ````        if (UNLIKELY_CC(HasAnySuspensionRequest())) {
         * ````            HandleSuspensionRequest();
         * ````        }
         * ------>     if (UNLIKELY_CC(HasOtherCallback())) {
         * ------>         ProcessCallback();
         * ------>     }
         * ````    }
         * But this will make `DoLeaveSaferegion` cost more, so we just merge it into suspension request,
         * and do some extra process at the end of `HandleSuspensionRequest`
        */
        SUSPENSION_FOR_FINALIZE = 1U << 31,
        CALLBACKS_TO_PROCESS = SUSPENSION_FOR_FINALIZE,
    };

    enum SaferegionState : uint32_t {
        SAFE_REGION_TRUE = 0x17161514,
        SAFE_REGION_FALSE = 0x03020100,
    };

    enum GCPhaseTransitionState : uint32_t {
        NO_TRANSITION,
        NEED_TRANSITION,
        IN_TRANSITION,
        FINISH_TRANSITION,
    };

    enum CpuProfileState : uint32_t {
        NO_CPUPROFILE,
        NEED_CPUPROFILE,
        IN_CPUPROFILING,
        FINISH_CPUPROFILE,
    };

    // Called when a mutator starts and finishes, respectively
    void Init();

    ~Mutator();

    static Mutator* NewMutator();

    void ResetMutator();

    static Mutator* GetMutator() noexcept;

    void InitTid();
    void SetArkthreadPtr(void* threadPtr) { this->thread_ = threadPtr;}
    void SetEcmaVMPtr(void* ecmaVMPtr) { this->ecmavm_ = ecmaVMPtr;}
    uint32_t GetTid() const { return tid_; }
    void* GetArkthreadPtr() const {return thread_;}
    void* GetEcmaVMPtr() const {return ecmavm_;}

    static uint32_t ConstructSuspensionFlag(uint32_t flag, uint32_t clearFlag, uint32_t setFlag)
    {
        return (flag & ~clearFlag) | setFlag;
    }

    __attribute__((always_inline)) inline void SetSafepointActive(bool state)
    {
        safepointActive_ = static_cast<uint32_t>(state);
    }

    bool GetSafepointActiveState() const
    {
        return safepointActive_;
    }

    // Sets saferegion state of this mutator.
    __attribute__((always_inline)) inline void SetInSaferegion(SaferegionState state)
    {
        // assure sequential execution of setting insaferegion state and checking suspended state.
        inSaferegion_.store(state, std::memory_order_seq_cst);
    }

    // Returns true if this mutator is in saferegion, otherwise false.
    __attribute__((always_inline)) inline bool InSaferegion() const
    {
        return inSaferegion_.load(std::memory_order_seq_cst) != SAFE_REGION_FALSE;
    }

    // Force current mutator enter saferegion, internal use only.
    __attribute__((always_inline)) inline void DoEnterSaferegion();
    // Force current mutator leave saferegion, internal use only.
    __attribute__((always_inline)) inline void DoLeaveSaferegion()
    {
        SetInSaferegion(SAFE_REGION_FALSE);
        // go slow path if the mutator should suspend
        if (UNLIKELY_CC(HasAnySuspensionRequest())) {
            HandleSuspensionRequest();
        }
    }

    // If current mutator is not in saferegion, enter and return true
    // If current mutator has been in saferegion, return false
    __attribute__((always_inline)) inline bool EnterSaferegion(bool updateUnwindContext) noexcept;
    // If current mutator is in saferegion, leave and return true
    // If current mutator has left saferegion, return false
    __attribute__((always_inline)) inline bool LeaveSaferegion() noexcept;

    __attribute__((always_inline)) inline bool FinishedTransition() const
    {
        return transitionState_ == FINISH_TRANSITION;
    }

    __attribute__((always_inline)) inline bool FinishedCpuProfile() const
    {
        return cpuProfileState_.load(std::memory_order_acquire) == FINISH_CPUPROFILE;
    }

    __attribute__((always_inline)) inline void SetCpuProfileState(CpuProfileState state)
    {
        cpuProfileState_.store(state, std::memory_order_relaxed);
    }

    // Spin wait phase transition finished when GC is tranverting this mutator's phase
    __attribute__((always_inline)) inline void WaitForPhaseTransition() const
    {
        GCPhaseTransitionState state = transitionState_.load(std::memory_order_acquire);
        while (state != FINISH_TRANSITION) {
            if (state != IN_TRANSITION) {
                return;
            }
            // Give up CPU to avoid overloading
            (void)sched_yield();
            state = transitionState_.load(std::memory_order_acquire);
        }
    }

    __attribute__((always_inline)) inline void WaitForCpuProfiling() const
    {
        while (cpuProfileState_.load(std::memory_order_acquire) != FINISH_CPUPROFILE) {
            // Give up CPU to avoid overloading
            (void)sched_yield();
        }
    }

    __attribute__((always_inline)) inline void SetSuspensionFlag(SuspensionType flag)
    {
        if (flag == SUSPENSION_FOR_GC_PHASE) {
            transitionState_.store(NEED_TRANSITION, std::memory_order_relaxed);
        } else if (flag == SUSPENSION_FOR_CPU_PROFILE) {
            cpuProfileState_.store(NEED_CPUPROFILE, std::memory_order_relaxed);
        }
        suspensionFlag_.fetch_or(flag, std::memory_order_seq_cst);
    }

    __attribute__((always_inline)) inline void ClearSuspensionFlag(SuspensionType flag)
    {
        suspensionFlag_.fetch_and(~flag, std::memory_order_seq_cst);
    }

    __attribute__((always_inline)) inline uint32_t GetSuspensionFlag() const
    {
        return suspensionFlag_.load(std::memory_order_acquire);
    }

    __attribute__((always_inline)) inline bool HasSuspensionRequest(SuspensionType flag) const
    {
        return (suspensionFlag_.load(std::memory_order_acquire) & flag) != 0;
    }

    // Check whether current mutator needs to be suspended for GC or other request
    __attribute__((always_inline)) inline bool HasAnySuspensionRequest() const
    {
        return (suspensionFlag_.load(std::memory_order_acquire) != 0);
    }

    // Check whether current mutator needs to be suspended for GC or other request, see comments in `SuspensionType`
    __attribute__((always_inline)) inline bool HasAnySuspensionRequestExceptCallbacks() const
    {
        uint32_t flag = suspensionFlag_.load(std::memory_order_acquire);
        return (flag & ~CALLBACKS_TO_PROCESS) != 0;
    }

    __attribute__((always_inline)) inline void ClearFinalizeRequest()
    {
        ClearSuspensionFlag(SUSPENSION_FOR_FINALIZE);
    }

    __attribute__((always_inline)) inline void SetFinalizeRequest()
    {
        SetSuspensionFlag(SUSPENSION_FOR_FINALIZE);
    }

    __attribute__((always_inline)) inline bool CASSetSuspensionFlag(uint32_t oldFlag, uint32_t newFlag)
    {
        return suspensionFlag_.compare_exchange_strong(oldFlag, newFlag);
    }

    // Called if current mutator should do corresponding task by suspensionFlag value
    __attribute__((visibility ("default"))) void HandleSuspensionRequest();

    __attribute__((always_inline)) inline void SetMutatorPhase(const GCPhase newPhase)
    {
        mutatorPhase_.store(newPhase, std::memory_order_release);
    }

    __attribute__((always_inline)) inline GCPhase GetMutatorPhase() const
    {
        return mutatorPhase_.load(std::memory_order_acquire);
    }

    // Called if current mutator should handle stw request
    void SuspendForStw();

    // temporary impl to clean GC callback, and need to refact to flip function
    __attribute__((visibility ("default"))) void HandleJSGCCallback();

    void GcPhaseEnum(GCPhase newPhase);
    void GCPhasePreForward(GCPhase newPhase);
    void HandleGCPhase(GCPhase newPhase);

    inline void HandleCpuProfile();

    void TransitionToCpuProfileExclusive();

    // Ensure that mutator phase is changed only once by mutator itself or GC
    bool TransitionGCPhase(bool bySelf);

    __attribute__((always_inline)) inline bool TransitionToCpuProfile(bool bySelf);

    void VisitMutatorRoots(const RootVisitor& visitor);

    void DumpMutator() const;

    // Init after fork.
    void InitAfterFork()
    {
        // tid changed after fork, so we re-initialize it.
        InitTid();
    }

    void SetFlipFunction(FlipFunction *flipFunction)
    {
        flipFunction_ = flipFunction;
    }

    void TransitionToGCPhaseExclusive(GCPhase newPhase);

    bool TryRunFlipFunction()
    {
        while (true) {
            uint32_t oldFlag = GetSuspensionFlag();
            if ((oldFlag & SuspensionType::SUSPENSION_FOR_PENDING_CALLBACK) != 0) {
                uint32_t newFlag =
                    Mutator::ConstructSuspensionFlag(oldFlag, SuspensionType::SUSPENSION_FOR_PENDING_CALLBACK,
                                                         SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK);
                if (CASSetSuspensionFlag(oldFlag, newFlag)) {
                    DCHECK_CC(flipFunction_);
                    (*flipFunction_)(*this);
                    flipFunction_ = nullptr;
                    std::unique_lock<std::mutex> lock(flipFunctionMtx_);
                    ClearSuspensionFlag(SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK);
                    flipFunctionCV_.notify_all();
                    return true;
                }
            } else {
                return false;
            }
        }
    }

    void WaitFlipFunctionFinish()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(flipFunctionMtx_);
            if (HasSuspensionRequest(SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK)) {
                flipFunctionCV_.wait(lock);
            } else {
                return;
            }
        }
    }

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void PushFrameInfoForMarking(const GCInfoNode& frameGCInfo) { gcInfos_.PushFrameInfoForMarking(frameGCInfo); }

    void PushFrameInfoForMarking(const GCInfoNode&& frameGCInfo) { gcInfos_.PushFrameInfoForMarking(frameGCInfo); }

    void PushFrameInfoForFix(const GCInfoNodeForFix& frameGCInfo) { gcInfos_.PushFrameInfoForFix(frameGCInfo); }

    void PushFrameInfoForFix(const GCInfoNodeForFix&& frameGCInfo) { gcInfos_.PushFrameInfoForFix(frameGCInfo); }

    void DumpGCInfos() const
    {
        DLOG(ENUM, "dump mutator gc info thread id: %d", tid);
        gcInfos_.DumpGCInfos();
    }
#endif

    NO_INLINE_CC void RememberObjectInSatbBuffer(const BaseObject* obj) { RememberObjectImpl(obj); }

    const void* GetSatbBufferNode() const;

    void ClearSatbBufferNode();

    void PushRawObject(BaseObject* obj) { rawObject_.object = obj; }

    BaseObject* PopRawObject()
    {
        BaseObject* obj = rawObject_.object;
        rawObject_.object = nullptr;
        return obj;
    }

    ThreadHolder *GetThreadHolder()
    {
        return holder_;
    }

    void RegisterJSThread(void *jsThread)
    {
        CHECK_CC(jsThread_ == nullptr);
        jsThread_ = jsThread;
    }

    void UnregisterJSThread()
    {
        jsThread_ = nullptr;
    }

    static constexpr size_t GetSafepointActiveOffset()
    {
        return MEMBER_OFFSET_CC(Mutator, safepointActive_);
    }

protected:
    // for exception ref
    void VisitRawObjects(const RootVisitor& func);
    void CreateCurrentGCInfo();

private:
    void RememberObjectImpl(const BaseObject* obj);

    // Indicate whether execution thread should check safepoint suspension request
    uint32_t safepointActive_ = 0;
    // Indicate the current mutator phase and use which barrier in concurrent gc
    std::atomic<GCPhase> mutatorPhase_ = { GCPhase::GC_PHASE_UNDEF };
    // in saferegion, it will not access any managed objects and can be visitted by observer
    std::atomic<uint32_t> inSaferegion_ = { SAFE_REGION_TRUE };
    // If set implies this mutator should process suspension requests
    std::atomic<uint32_t> suspensionFlag_ = { 0 };
    // Indicate the state of mutator's phase transition
    std::atomic<GCPhaseTransitionState> transitionState_ = { NO_TRANSITION };

    std::atomic<CpuProfileState> cpuProfileState_ = { NO_CPUPROFILE };

    // used to synchronize cmc-gc phase to JSThread
    void *jsThread_ {nullptr};

    // thread id
    uint32_t tid_ = 0;
    // thread ptr
    void* thread_;
    // ecmavm
    void* ecmavm_ = nullptr;

    ObjectRef rawObject_{ nullptr };
    std::mutex flipFunctionMtx_;
    std::condition_variable flipFunctionCV_;
    FlipFunction* flipFunction_ {nullptr};
    void* satbNode_ = nullptr;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    GCInfos gcInfos_;
#endif

    ThreadHolder *holder_;

    friend ThreadHolder;
};

// This function is mainly used to initialize the context of mutator.
// Ensured that updated fa is the caller layer of the managed function to be called.
void PreRunManagedCode(Mutator* mutator, int layers, ThreadLocalData* threadData);

ThreadLocalData *GetThreadLocalData();

}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_MUTATOR_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           cppcoreguidelines-pro-type-union-access, modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)
