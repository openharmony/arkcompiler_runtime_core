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

#include "libpandabase/os/mutex.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/object_helpers.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"

namespace ark::mem::ets {

EtsReferenceProcessor::EtsReferenceProcessor(GC *gc) : gc_(gc) {}

bool EtsReferenceProcessor::IsReference(const BaseClass *baseCls, const ObjectHeader *ref,
                                        const ReferenceCheckPredicateT &pred) const
{
    ASSERT(baseCls != nullptr);
    ASSERT(ref != nullptr);
    ASSERT(baseCls->GetSourceLang() == panda_file::SourceLang::ETS);

    const auto *objEtsClass = ark::ets::EtsClass::FromRuntimeClass(static_cast<const Class *>(baseCls));

    if (objEtsClass->IsWeakReference()) {
        const auto *etsRef = reinterpret_cast<const ark::ets::EtsWeakReference *>(ref);

        auto *referent = etsRef->GetReferent();
        if (referent == nullptr) {
            LOG(DEBUG, REF_PROC) << "Treat " << GetDebugInfoAboutObject(ref)
                                 << " as normal object, because referent is null";
            return false;
        }

        ASSERT(IsMarking(gc_->GetGCPhase()));
        bool referentIsMarked = false;
        if (pred(referent->GetCoreType())) {
            referentIsMarked = gc_->IsMarked(referent->GetCoreType());
        } else {
            LOG(DEBUG, REF_PROC) << "Treat " << GetDebugInfoAboutObject(ref) << " as normal object, because referent "
                                 << std::hex << GetDebugInfoAboutObject(referent->GetCoreType())
                                 << " doesn't suit predicate";
            referentIsMarked = true;
        }
        return !referentIsMarked;
    }
    return false;
}

void EtsReferenceProcessor::HandleReference([[maybe_unused]] GC *gc, [[maybe_unused]] GCMarkingStackType *objectsStack,
                                            [[maybe_unused]] const BaseClass *cls, const ObjectHeader *object,
                                            [[maybe_unused]] const ReferenceProcessPredicateT &pred)
{
    os::memory::LockHolder lock(weakRefLock_);
    LOG(DEBUG, REF_PROC) << GetDebugInfoAboutObject(object) << " is added to weak references set for processing";
    weakReferences_.insert(const_cast<ObjectHeader *>(object));
}

void EtsReferenceProcessor::ProcessReferences([[maybe_unused]] bool concurrent,
                                              [[maybe_unused]] bool clearSoftReferences,
                                              [[maybe_unused]] GCPhase gcPhase,
                                              const mem::GC::ReferenceClearPredicateT &pred)
{
    os::memory::LockHolder lock(weakRefLock_);
    while (!weakReferences_.empty()) {
        auto *weakRefObj = weakReferences_.extract(weakReferences_.begin()).value();
        ASSERT(ark::ets::EtsClass::FromRuntimeClass(weakRefObj->ClassAddr<Class>())->IsWeakReference());
        auto *weakRef = static_cast<ark::ets::EtsWeakReference *>(ark::ets::EtsObject::FromCoreType(weakRefObj));
        auto *referent = weakRef->GetReferent();
        if (referent == nullptr) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weakRefObj)
                                 << " because referent is null";
            continue;
        }
        auto *referentObj = referent->GetCoreType();
        if (!pred(referentObj)) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weakRefObj)
                                 << " because referent " << GetDebugInfoAboutObject(referentObj) << " failed predicate";
            continue;
        }
        if (gc_->IsMarked(referentObj)) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weakRefObj)
                                 << " because referent " << GetDebugInfoAboutObject(referentObj) << " is marked";
            continue;
        }
        LOG(DEBUG, REF_PROC) << "In " << GetDebugInfoAboutObject(weakRefObj) << " clear referent";
        weakRef->ClearReferent();
    }
}

size_t EtsReferenceProcessor::GetReferenceQueueSize() const
{
    os::memory::LockHolder lock(weakRefLock_);
    return weakReferences_.size();
}

}  // namespace ark::mem::ets
