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

#ifndef PANDA_RUNTIME_MEM_GC_DYNAMIC_GC_MARKER_DYNAMIC_INL_H
#define PANDA_RUNTIME_MEM_GC_DYNAMIC_GC_MARKER_DYNAMIC_INL_H

#include "runtime/mem/gc/gc_marker.h"
#include "runtime/include/coretypes/dyn_objects.h"

namespace panda::mem {
template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC, false>::HandleObject(GCMarkingStackType *objects_stack,
                                                              const ObjectHeader *object, const BaseClass *base_cls)
{
    ASSERT(base_cls->IsDynamicClass());
    auto *cls = static_cast<const HClass *>(base_cls);
    // Handle dyn_class
    ObjectHeader *dyn_class = cls->GetManagedObject();
    if (AsMarker()->MarkIfNotMarked(dyn_class)) {
        objects_stack->PushToStack(object, dyn_class);
    }
    // mark object data
    uint32_t obj_body_size = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
    ASSERT(obj_body_size % TaggedValue::TaggedTypeSize() == 0);
    uint32_t num_of_fields = obj_body_size / TaggedValue::TaggedTypeSize();
    size_t start_addr = reinterpret_cast<uintptr_t>(object) + ObjectHeader::ObjectHeaderSize();
    for (uint32_t i = 0; i < num_of_fields; i++) {
        uint32_t field_offset = i * TaggedValue::TaggedTypeSize();
        if (cls->IsNativeField(ObjectHeader::ObjectHeaderSize() + field_offset)) {
            continue;
        }
        auto *field_addr = reinterpret_cast<std::atomic<TaggedType> *>(start_addr + field_offset);
        // Atomic with relaxed order reason: to correct read the value
        TaggedValue tagged_value(field_addr->load(std::memory_order_relaxed));
        if (!tagged_value.IsHeapObject()) {
            continue;
        }

        ObjectHeader *object_header = tagged_value.GetHeapObject();
        if (AsMarker()->MarkIfNotMarked(object_header)) {
            objects_stack->PushToStack(object, object_header);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC, false>::HandleClass(GCMarkingStackType *objects_stack,
                                                             const coretypes::DynClass *cls)
{
    // mark Hclass Data & Prototype
    HClass *klass = const_cast<coretypes::DynClass *>(cls)->GetHClass();
    // klass_size is sizeof DynClass include JSHClass, which is saved in root DynClass.
    size_t klass_size = cls->ClassAddr<HClass>()->GetObjectSize();

    uintptr_t start_addr = reinterpret_cast<uintptr_t>(klass) + sizeof(HClass);
    size_t body_size = klass_size - sizeof(coretypes::DynClass) - sizeof(HClass);
    size_t num_of_fields = body_size / TaggedValue::TaggedTypeSize();
    for (size_t i = 0; i < num_of_fields; i++) {
        auto *addr = reinterpret_cast<std::atomic<TaggedType> *>(start_addr + i * TaggedValue::TaggedTypeSize());
        // Atomic with relaxed order reason: to correct read the value
        coretypes::TaggedValue tagged_value(addr->load(std::memory_order_relaxed));
        if (!tagged_value.IsHeapObject()) {
            continue;
        }
        ObjectHeader *object_header = tagged_value.GetHeapObject();
        if (AsMarker()->MarkIfNotMarked(object_header)) {
            objects_stack->PushToStack(cls, object_header);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC, false>::HandleArrayClass(GCMarkingStackType *objects_stack,
                                                                  const coretypes::Array *array_object,
                                                                  [[maybe_unused]] const BaseClass *cls)
{
    LOG(DEBUG, GC) << "Dyn Array object: " << GetDebugInfoAboutObject(array_object);
    auto array_length = array_object->GetLength();
    ASSERT(cls->IsDynamicClass());
    for (coretypes::array_size_t i = 0; i < array_length; i++) {
        TaggedValue array_element(array_object->Get<TaggedType, true, true>(i));
        if (!array_element.IsHeapObject()) {
            continue;
        }
        ObjectHeader *element_object = array_element.GetHeapObject();
        if (AsMarker()->MarkIfNotMarked(element_object)) {
            objects_stack->PushToStack(array_object, element_object);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_DYNAMIC, false>::MarkInstance(GCMarkingStackType *objects_stack,
                                                              const ReferenceCheckPredicateT &ref_pred,
                                                              const ObjectHeader *object, const BaseClass *base_cls)
{
    ASSERT(base_cls->IsDynamicClass());
    if (GetGC()->IsReference(nullptr, object, ref_pred)) {
        GetGC()->ProcessReference(objects_stack, nullptr, object, GC::EmptyReferenceProcessPredicate);
        return;
    }
    auto *cls = static_cast<const HClass *>(base_cls);
    // push to stack after marked, so just return here.
    if (cls->IsNativePointer() || cls->IsString()) {
        return;
    }
    if (cls->IsHClass()) {
        auto dyn_class = static_cast<const panda::coretypes::DynClass *>(object);
        HandleClass(objects_stack, dyn_class);
    } else if (cls->IsArray()) {
        auto *array_object = static_cast<const panda::coretypes::Array *>(object);
        HandleArrayClass(objects_stack, array_object, cls);
    } else {
        HandleObject(objects_stack, object, cls);
    }
}
}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_DYNAMIC_GC_MARKER_DYNAMIC_INL_H
