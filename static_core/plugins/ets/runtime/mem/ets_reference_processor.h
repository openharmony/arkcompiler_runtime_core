/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_PROCESSOR_H
#define PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_PROCESSOR_H

#include "runtime/mem/gc/reference-processor/reference_processor.h"

namespace panda::mem::ets {

class EtsReferenceProcessor final : public ReferenceProcessor {
public:
    explicit EtsReferenceProcessor(GC *gc);
    NO_COPY_SEMANTIC(EtsReferenceProcessor);
    NO_MOVE_SEMANTIC(EtsReferenceProcessor);
    ~EtsReferenceProcessor() final = default;

    bool IsReference(const BaseClass *base_cls, const ObjectHeader *ref,
                     const ReferenceCheckPredicateT &pred) const final;

    void HandleReference(GC *gc, GCMarkingStackType *objects_stack, const BaseClass *cls, const ObjectHeader *object,
                         const ReferenceProcessPredicateT &pred) final;

    void ProcessReferences(bool concurrent, bool clear_soft_references, GCPhase gc_phase,
                           const mem::GC::ReferenceClearPredicateT &pred) final;

    panda::mem::Reference *CollectClearedReferences() final
    {
        return nullptr;
    }

    void ScheduleForEnqueue([[maybe_unused]] Reference *cleared_references) final
    {
        UNREACHABLE();
    }

    void Enqueue([[maybe_unused]] panda::mem::Reference *cleared_references) final
    {
        UNREACHABLE();
    }

    /// @return size of the queue of weak references
    size_t GetReferenceQueueSize() const final;

private:
    mutable os::memory::Mutex weak_ref_lock_;
    PandaUnorderedSet<ObjectHeader *> weak_references_ GUARDED_BY(weak_ref_lock_);
    GC *gc_;
};

}  // namespace panda::mem::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_PROCESSOR_H
