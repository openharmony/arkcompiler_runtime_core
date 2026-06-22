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

bool IsRuntimeThread();
bool IsGcThread();

class Mutator;
using FlipFunction = std::function<void(Mutator &)>;
class PANDA_PUBLIC_API Mutator {
public:
    virtual ~Mutator();

    void ResetMutator();

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

    // temporary impl to clean GC callback, and need to refact to flip function
    __attribute__((visibility("default"))) void HandleGCCallback();

    void GcPhaseEnum(GCPhase newPhase);
    void GCPhasePreForward(GCPhase newPhase);
    void HandleGCPhase(GCPhase newPhase);

    virtual void VisitMutatorRoots(const RefFieldVisitor &visitor);

    NO_INLINE void RememberObjectInSatbBuffer(const BaseObject *obj)
    {
        RememberObjectImpl(obj);
    }

    const void *GetSatbBufferNode() const;

    void ClearSatbBufferNode();

    void *GetAllocBuffer() const
    {
        DCHECK(allocBuffer_ != nullptr);
        return allocBuffer_;
    }

    void ReleaseAllocBuffer();

private:
    void RememberObjectImpl(const BaseObject *obj);

    // Indicate the current mutator phase and use which barrier in concurrent gc
    std::atomic<GCPhase> mutatorPhase_ = {GCPhase::GC_PHASE_UNDEF};

    void *satbNode_ = nullptr;

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
