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

bool GCStaticObjectHelpers::IsClassObject(ObjectHeader *obj)
{
    return obj->ClassAddr<Class>()->IsClassClass();
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseClass(Class *cls, Handler &handler)
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
            [[maybe_unused]] bool res = handler(object, field_object, obj_offset, is_volatile);
            if constexpr (INTERRUPTIBLE) {
                if (!res) {
                    return false;
                }
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseObject(ObjectHeader *object, Class *cls, Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());
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
            [[maybe_unused]] bool res = handler(object, field_object, offset, is_volatile);
            if constexpr (INTERRUPTIBLE) {
                if (!res) {
                    return false;
                }
            }
        }

        cls = cls->GetBase();
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseArray(coretypes::Array *array, [[maybe_unused]] Class *cls, void *begin, void *end,
                                          Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());
    ASSERT(cls->IsObjectArrayClass());
    ASSERT(IsAligned(ToUintPtr(begin), DEFAULT_ALIGNMENT_IN_BYTES));

    auto *array_start = array->GetBase<ObjectPointerType *>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *array_end = array_start + array->GetLength();
    auto *p = begin < array_start ? array_start : reinterpret_cast<ObjectPointerType *>(begin);

    if (end > array_end) {
        end = array_end;
    }

    auto element_size = coretypes::Array::GetElementSize<ObjectHeader *, false>();
    auto offset = ToUintPtr(p) - ToUintPtr(array);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (ArraySizeT i = p - array_start; p < end; ++p, ++i, offset += element_size) {
        auto *array_element = array->Get<ObjectHeader *>(i);
        if (array_element != nullptr) {
            [[maybe_unused]] bool res = handler(array, array_element, offset, false);
            if constexpr (INTERRUPTIBLE) {
                if (!res) {
                    return false;
                }
            }
        }
    }

    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCStaticObjectHelpers::TraverseAllObjectsWithInfo(ObjectHeader *object_header, Handler &handler, void *begin,
                                                       void *end)
{
    auto *cls = object_header->ClassAddr<Class>();
    ASSERT(cls != nullptr);

    if (cls->IsObjectArrayClass()) {
        return TraverseArray<INTERRUPTIBLE>(static_cast<coretypes::Array *>(object_header), cls, begin, end, handler);
    }
    if (cls->IsClassClass()) {
        auto object_cls = panda::Class::FromClassObject(object_header);
        if (object_cls->IsInitializing() || object_cls->IsInitialized()) {
            if (!TraverseClass<INTERRUPTIBLE>(object_cls, handler)) {
                return false;
            }
        }
    }
    return TraverseObject<INTERRUPTIBLE>(object_header, cls, handler);
}

bool GCDynamicObjectHelpers::IsClassObject(ObjectHeader *obj)
{
    return obj->ClassAddr<HClass>()->IsHClass();
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseClass(coretypes::DynClass *dyn_class, Handler &handler)
{
    size_t hklass_size = dyn_class->ClassAddr<HClass>()->GetObjectSize() - sizeof(coretypes::DynClass);
    size_t body_size = hklass_size - sizeof(HClass);
    size_t num_of_fields = body_size / TaggedValue::TaggedTypeSize();
    for (size_t i = 0; i < num_of_fields; i++) {
        size_t field_offset = sizeof(ObjectHeader) + sizeof(HClass) + i * TaggedValue::TaggedTypeSize();
        auto tagged_value = ObjectAccessor::GetDynValue<TaggedValue>(dyn_class, field_offset);
        if (tagged_value.IsHeapObject()) {
            [[maybe_unused]] bool res = handler(dyn_class, tagged_value.GetHeapObject(), field_offset, false);
            if constexpr (INTERRUPTIBLE) {
                if (!res) {
                    return false;
                }
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseObject(ObjectHeader *object, HClass *cls, Handler &handler)
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
            [[maybe_unused]] bool res = handler(object, tagged_value.GetHeapObject(), field_offset, false);
            if constexpr (INTERRUPTIBLE) {
                if (!res) {
                    return false;
                }
            }
        }
    }
    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseArray(coretypes::Array *array, [[maybe_unused]] HClass *cls, void *begin,
                                           void *end, Handler &handler)
{
    ASSERT(cls != nullptr);
    ASSERT(cls->IsDynamicClass());
    ASSERT(cls->IsArray());
    ASSERT(IsAligned(ToUintPtr(begin), DEFAULT_ALIGNMENT_IN_BYTES));

    auto *array_start = array->GetBase<TaggedType *>();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *array_end = array_start + array->GetLength();
    auto *p = begin < array_start ? array_start : reinterpret_cast<TaggedType *>(begin);

    if (end > array_end) {
        end = array_end;
    }

    auto element_size = coretypes::Array::GetElementSize<TaggedType, true>();
    auto offset = ToUintPtr(p) - ToUintPtr(array);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (ArraySizeT i = p - array_start; p < end; ++p, ++i, offset += element_size) {
        TaggedValue array_element(array->Get<TaggedType, false, true>(i));
        if (array_element.IsHeapObject()) {
            [[maybe_unused]] bool res = handler(array, array_element.GetHeapObject(), offset, false);
            if constexpr (INTERRUPTIBLE) {
                if (!res) {
                    return false;
                }
            }
        }
    }

    return true;
}

template <bool INTERRUPTIBLE, typename Handler>
bool GCDynamicObjectHelpers::TraverseAllObjectsWithInfo(ObjectHeader *object_header, Handler &handler, void *begin,
                                                        void *end)
{
    auto *cls = object_header->ClassAddr<HClass>();
    ASSERT(cls != nullptr && cls->IsDynamicClass());
    if (cls->IsString() || cls->IsNativePointer()) {
        return true;
    }
    if (cls->IsArray()) {
        return TraverseArray<INTERRUPTIBLE>(static_cast<coretypes::Array *>(object_header), cls, begin, end, handler);
    }
    if (cls->IsHClass()) {
        return TraverseClass<INTERRUPTIBLE>(coretypes::DynClass::Cast(object_header), handler);
    }

    return TraverseObject<INTERRUPTIBLE>(object_header, cls, handler);
}

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_OBJECT_HELPERS_INL_H
