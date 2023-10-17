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

namespace panda::mem::ets {

EtsReferenceProcessor::EtsReferenceProcessor(GC *gc) : gc_(gc) {}

bool EtsReferenceProcessor::IsReference(const BaseClass *base_cls, const ObjectHeader *ref,
                                        const ReferenceCheckPredicateT &pred) const
{
    ASSERT(base_cls != nullptr);
    ASSERT(ref != nullptr);
    ASSERT(base_cls->GetSourceLang() == panda_file::SourceLang::ETS);

    const auto *obj_ets_class = panda::ets::EtsClass::FromRuntimeClass(static_cast<const Class *>(base_cls));

    if (obj_ets_class->IsWeakReference()) {
        const auto *ets_ref = reinterpret_cast<const panda::ets::EtsWeakReference *>(ref);

        auto *referent = ets_ref->GetReferent();
        if (referent == nullptr) {
            LOG(DEBUG, REF_PROC) << "Treat " << GetDebugInfoAboutObject(ref)
                                 << " as normal object, because referent is null";
            return false;
        }

        ASSERT(IsMarking(gc_->GetGCPhase()));
        bool referent_is_marked = false;
        if (pred(referent->GetCoreType())) {
            referent_is_marked = gc_->IsMarked(referent->GetCoreType());
        } else {
            LOG(DEBUG, REF_PROC) << "Treat " << GetDebugInfoAboutObject(ref) << " as normal object, because referent "
                                 << std::hex << GetDebugInfoAboutObject(referent->GetCoreType())
                                 << " doesn't suit predicate";
            referent_is_marked = true;
        }
        return !referent_is_marked;
    }
    return false;
}

void EtsReferenceProcessor::HandleReference([[maybe_unused]] GC *gc, [[maybe_unused]] GCMarkingStackType *objects_stack,
                                            [[maybe_unused]] const BaseClass *cls, const ObjectHeader *object,
                                            [[maybe_unused]] const ReferenceProcessPredicateT &pred)
{
    os::memory::LockHolder lock(weak_ref_lock_);
    LOG(DEBUG, REF_PROC) << GetDebugInfoAboutObject(object) << " is added to weak references set for processing";
    weak_references_.insert(const_cast<ObjectHeader *>(object));
}

void EtsReferenceProcessor::ProcessReferences([[maybe_unused]] bool concurrent,
                                              [[maybe_unused]] bool clear_soft_references,
                                              [[maybe_unused]] GCPhase gc_phase,
                                              const mem::GC::ReferenceClearPredicateT &pred)
{
    os::memory::LockHolder lock(weak_ref_lock_);
    while (!weak_references_.empty()) {
        auto *weak_ref_obj = weak_references_.extract(weak_references_.begin()).value();
        ASSERT(panda::ets::EtsClass::FromRuntimeClass(weak_ref_obj->ClassAddr<Class>())->IsWeakReference());
        auto *weak_ref = static_cast<panda::ets::EtsWeakReference *>(panda::ets::EtsObject::FromCoreType(weak_ref_obj));
        auto *referent = weak_ref->GetReferent();
        if (referent == nullptr) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weak_ref_obj)
                                 << " because referent is null";
            continue;
        }
        auto *referent_obj = referent->GetCoreType();
        if (!pred(referent_obj)) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weak_ref_obj)
                                 << " because referent " << GetDebugInfoAboutObject(referent_obj)
                                 << " failed predicate";
            continue;
        }
        if (gc_->IsMarked(referent_obj)) {
            LOG(DEBUG, REF_PROC) << "Don't process reference " << GetDebugInfoAboutObject(weak_ref_obj)
                                 << " because referent " << GetDebugInfoAboutObject(referent_obj) << " is marked";
            continue;
        }
        LOG(DEBUG, REF_PROC) << "In " << GetDebugInfoAboutObject(weak_ref_obj) << " clear referent";
        weak_ref->ClearReferent();
    }
}

size_t EtsReferenceProcessor::GetReferenceQueueSize() const
{
    os::memory::LockHolder lock(weak_ref_lock_);
    return weak_references_.size();
}

}  // namespace panda::mem::ets
