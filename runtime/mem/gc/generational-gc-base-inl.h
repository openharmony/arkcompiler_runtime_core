/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H
#define RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H

#include "runtime/mem/gc/generational-gc-base.h"

namespace panda::mem {

template <class LanguageConfig>
template <typename Marker>
void GenerationalGC<LanguageConfig>::MarkStack(Marker *marker, GCMarkingStackType *stack,
                                               const ReferenceCheckPredicateT &ref_pred,
                                               const GC::MarkPredicate &markPredicate)
{
    auto pred = [stack]() { return !stack->Empty(); };
    MarkStackCond(marker, stack, pred, ref_pred, markPredicate);
}

template <class LanguageConfig>
template <typename Marker>
void GenerationalGC<LanguageConfig>::MarkStackCond(Marker *marker, GCMarkingStackType *stack,
                                                   const ConcurrentMarkPredicateT &pred,
                                                   const ReferenceCheckPredicateT &ref_pred,
                                                   const GC::MarkPredicate &markPredicate)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    ASSERT(stack != nullptr);
    while (pred()) {
        auto *object = this->PopObjectFromStack(stack);
        ValidateObject(nullptr, object);
        auto *object_class = object->template ClassAddr<BaseClass>();
        // We need annotation here for the FullMemoryBarrier used in InitializeClassByIdEntrypoint
        TSAN_ANNOTATE_HAPPENS_AFTER(object_class);
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);

        ASSERT(!object->IsForwarded());
        if (markPredicate(object)) {
            static_cast<Marker *>(marker)->MarkInstance(stack, ref_pred, object, object_class);
        }
    }
}

template <class LanguageConfig>
template <typename Marker>
NO_THREAD_SAFETY_ANALYSIS void GenerationalGC<LanguageConfig>::ConcurrentMark(Marker *marker,
                                                                              GCMarkingStackType *objects_stack,
                                                                              CardTableVisitFlag visit_card_table_roots,
                                                                              const ConcurrentMarkPredicateT &pred,
                                                                              const ReferenceCheckPredicateT &ref_pred,
                                                                              const MemRangeChecker &mem_range_checker)
{
    GCScope<TRACE_TIMING_PHASE> scoped_func(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    ConcurrentScope concurrent_scope(this);
    MarkImpl(marker, objects_stack, visit_card_table_roots, pred, ref_pred, mem_range_checker);
}

template <class LanguageConfig>
template <typename Marker>
NO_THREAD_SAFETY_ANALYSIS void GenerationalGC<LanguageConfig>::OnPauseMark(Marker *marker,
                                                                           GCMarkingStackType *objects_stack,
                                                                           CardTableVisitFlag visit_card_table_roots,
                                                                           const ConcurrentMarkPredicateT &pred,
                                                                           const ReferenceCheckPredicateT &ref_pred,
                                                                           const MemRangeChecker &mem_range_checker)
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    MarkImpl(marker, objects_stack, visit_card_table_roots, pred, ref_pred, mem_range_checker);
}

template <class LanguageConfig>
template <typename Marker>
NO_THREAD_SAFETY_ANALYSIS void GenerationalGC<LanguageConfig>::MarkImpl(Marker *marker,
                                                                        GCMarkingStackType *objects_stack,
                                                                        CardTableVisitFlag visit_card_table_roots,
                                                                        const ConcurrentMarkPredicateT &pred,
                                                                        const ReferenceCheckPredicateT &ref_pred,
                                                                        const MemRangeChecker &mem_range_checker)
{
    // concurrent visit class roots
    this->VisitClassRoots([this, marker, objects_stack](const GCRoot &gc_root) {
        if (marker->MarkIfNotMarked(gc_root.GetObjectHeader())) {
            ASSERT(gc_root.GetObjectHeader() != nullptr);
            objects_stack->PushToStack(RootType::ROOT_CLASS, gc_root.GetObjectHeader());
        } else {
            LOG_DEBUG_GC << "Skip root: " << gc_root.GetObjectHeader();
        }
    });
    MarkStackCond(marker, objects_stack, pred, ref_pred, GC::EmptyReferenceProcessPredicate);
    {
        ScopedTiming t1("VisitInternalStringTable", *this->GetTiming());
        this->GetPandaVm()->VisitStringTable(
            [marker, objects_stack](ObjectHeader *str) {
                if (marker->MarkIfNotMarked(str)) {
                    ASSERT(str != nullptr);
                    objects_stack->PushToStack(RootType::STRING_TABLE, str);
                }
            },
            VisitGCRootFlags::ACCESS_ROOT_ALL | VisitGCRootFlags::START_RECORDING_NEW_ROOT);
    }
    MarkStackCond(marker, objects_stack, pred, ref_pred, GC::EmptyReferenceProcessPredicate);

    // concurrent visit card table
    if (visit_card_table_roots == CardTableVisitFlag::VISIT_ENABLED) {
        GCRootVisitor gc_mark_roots = [this, marker, objects_stack, &ref_pred](const GCRoot &gc_root) {
            ObjectHeader *from_object = gc_root.GetFromObjectHeader();
            if (UNLIKELY(from_object != nullptr) &&
                this->IsReference(from_object->ClassAddr<BaseClass>(), from_object, ref_pred)) {
                LOG_DEBUG_GC << "Add reference: " << GetDebugInfoAboutObject(from_object) << " to stack";
                marker->Mark(from_object);
                this->ProcessReference(objects_stack, from_object->ClassAddr<BaseClass>(), from_object,
                                       GC::EmptyReferenceProcessPredicate);
            } else {
                objects_stack->PushToStack(gc_root.GetType(), gc_root.GetObjectHeader());
                marker->Mark(gc_root.GetObjectHeader());
            }
        };

        auto allocator = this->GetObjectAllocator();
        MemRangeChecker range_checker = [&mem_range_checker](MemRange &mem_range) -> bool {
            return mem_range_checker(mem_range);
        };
        ObjectChecker tenured_object_checker = [&allocator](const ObjectHeader *object_header) -> bool {
            return !allocator->IsAddressInYoungSpace(ToUintPtr(object_header));
        };
        ObjectChecker from_object_checker = [marker](const ObjectHeader *object_header) -> bool {
            return marker->IsMarked(object_header);
        };
        this->VisitCardTableRoots(this->GetCardTable(), gc_mark_roots, range_checker, tenured_object_checker,
                                  from_object_checker,
                                  CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED |
                                      CardTableProcessedFlag::SET_PROCESSED);
        MarkStackCond(marker, objects_stack, pred, ref_pred, GC::EmptyReferenceProcessPredicate);
    }
}

}  // namespace panda::mem
#endif  // RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H
