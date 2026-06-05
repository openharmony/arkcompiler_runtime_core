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

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_H
#define COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_H

#include <atomic>
#include <unordered_set>

#include "common_interfaces/base/common.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/objects/base_object.h"
#include "runtime/include/mutator_status.h"
#include "libarkbase/os/mutex.h"

namespace panda::ecmascript {
class JSThread;
}

namespace ark {
class Coroutine;
}

namespace ark::common_vm {
class Mutator;
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
    GC_PHASE_YOUNG_COPY = 17,
};

class Mutator;
using FlipFunction = std::function<void(Mutator &)>;
class PANDA_PUBLIC_API Mutator {
public:
    // NOTE(ivagin): language specififc code will be removed in the next patches
    using JSThread = panda::ecmascript::JSThread;
    using Coroutine = ark::Coroutine;

    enum GCPhaseTransitionState : uint32_t {
        NO_TRANSITION,
        NEED_TRANSITION,
        IN_TRANSITION,
        FINISH_TRANSITION,
    };

    virtual ~Mutator();

    static Mutator *NewMutator();

    void ResetMutator();

    static Mutator *GetMutator() noexcept;

    void InitTid();
    uint32_t GetTid() const
    {
        return tid_;
    }

    static uint32_t ConstructSuspensionFlag(uint32_t flag, uint32_t clearFlag, uint32_t setFlag)
    {
        return (flag & ~clearFlag) | setFlag;
    }

    // Sets saferegion state of this mutator.
    __attribute__((always_inline)) inline void SetInSaferegion(ark::MutatorStatus state);

    // Returns true if this mutator is in saferegion, otherwise false.
    __attribute__((always_inline)) inline bool InSaferegion() const;

    // Force current mutator enter saferegion, internal use only.
    __attribute__((always_inline)) inline void DoEnterSaferegion() NO_THREAD_SAFETY_ANALYSIS;
    // Force current mutator leave saferegion, internal use only.
    __attribute__((always_inline)) inline void DoLeaveSaferegion();

    // If current mutator is not in saferegion, enter and return true
    // If current mutator has been in saferegion, return false
    __attribute__((always_inline)) inline bool EnterSaferegion(bool updateUnwindContext) noexcept;
    // If current mutator is in saferegion, leave and return true
    // If current mutator has left saferegion, return false
    __attribute__((always_inline)) inline bool LeaveSaferegion() noexcept;

    __attribute__((always_inline)) inline bool FinishedTransition() const
    {
        // Atomic with acquire order reason: data race with transitionState_ with dependecies on reads after the load
        return transitionState_.load(std::memory_order_acquire) == FINISH_TRANSITION;
    }

    __attribute__((always_inline)) inline void SetSuspensionFlag(ark::MutatorFlag flag);

    __attribute__((always_inline)) inline void ClearSuspensionFlag(ark::MutatorFlag flag);

    __attribute__((always_inline)) inline uint32_t GetSuspensionFlag() const;

    __attribute__((always_inline)) inline bool HasSuspensionRequest(ark::MutatorFlag flag) const;

    // Check whether current mutator needs to be suspended for GC or other request
    __attribute__((always_inline)) inline bool HasAnySuspensionRequest() const;

    // Check whether current mutator needs to be suspended for GC or other request, see comments in `ark::MutatorFlag`
    __attribute__((always_inline)) inline bool HasAnySuspensionRequestExceptCallbacks() const;

    __attribute__((always_inline)) inline void ClearFinalizeRequest();

    __attribute__((always_inline)) inline void SetFinalizeRequest();

    __attribute__((always_inline)) inline bool CASSetSuspensionFlag(uint32_t oldFlag, uint32_t newFlag);

    // Called if current mutator should do corresponding task by suspensionFlag value
    __attribute__((visibility("default"))) void HandleSuspensionRequest();

    __attribute__((always_inline)) inline void SetMutatorPhase(const GCPhase newPhase)
    {
        // Atomic with release order reason: data race with mutatorPhase_ with dependecies on writes before the store
        mutatorPhase_.store(newPhase, std::memory_order_release);
    }

