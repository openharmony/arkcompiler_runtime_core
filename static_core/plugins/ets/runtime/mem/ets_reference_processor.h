/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"

#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref.h"

namespace ark::mem::ets {

class EtsReferenceProcessor final : public ReferenceProcessor {
    using RefFinalizer = ark::ets::EtsFinalizableWeakRef::Finalizer;

public:
    explicit EtsReferenceProcessor(GC *gc);
    NO_COPY_SEMANTIC(EtsReferenceProcessor);
    NO_MOVE_SEMANTIC(EtsReferenceProcessor);
    ~EtsReferenceProcessor() final = default;

    void Initialize() override;

    bool IsReference(const BaseClass *baseCls, const ObjectHeader *ref,
                     const ReferenceCheckPredicateT &pred) const final;

    void HandleReference(GC *gc, GCMarkingStackType *objectsStack, const BaseClass *cls, const ObjectHeader *object,
                         const ReferenceProcessPredicateT &pred) final;
    void HandleReference(GC *gc, const BaseClass *cls, const ObjectHeader *object,
                         const ReferenceProcessorT &processor) final;

    void ProcessReferences(bool concurrent, bool clearSoftReferences, GCPhase gcPhase,
                           const mem::GC::ReferenceClearPredicateT &pred) final;
    void ProcessReferencesAfterCompaction([[maybe_unused]] const mem::GC::ReferenceClearPredicateT &pred) final;

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

    /// Process native finalizers of FinalizableWeakRef
    void ProcessFinalizers() final;

private:
    template <typename Handler>
    void ProcessReferences(const mem::GC::ReferenceClearPredicateT &pred, const Handler &handler);

    /// Add finalizer of dead referent of FinalizableWeakRef to the queue
    void EnqueueFinalizer(ark::ets::EtsWeakReference *weakRef);

    /// Handle fields of references
    template <bool USE_OBJECT_REF>
    void HandleOtherFields(const BaseClass *cls, const ObjectHeader *object, const ReferenceProcessorT &processor)
        REQUIRES(weakRefLock_);

    mutable os::memory::Mutex weakRefLock_;
    PandaUnorderedSet<ObjectHeader *> weakReferences_ GUARDED_BY(weakRefLock_);
    GC *gc_ {nullptr};
    ark::ets::EtsObject *undefinedObject_ {nullptr};
    PandaDeque<RefFinalizer> finalizerQueue_;
};

template <typename Handler>
void EtsReferenceProcessor::ProcessReferences(const mem::GC::ReferenceClearPredicateT &pred, const Handler &handler)
{
    os::memory::LockHolder lock(weakRefLock_);
    while (!weakReferences_.empty()) {
        auto *weakRefObj = weakReferences_.extract(weakReferences_.begin()).value();
        ASSERT(ark::ets::EtsClass::FromRuntimeClass(weakRefObj->ClassAddr<Class>())->IsWeakReference());
        auto *weakRef = static_cast<ark::ets::EtsWeakReference *>(ark::ets::EtsObject::FromCoreType(weakRefObj));
        auto *referent = weakRef->GetReferent();
        if (referent == nullptr || referent == undefinedObject_) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weakRefObj)
                                 << " because referent is nullish";
            continue;
        }
        auto *referentObj = referent->GetCoreType();
        if (!pred(referentObj)) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weakRefObj)
                                 << " because referent " << GetDebugInfoAboutObject(referentObj) << " failed predicate";
            continue;
        }

        handler(weakRefObj, referentObj);
    }
}
}  // namespace ark::mem::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_PROCESSOR_H
