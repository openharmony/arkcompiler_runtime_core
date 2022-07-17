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

#ifndef PANDA_RUNTIME_MEM_GC_STATIC_GC_MARKER_STATIC_INL_H
#define PANDA_RUNTIME_MEM_GC_STATIC_GC_MARKER_STATIC_INL_H

#include "runtime/mem/gc/gc_marker.h"

namespace panda::mem {

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC, false>::HandleObject(GCMarkingStackType *objects_stack,
                                                             const ObjectHeader *object, const Class *cls)
{
    while (cls != nullptr) {
        // Iterate over instance fields
        uint32_t ref_num = cls->GetRefFieldsNum<false>();
        if (ref_num > 0) {
            uint32_t offset = cls->GetRefFieldsOffset<false>();
            uint32_t ref_volatile_num = cls->GetVolatileRefFieldsNum<false>();
            for (uint32_t i = 0; i < ref_num; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
                auto *field_object = (i < ref_volatile_num) ? object->GetFieldObject<true>(offset)
                                                            : object->GetFieldObject<false>(offset);
                ValidateObject(object, field_object);
                if (field_object != nullptr && AsMarker()->MarkIfNotMarked(field_object)) {
                    objects_stack->PushToStack(object, field_object);
                }
            }
        }
        cls = cls->GetBase();
    }
}
template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC, false>::HandleClass(GCMarkingStackType *objects_stack, const Class *cls)
{
    // Iterate over static fields
    uint32_t ref_num = cls->GetRefFieldsNum<true>();
    if (ref_num > 0) {
        uint32_t offset = cls->GetRefFieldsOffset<true>();
        uint32_t ref_volatile_num = cls->GetVolatileRefFieldsNum<true>();
        for (uint32_t i = 0; i < ref_num; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            auto *field_object =
                (i < ref_volatile_num) ? cls->GetFieldObject<true>(offset) : cls->GetFieldObject<false>(offset);
            if (field_object != nullptr && AsMarker()->MarkIfNotMarked(field_object)) {
                objects_stack->PushToStack(cls->GetManagedObject(), field_object);
            }
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC, false>::HandleArrayClass(GCMarkingStackType *objects_stack,
                                                                 const coretypes::Array *array_object,
                                                                 [[maybe_unused]] const Class *cls)
{
    LOG(DEBUG, GC) << "Array object: " << GetDebugInfoAboutObject(array_object);
    auto array_length = array_object->GetLength();

    ASSERT(cls->IsObjectArrayClass());

    LOG(DEBUG, GC) << "Iterate over: " << array_length << " elements in array";
    for (coretypes::array_size_t i = 0; i < array_length; i++) {
        auto *array_element = array_object->Get<ObjectHeader *>(i);
        if (array_element == nullptr) {
            continue;
        }
#ifndef NDEBUG
        auto array_element_cls = array_element->ClassAddr<Class>();
        LOG_IF(array_element_cls == nullptr, DEBUG, GC)
            << " object's class is nullptr: " << array_element << " from array: " << array_object;
        ASSERT(array_element_cls != nullptr);
#endif
        if (AsMarker()->MarkIfNotMarked(array_element)) {
            objects_stack->PushToStack(array_object, array_element);
        }
    }
}

template <typename Marker>
void GCMarker<Marker, LANG_TYPE_STATIC, false>::MarkInstance(GCMarkingStackType *objects_stack,
                                                             const ReferenceCheckPredicateT &ref_pred,
                                                             const ObjectHeader *object, const BaseClass *base_cls)
{
    ASSERT(!base_cls->IsDynamicClass());
    auto cls = static_cast<const Class *>(base_cls);
    if (GetGC()->IsReference(cls, object, ref_pred)) {
        GetGC()->ProcessReference(objects_stack, cls, object, GC::EmptyReferenceProcessPredicate);
    } else if (cls->IsObjectArrayClass()) {
        auto *array_object = static_cast<const panda::coretypes::Array *>(object);
        HandleArrayClass(objects_stack, array_object, cls);
    } else if (cls->IsClassClass()) {
        // Handle Class handles static fields only, so we need to Handle regular fields explicitly too
        auto object_cls = panda::Class::FromClassObject(object);
        if (object_cls->IsInitializing() || object_cls->IsInitialized()) {
            HandleClass(objects_stack, object_cls);
        }
        HandleObject(objects_stack, object, cls);
    } else if (cls->IsInstantiable()) {
        HandleObject(objects_stack, object, cls);
    } else {
        if (!cls->IsPrimitive()) {
            LOG(FATAL, GC) << "Wrong handling, missed type: " << cls->GetDescriptor();
        }
    }
}

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_STATIC_GC_MARKER_STATIC_INL_H
