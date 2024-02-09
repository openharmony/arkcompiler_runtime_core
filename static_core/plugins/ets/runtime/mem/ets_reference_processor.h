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

namespace ark::mem::ets {

class EtsReferenceProcessor final : public ReferenceProcessor {
public:
    explicit EtsReferenceProcessor(GC *gc);
    NO_COPY_SEMANTIC(EtsReferenceProcessor);
    NO_MOVE_SEMANTIC(EtsReferenceProcessor);
    ~EtsReferenceProcessor() final = default;

    bool IsReference(const BaseClass *baseCls, const ObjectHeader *ref,
                     const ReferenceCheckPredicateT &pred) const final;

    void HandleReference(GC *gc, GCMarkingStackType *objectsStack, const BaseClass *cls, const ObjectHeader *object,
                         const ReferenceProcessPredicateT &pred) final;

    void ProcessReferences(bool concurrent, bool clearSoftReferences, GCPhase gcPhase,
                           const mem::GC::ReferenceClearPredicateT &pred) final;

    ark::mem::Reference *CollectClearedReferences() final
    {
        return nullptr;
    }

    void ScheduleForEnqueue([[maybe_unused]] Reference *clearedReferences) final
    {
        UNREACHABLE();
    }

    void Enqueue([[maybe_unused]] ark::mem::Reference *clearedReferences) final
    {
        UNREACHABLE();
    }

    /// @return size of the queue of weak references
    size_t GetReferenceQueueSize() const final;

private:
    mutable os::memory::Mutex weakRefLock_;
    PandaUnorderedSet<ObjectHeader *> weakReferences_ GUARDED_BY(weakRefLock_);
    GC *gc_;
};

}  // namespace ark::mem::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_PROCESSOR_H