    __attribute__((always_inline)) inline GCPhase GetMutatorPhase() const
    {
        // Atomic with acquire order reason: data race with mutatorPhase_ with dependecies on reads after the load
        return mutatorPhase_.load(std::memory_order_acquire);
    }

    // Called if current mutator should handle stw request
    void SuspendForStw();

    // temporary impl to clean GC callback, and need to refact to flip function
    __attribute__((visibility("default"))) void HandleGCCallback();

    void GcPhaseEnum(GCPhase newPhase);
    void GCPhasePreForward(GCPhase newPhase);
    void HandleGCPhase(GCPhase newPhase);

    virtual void VisitMutatorRoots(const RefFieldVisitor &visitor);

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

    bool TryRunFlipFunction();

    void WaitFlipFunctionFinish();

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void PushFrameInfoForMarking(const GCInfoNode &frameGCInfo)
    {
        gcInfos_.PushFrameInfoForMarking(frameGCInfo);
    }

    void PushFrameInfoForMarking(const GCInfoNode &&frameGCInfo)
    {
        gcInfos_.PushFrameInfoForMarking(frameGCInfo);
    }

    void PushFrameInfoForFix(const GCInfoNodeForFix &frameGCInfo)
    {
        gcInfos_.PushFrameInfoForFix(frameGCInfo);
    }

    void PushFrameInfoForFix(const GCInfoNodeForFix &&frameGCInfo)
    {
        gcInfos_.PushFrameInfoForFix(frameGCInfo);
    }

    void DumpGCInfos() const
    {
        LOG(DEBUG, GC) << "dump mutator gc info thread id: " << tid;
        gcInfos_.DumpGCInfos();
    }
#endif

    NO_INLINE void RememberObjectInSatbBuffer(const BaseObject *obj)
    {
        RememberObjectImpl(obj);
    }

    const void *GetSatbBufferNode() const;

    void ClearSatbBufferNode();

    void PushRawObject(BaseObject *obj)
    {
        rawObject_.object = obj;
    }

    BaseObject *PopRawObject()
    {
        BaseObject *obj = rawObject_.object;
        rawObject_.object = nullptr;
        return obj;
    }

    __attribute__((always_inline)) inline bool IsInRunningState() const;

    // Thread must be binded mutator before to allocate. Otherwise it cannot allocate heap object in this thread.
    // One thread only allow to bind one muatator. If try bind sencond mutator, will be fatal.
    void BindMutator();
    // One thread only allow to bind one muatator. So it must be unbinded mutator before bind another one.
    void UnbindMutator();

    void *GetAllocBuffer() const
    {
        DCHECK(allocBuffer_ != nullptr);
        return allocBuffer_;
    }

    void ReleaseAllocBuffer();

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
    class TryBindMutatorScope {
    public:
        TryBindMutatorScope(Mutator *mutator);
        ~TryBindMutatorScope();

    private:
        Mutator *mutator_ {nullptr};
    };

protected:
    // for exception ref
    void VisitRawObjects(const RootVisitor &func);
    void CreateCurrentGCInfo();

private:
    void RememberObjectImpl(const BaseObject *obj);

    // Return false if thread has already binded mutator. Otherwise bind a mutator.
    bool TryBindMutator();

    // Indicate the current mutator phase and use which barrier in concurrent gc
    std::atomic<GCPhase> mutatorPhase_ = {GCPhase::GC_PHASE_UNDEF};

    // Indicate the state of mutator's phase transition
    std::atomic<GCPhaseTransitionState> transitionState_ = {NO_TRANSITION};

    // thread id
    uint32_t tid_ = 0;

    ObjectRef rawObject_ {nullptr};
    ark::os::memory::Mutex flipFunctionMtx_;
    ark::os::memory::ConditionVariable flipFunctionCV_;
    FlipFunction *flipFunction_ {nullptr};
    void *satbNode_ = nullptr;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    GCInfos gcInfos_;
#endif

    // Used for allocation fastpath, it is binded to thread local panda::AllocationBuffer.
    void *allocBuffer_ {nullptr};
};

ThreadLocalData *GetThreadLocalData();

}  // namespace ark::common_vm
#endif  // COMMON_RUNTIME_COMMON_INTERFACES_THREAD_MUTATOR_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           cppcoreguidelines-pro-type-union-access, modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)
