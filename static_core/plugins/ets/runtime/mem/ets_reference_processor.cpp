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

#include "libpandabase/os/mutex.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/object_helpers.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::mem::ets {

EtsReferenceProcessor::EtsReferenceProcessor(GC *gc) : gc_(gc) {}

void EtsReferenceProcessor::Initialize()
{
    auto *vm = ark::ets::PandaEtsVM::GetCurrent();
    ASSERT(vm != nullptr);
    auto *undefinedObjHeader = vm->GetUndefinedObject();
    undefinedObject_ = ark::ets::EtsObject::FromCoreType(undefinedObjHeader);
    ASSERT(gc_->GetObjectAllocator()->IsObjectInNonMovableSpace(undefinedObject_->GetCoreType()));
}

bool EtsReferenceProcessor::IsReference(const BaseClass *baseCls, const ObjectHeader *ref,
                                        const ReferenceCheckPredicateT &pred) const
{
    ASSERT(baseCls != nullptr);
    ASSERT(ref != nullptr);
    ASSERT(baseCls->GetSourceLang() == panda_file::SourceLang::ETS);

    const auto *objEtsClass = ark::ets::EtsClass::FromRuntimeClass(static_cast<const Class *>(baseCls));

    if (!objEtsClass->IsReference()) {
        return false;
    }
    ASSERT(objEtsClass->IsWeakReference() || objEtsClass->IsFinalizerReference());

    const auto *etsRef = reinterpret_cast<const ark::ets::EtsWeakReference *>(ref);

    auto *referent = etsRef->GetReferent();
    if (referent == nullptr || referent == undefinedObject_) {
        LOG(DEBUG, REF_PROC) << "Treat " << GetDebugInfoAboutObject(ref)
                             << " as normal object, because referent is nullish";
        return false;
    }

    if (!pred(referent->GetCoreType())) {
        LOG(DEBUG, REF_PROC) << "Treat " << GetDebugInfoAboutObject(ref) << " as normal object, because referent "
                             << std::hex << GetDebugInfoAboutObject(referent->GetCoreType())
                             << " doesn't suit predicate";
        return false;
    }

    return !gc_->IsMarkedEx(referent->GetCoreType());
}

void EtsReferenceProcessor::HandleReference(GC *gc, GCMarkingStackType *objectsStack, const BaseClass *cls,
                                            const ObjectHeader *object,
                                            [[maybe_unused]] const ReferenceProcessPredicateT &pred)
{
    os::memory::LockHolder lock(weakRefLock_);
    LOG(DEBUG, REF_PROC) << GetDebugInfoAboutObject(object) << " is added to weak references set for processing";
    weakReferences_.insert(const_cast<ObjectHeader *>(object));
    HandleOtherFields<false>(cls, object, [gc, objectsStack, object](void *reference) {
        auto *refObject = reinterpret_cast<ObjectHeader *>(reference);
        if (gc->MarkObjectIfNotMarked(refObject)) {
            objectsStack->PushToStack(object, refObject);
        }
    });
}

void EtsReferenceProcessor::HandleReference([[maybe_unused]] GC *gc, const BaseClass *cls, const ObjectHeader *object,
                                            const ReferenceProcessorT &processor)
{
    os::memory::LockHolder lock(weakRefLock_);
    LOG(DEBUG, REF_PROC) << GetDebugInfoAboutObject(object) << " is added to weak references set for processing";
    weakReferences_.insert(const_cast<ObjectHeader *>(object));
    HandleOtherFields<true>(cls, object, processor);
}

