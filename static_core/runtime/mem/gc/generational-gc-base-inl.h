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

#include <type_traits>
#include "runtime/mem/gc/generational-gc-base.h"

namespace panda::mem {

template <class LanguageConfig>
template <typename Marker, class... ReferenceCheckPredicate>
void GenerationalGC<LanguageConfig>::MarkStack(Marker *marker, GCMarkingStackType *stack,
                                               const GC::MarkPreprocess &mark_preprocess,
                                               const ReferenceCheckPredicate &...ref_pred)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    // Check that we use correct type for ref_predicate (or not use it at all)
    static_assert(sizeof...(ReferenceCheckPredicate) == 0 ||
                  (sizeof...(ReferenceCheckPredicate) == 1 &&
                   std::is_constructible_v<ReferenceCheckPredicateT, ReferenceCheckPredicate...>));
    ASSERT(stack != nullptr);
    while (!stack->Empty()) {
        auto *object = this->PopObjectFromStack(stack);
        ASSERT(marker->IsMarked(object));
        ValidateObject(nullptr, object);
        auto *object_class = object->template ClassAddr<BaseClass>();
        // We need annotation here for the FullMemoryBarrier used in InitializeClassByIdEntrypoint
        TSAN_ANNOTATE_HAPPENS_AFTER(object_class);
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);

        ASSERT(!object->IsForwarded());
        mark_preprocess(object, object_class);
        static_cast<Marker *>(marker)->MarkInstance(stack, object, object_class, ref_pred...);
    }
}

template <class LanguageConfig>
template <typename Marker>
NO_THREAD_SAFETY_ANALYSIS void GenerationalGC<LanguageConfig>::MarkImpl(Marker *marker,
                                                                        GCMarkingStackType *objects_stack,
                                                                        CardTableVisitFlag visit_card_table_roots,
                                                                        const ReferenceCheckPredicateT &ref_pred,
                                                                        const MemRangeChecker &mem_range_checker,
                                                                        const GC::MarkPreprocess &mark_preprocess)
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
    MarkStack(marker, objects_stack, mark_preprocess, ref_pred);
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
    MarkStack(marker, objects_stack, mark_preprocess, ref_pred);

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
                if (marker->MarkIfNotMarked(gc_root.GetObjectHeader())) {
                    objects_stack->PushToStack(gc_root.GetType(), gc_root.GetObjectHeader());
                }
            }
        };

        auto allocator = this->GetObjectAllocator();
        MemRangeChecker range_checker = [&mem_range_checker](MemRange &mem_range) -> bool {
            return mem_range_checker(mem_range);
        };
        ObjectChecker tenured_object_checker = [&allocator](const ObjectHeader *object_header) -> bool {
            return !allocator->IsObjectInYoungSpace(object_header);
        };
        ObjectChecker from_object_checker = [marker](const ObjectHeader *object_header) -> bool {
            return marker->IsMarked(object_header);
        };
        this->VisitCardTableRoots(this->GetCardTable(), gc_mark_roots, range_checker, tenured_object_checker,
                                  from_object_checker,
                                  CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED |
                                      CardTableProcessedFlag::SET_PROCESSED);
        MarkStack(marker, objects_stack, mark_preprocess, ref_pred);
    }
}

}  // namespace panda::mem
#endif  // RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H
