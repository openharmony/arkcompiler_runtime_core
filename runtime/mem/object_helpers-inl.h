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

#ifndef PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H
#define PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H

#include "runtime/mem/object_helpers.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/hclass.h"

namespace panda::mem {

template <typename Handler>
void GCStaticObjectHelpers::TraverseClass(Class *cls, Handler &handler)
{
    // Iterate over static fields
    uint32_t ref_num = cls->GetRefFieldsNum<true>();
    if (ref_num > 0) {
        uint32_t offset = cls->GetRefFieldsOffset<true>();
        ObjectHeader *object = cls->GetManagedObject();
        ASSERT(ToUintPtr(cls) + offset >= ToUintPtr(object));
        // The offset is relative to the class. Adjust it to make relative to the managed object
        uint32_t obj_offset = ToUintPtr(cls) + offset - ToUintPtr(object);
        uint32_t ref_volatile_num = cls->GetVolatileRefFieldsNum<true>();
        for (uint32_t i = 0; i < ref_num;
             i++, offset += ClassHelper::OBJECT_POINTER_SIZE, obj_offset += ClassHelper::OBJECT_POINTER_SIZE) {
            bool is_volatile = (i < ref_volatile_num);
            auto *field_object = is_volatile ? cls->GetFieldObject<true>(offset) : cls->GetFieldObject<false>(offset);
            if (field_object == nullptr) {
                continue;
            }
            bool res = handler(object, field_object, obj_offset, is_volatile);
            if (!res) {
                return;
            }
        }
    }
}

template <typename Handler>
void GCStaticObjectHelpers::TraverseObject(ObjectHeader *object, Class *cls, Handler &handler)
{
    ASSERT(cls != nullptr && !cls->IsDynamicClass());
    while (cls != nullptr) {
        // Iterate over instance fields
        uint32_t ref_num = cls->GetRefFieldsNum<false>();
        if (ref_num == 0) {
            cls = cls->GetBase();
            continue;
        }

        uint32_t offset = cls->GetRefFieldsOffset<false>();
        uint32_t ref_volatile_num = cls->GetVolatileRefFieldsNum<false>();
        for (uint32_t i = 0; i < ref_num; i++, offset += ClassHelper::OBJECT_POINTER_SIZE) {
            bool is_volatile = (i < ref_volatile_num);
            auto *field_object =
                is_volatile ? object->GetFieldObject<true>(offset) : object->GetFieldObject<false>(offset);
            if (field_object == nullptr) {
                continue;
            }
            ValidateObject(object, field_object);
            bool res = handler(object, field_object, offset, is_volatile);
            if (!res) {
                return;
            }
        }

        cls = cls->GetBase();
    }
}

template <typename Handler>
void GCStaticObjectHelpers::TraverseArray(coretypes::Array *array, [[maybe_unused]] Class *cls, Handler &handler)
{
    ASSERT(!cls->IsDynamicClass());
    ASSERT(cls != nullptr);
    ASSERT(cls->IsObjectArrayClass());
    auto array_length = array->GetLength();
    for (coretypes::array_size_t i = 0; i < array_length; i++) {
        auto *array_element = array->Get<ObjectHeader *>(i);
        if (array_element != nullptr) {
            bool res = handler(array, array_element, array->GetElementOffset<false>(i), false);
            if (!res) {
                return;
            }
        }
    }
}

template <typename Handler>
void GCStaticObjectHelpers::TraverseAllObjectsWithInfo(ObjectHeader *object_header, Handler &handler)
{
    auto *cls = object_header->ClassAddr<Class>();
    ASSERT(cls != nullptr);

    if (cls->IsObjectArrayClass()) {
        TraverseArray(static_cast<coretypes::Array *>(object_header), cls, handler);
    } else {
        if (cls->IsClassClass()) {
            auto object_cls = panda::Class::FromClassObject(object_header);
            if (object_cls->IsInitializing() || object_cls->IsInitialized()) {
                TraverseClass(object_cls, handler);
            }
        }
        TraverseObject(object_header, cls, handler);
    }
}

template <typename Handler>
void GCDynamicObjectHelpers::TraverseClass(coretypes::DynClass *dyn_class, Handler &handler)
{
    size_t hklass_size = dyn_class->ClassAddr<HClass>()->GetObjectSize() - sizeof(coretypes::DynClass);
    size_t body_size = hklass_size - sizeof(HClass);
    size_t num_of_fields = body_size / TaggedValue::TaggedTypeSize();
    for (size_t i = 0; i < num_of_fields; i++) {
        size_t field_offset = sizeof(ObjectHeader) + sizeof(HClass) + i * TaggedValue::TaggedTypeSize();
        auto tagged_value = ObjectAccessor::GetDynValue<TaggedValue>(dyn_class, field_offset);
        if (tagged_value.IsHeapObject()) {
            bool res = handler(dyn_class, tagged_value.GetHeapObject(), field_offset, false);
            if (!res) {
                return;
            }
        }
    }
}

template <typename Handler>
void GCDynamicObjectHelpers::TraverseObject(ObjectHeader *object, HClass *cls, Handler &handler)
{
    ASSERT(cls->IsDynamicClass());
    LOG(DEBUG, GC) << "TraverseObject Current object: " << GetDebugInfoAboutObject(object);
    // handle object data
    uint32_t obj_body_size = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
    ASSERT(obj_body_size % TaggedValue::TaggedTypeSize() == 0);
    uint32_t num_of_fields = obj_body_size / TaggedValue::TaggedTypeSize();
    size_t data_offset = ObjectHeader::ObjectHeaderSize();
    for (uint32_t i = 0; i < num_of_fields; i++) {
        size_t field_offset = data_offset + i * TaggedValue::TaggedTypeSize();
        if (cls->IsNativeField(field_offset)) {
            continue;
        }
        auto tagged_value = ObjectAccessor::GetDynValue<TaggedValue>(object, field_offset);
        if (tagged_value.IsHeapObject()) {
            bool res = handler(object, tagged_value.GetHeapObject(), field_offset, false);
            if (!res) {
                return;
            }
        }
    }
}

template <typename Handler>
void GCDynamicObjectHelpers::TraverseArray(coretypes::Array *array, [[maybe_unused]] HClass *cls, Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(cls->IsDynamicClass());

    ASSERT(cls->IsArray());
    auto array_length = array->GetLength();
    for (coretypes::array_size_t i = 0; i < array_length; i++) {
        TaggedValue array_element(array->Get<TaggedType, false, true>(i));
        uint32_t offset = array->GetElementOffset<true>(i);
        if (array_element.IsHeapObject()) {
            bool res = handler(array, array_element.GetHeapObject(), offset, false);
            if (!res) {
                return;
            }
        }
    }
}

template <typename Handler>
void GCDynamicObjectHelpers::TraverseAllObjectsWithInfo(ObjectHeader *object_header, Handler &handler)
{
    auto *cls = object_header->ClassAddr<HClass>();
    ASSERT(cls != nullptr && cls->IsDynamicClass());
    if (cls->IsString() || cls->IsNativePointer()) {
        return;
    }
    if (cls->IsArray()) {
        TraverseArray(static_cast<coretypes::Array *>(object_header), cls, handler);
    } else {
        if (cls->IsHClass()) {
            TraverseClass(coretypes::DynClass::Cast(object_header), handler);
        } else {
            TraverseObject(object_header, cls, handler);
        }
    }
}

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H