template <bool USE_OBJECT_REF>
void EtsReferenceProcessor::HandleOtherFields(const BaseClass *cls, const ObjectHeader *object,
                                              const ReferenceProcessorT &processor)
{
    auto *etsClass = ark::ets::EtsClass::FromRuntimeClass(static_cast<const Class *>(cls));
    if (!etsClass->IsFinalizerReference()) {
        return;
    }
    // Currently, only finalizer references' other fields are handled
    ASSERT(etsClass->IsWeakReference());
    auto *finalizableWeakRef = ark::ets::EtsFinalizableWeakRef::FromCoreType(object);
    if constexpr (USE_OBJECT_REF) {
        auto refHandler = [processor, finalizableWeakRef](auto *ref, size_t offset) {
            if (ref == nullptr) {
                return;
            }
            ASSERT(ref->GetReferent() != nullptr);
            processor(ToVoidPtr(ToUintPtr(finalizableWeakRef) + offset));
        };
        refHandler(finalizableWeakRef->GetPrev(), ark::ets::EtsFinalizableWeakRef::GetPrevOffset());
        refHandler(finalizableWeakRef->GetNext(), ark::ets::EtsFinalizableWeakRef::GetNextOffset());
    } else {
        auto refHandler = [processor](auto *ref) {
            if (ref == nullptr) {
                return;
            }
            ASSERT(ref->GetReferent() != nullptr);
            processor(ref->GetCoreType());
        };
        refHandler(finalizableWeakRef->GetPrev());
        refHandler(finalizableWeakRef->GetNext());
    }
}

void EtsReferenceProcessor::ProcessReferences([[maybe_unused]] bool concurrent,
                                              [[maybe_unused]] bool clearSoftReferences,
                                              [[maybe_unused]] GCPhase gcPhase,
                                              const mem::GC::ReferenceClearPredicateT &pred)
{
    ProcessReferences(pred, [this](auto *weakRefObj, auto *referentObj) {
        if (gc_->IsMarked(referentObj)) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weakRefObj)
                                 << " because referent " << GetDebugInfoAboutObject(referentObj) << " is marked";
            return;
        }
        LOG(DEBUG, REF_PROC) << "In " << GetDebugInfoAboutObject(weakRefObj) << " clear referent";
        auto *weakRef = static_cast<ark::ets::EtsWeakReference *>(ark::ets::EtsObject::FromCoreType(weakRefObj));
        weakRef->ClearReferent();
        EnqueueFinalizer(weakRef);
    });
}

void EtsReferenceProcessor::ProcessReferencesAfterCompaction(const mem::GC::ReferenceClearPredicateT &pred)
{
    ProcessReferences(pred, [this](auto *weakRefObj, auto *referentObj) {
        auto *weakRef = static_cast<ark::ets::EtsWeakReference *>(ark::ets::EtsObject::FromCoreType(weakRefObj));
        auto markword = referentObj->AtomicGetMark(std::memory_order_relaxed);
        auto forwarded = markword.GetState() == MarkWord::ObjectState::STATE_GC;
        if (forwarded) {
            LOG(DEBUG, REF_PROC) << "Update " << GetDebugInfoAboutObject(weakRefObj) << " referent "
                                 << GetDebugInfoAboutObject(referentObj) << " with forward address";
            auto *newAddr = reinterpret_cast<ObjectHeader *>(markword.GetForwardingAddress());
            weakRef->SetReferent(ark::ets::EtsObject::FromCoreType(newAddr));
            return;
        }
        LOG(DEBUG, REF_PROC) << "In " << GetDebugInfoAboutObject(weakRefObj) << " clear referent";
        weakRef->ClearReferent();
        EnqueueFinalizer(weakRef);
    });
}

size_t EtsReferenceProcessor::GetReferenceQueueSize() const
{
    os::memory::LockHolder lock(weakRefLock_);
    return weakReferences_.size();
}

void EtsReferenceProcessor::ProcessFinalizers()
{
    auto finalizerIt = finalizerQueue_.begin();
    while (finalizerIt != finalizerQueue_.end()) {
        finalizerIt->Run();
        finalizerIt = finalizerQueue_.erase(finalizerIt);
    }
}

void EtsReferenceProcessor::EnqueueFinalizer(ark::ets::EtsWeakReference *weakRef)
{
    auto *weakRefClass = weakRef->GetClass();
    if (weakRefClass->IsFinalizerReference()) {
        auto *finalizableWeakRef = ark::ets::EtsFinalizableWeakRef::FromEtsObject(weakRef);
        auto finalizer = finalizableWeakRef->ReleaseFinalizer();
        if (!finalizer.IsEmpty()) {
            finalizerQueue_.push_back(finalizer);
        }
    }
}

}  // namespace ark::mem::ets
