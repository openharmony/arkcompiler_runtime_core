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
#include "runtime/mem/gc/gc_phase.h"

namespace ark::common_vm {
class Mutator;
struct ThreadLocalData;
enum class ThreadType;

bool IsRuntimeThread();
bool IsGcThread();

class ScopedGcThreadType {
public:
    ScopedGcThreadType();
    ~ScopedGcThreadType();

    NO_COPY_SEMANTIC(ScopedGcThreadType);
    NO_MOVE_SEMANTIC(ScopedGcThreadType);

private:
    ThreadType oldType_;
};

class Mutator;
using FlipFunction = std::function<void(Mutator &)>;
class PANDA_PUBLIC_API Mutator {
public:
    virtual ~Mutator();

    void ResetMutator();

    // temporary impl to clean GC callback, and need to refact to flip function
    __attribute__((visibility("default"))) void HandleGCCallback();

    void HandleGCPhase(mem::GCPhase newPhase);

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
