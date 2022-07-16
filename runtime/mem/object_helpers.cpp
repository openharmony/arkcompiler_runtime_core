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
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include "runtime/mem/object_helpers-inl.h"

#include "libpandabase/utils/utf.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/free_object.h"
#include "runtime/mem/gc/dynamic/gc_dynamic_data.h"

namespace panda::mem {

using DynClass = coretypes::DynClass;
using TaggedValue = coretypes::TaggedValue;
using TaggedType = coretypes::TaggedType;

Logger::Buffer GetDebugInfoAboutObject(const ObjectHeader *header)
{
    ValidateObject(nullptr, header);

    auto *base_class = header->ClassAddr<BaseClass>();
    const uint8_t *descriptor = nullptr;
    if (base_class->IsDynamicClass()) {
        descriptor = utf::CStringAsMutf8("Dynamic");
    } else {
        descriptor = static_cast<Class *>(base_class)->GetDescriptor();
    }

    const void *rawptr = static_cast<const void *>(header);
    uintmax_t mark = header->AtomicGetMark().GetValue();
    size_t size = header->ObjectSize();

    Logger::Buffer buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    buffer.printf("(\"%s\" %p %zu bytes) mword = %" PRIuMAX, descriptor, rawptr, size, mark);

    return buffer;
}

void DumpObject(ObjectHeader *object_header, std::basic_ostream<char, std::char_traits<char>> *o_stream)
{
    auto *cls = object_header->ClassAddr<Class>();
    ASSERT(cls != nullptr);
    *o_stream << "Dump object object_header = " << std::hex << object_header << ", cls = " << std::hex << cls->GetName()
              << std::endl;

    if (cls->IsArrayClass()) {
        auto array = static_cast<coretypes::Array *>(object_header);
        *o_stream << "Array " << std::hex << object_header << " " << cls->GetComponentType()->GetName()
                  << " length = " << std::dec << array->GetLength() << std::endl;
    } else {
        while (cls != nullptr) {
            Span<Field> fields = cls->GetInstanceFields();
            *o_stream << "Dump object: " << std::hex << object_header << std::endl;
            if (cls->IsStringClass()) {
                auto *str_object = static_cast<panda::coretypes::String *>(object_header);
                if (str_object->GetLength() > 0 && !str_object->IsUtf16()) {
                    *o_stream << "length = " << std::dec << str_object->GetLength() << std::endl;
                    constexpr size_t BUFF_SIZE = 256;
                    std::array<char, BUFF_SIZE> buff {0};
                    auto str_res =
                        strncpy_s(&buff[0], BUFF_SIZE, reinterpret_cast<const char *>(str_object->GetDataMUtf8()),
                                  std::min(BUFF_SIZE - 1, static_cast<size_t>(str_object->GetLength())));
                    if (UNLIKELY(str_res != EOK)) {
                        LOG(ERROR, RUNTIME) << "Couldn't copy string by strncpy_s, error code: " << str_res;
                    }
                    *o_stream << "String data: " << &buff[0] << std::endl;
                }
            }
            for (Field &field : fields) {
                *o_stream << "\tfield \"" << GetFieldName(field) << "\" ";
                size_t offset = field.GetOffset();
                panda_file::Type::TypeId type_id = field.GetTypeId();
                if (type_id == panda_file::Type::TypeId::REFERENCE) {
                    ObjectHeader *field_object = object_header->GetFieldObject(offset);
                    if (field_object != nullptr) {
                        *o_stream << std::hex << field_object << std::endl;
                    } else {
                        *o_stream << "NULL" << std::endl;
                    }
                } else if (type_id != panda_file::Type::TypeId::VOID) {
                    *o_stream << std::dec;
                    switch (type_id) {
                        case panda_file::Type::TypeId::U1: {
                            auto val = object_header->GetFieldPrimitive<bool>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::I8: {
                            auto val = object_header->GetFieldPrimitive<int8_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::U8: {
                            auto val = object_header->GetFieldPrimitive<uint8_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::I16: {
                            auto val = object_header->GetFieldPrimitive<int16_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::U16: {
                            auto val = object_header->GetFieldPrimitive<uint16_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::I32: {
                            auto val = object_header->GetFieldPrimitive<int32_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::U32: {
                            auto val = object_header->GetFieldPrimitive<uint32_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::F32: {
                            auto val = object_header->GetFieldPrimitive<float>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::F64: {
                            auto val = object_header->GetFieldPrimitive<double>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::I64: {
                            auto val = object_header->GetFieldPrimitive<int64_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        case panda_file::Type::TypeId::U64: {
                            auto val = object_header->GetFieldPrimitive<uint64_t>(offset);
                            *o_stream << val << std::endl;
                            break;
                        }
                        default:
                            LOG(FATAL, COMMON) << "Error at object dump - wrong type id";
                    }
                }
            }
            cls = cls->GetBase();
        }
    }
}

template <typename FieldVisitor>
void TraverseFields(const Span<Field> &fields, const Class *cls, const ObjectHeader *object_header,
                    const FieldVisitor &field_visitor)
{
    for (const Field &field : fields) {
        LOG(DEBUG, GC) << " current field \"" << GetFieldName(field) << "\"";
        size_t offset = field.GetOffset();
        panda_file::Type::TypeId type_id = field.GetTypeId();
        if (type_id == panda_file::Type::TypeId::REFERENCE) {
            ObjectHeader *field_object = object_header->GetFieldObject(offset);
            if (field_object != nullptr) {
                LOG(DEBUG, GC) << " field val = " << std::hex << field_object;
                field_visitor(cls, object_header, &field, field_object);
            } else {
                LOG(DEBUG, GC) << " field val = nullptr";
            }
        }
    }
}

void DumpClass(const Class *cls, std::basic_ostream<char, std::char_traits<char>> *o_stream)
{
    if (UNLIKELY(cls == nullptr)) {
        return;
    }
    std::function<void(const Class *, const ObjectHeader *, const Field *, ObjectHeader *)> field_dump(
        [o_stream]([[maybe_unused]] const Class *kls, [[maybe_unused]] const ObjectHeader *obj, const Field *field,
                   ObjectHeader *field_object) {
            *o_stream << "field = " << GetFieldName(*field) << std::hex << " " << field_object << std::endl;
        });
    // Dump class static fields
    *o_stream << "Dump class: addr = " << std::hex << cls << ", cls = " << cls->GetDescriptor() << std::endl;
    *o_stream << "Dump static fields:" << std::endl;
    const Span<Field> &fields = cls->GetStaticFields();
    ObjectHeader *cls_object = cls->GetManagedObject();
    TraverseFields(fields, cls, cls_object, field_dump);
    *o_stream << "Dump cls object fields:" << std::endl;
    DumpObject(cls_object);
}

ObjectHeader *GetForwardAddress(ObjectHeader *object_header)
{
    ASSERT(object_header->IsForwarded());
    MarkWord mark_word = object_header->AtomicGetMark();
    MarkWord::markWordSize addr = mark_word.GetForwardingAddress();
    return reinterpret_cast<ObjectHeader *>(addr);
}

const char *GetFieldName(const Field &field)
{
    static const char *empty_string = "";
    const char *ret = empty_string;
    bool is_proxy = field.GetClass()->IsProxy();
    // For proxy class it is impossible to get field name in standard manner
    if (!is_proxy) {
        ret = reinterpret_cast<const char *>(field.GetName().data);
    }
    return ret;
}

class StdFunctionAdapter {
public:
    explicit StdFunctionAdapter(const std::function<void(ObjectHeader *, ObjectHeader *)> &callback)
        : callback_(callback)
    {
    }

    bool operator()(ObjectHeader *obj, ObjectHeader *field, [[maybe_unused]] uint32_t offset,
                    [[maybe_unused]] bool is_volatile)
    {
        callback_(obj, field);
        return true;
    }

private:
    const std::function<void(ObjectHeader *, ObjectHeader *)> &callback_;
};

void GCDynamicObjectHelpers::TraverseAllObjects(ObjectHeader *object_header,
                                                const std::function<void(ObjectHeader *, ObjectHeader *)> &obj_visitor)
{
    StdFunctionAdapter handler(obj_visitor);
    TraverseAllObjectsWithInfo(object_header, handler);
}

void GCDynamicObjectHelpers::RecordDynWeakReference(GC *gc, coretypes::TaggedType *value)
{
    GCExtensionData *data = gc->GetExtensionData();
    ASSERT(data != nullptr);
    ASSERT(data->GetLangType() == LANG_TYPE_DYNAMIC);
    static_cast<GCDynamicData *>(data)->GetDynWeakReferences()->push(value);
}

void GCDynamicObjectHelpers::HandleDynWeakReferences(GC *gc)
{
    GCExtensionData *data = gc->GetExtensionData();
    ASSERT(data != nullptr);
    ASSERT(data->GetLangType() == LANG_TYPE_DYNAMIC);
    auto *weak_refs = static_cast<GCDynamicData *>(data)->GetDynWeakReferences();
    while (!weak_refs->empty()) {
        coretypes::TaggedType *object_pointer = weak_refs->top();
        weak_refs->pop();
        TaggedValue value(*object_pointer);
        if (value.IsUndefined()) {
            continue;
        }
        ASSERT(value.IsWeak());
        ObjectHeader *object = value.GetWeakReferent();
        /* Note: If it is in young GC, the weak reference whose referent is in tenured space will not be marked. The */
        /*       weak reference whose referent is in young space will be moved into the tenured space or reset in    */
        /*       CollecYoungAndMove. If the weak referent here is not moved in young GC, it shoule be cleared.       */
        if (gc->GetGCPhase() == GCPhase::GC_PHASE_MARK_YOUNG) {
            if (gc->GetObjectAllocator()->IsAddressInYoungSpace(ToUintPtr(object)) && !gc->IsMarked(object)) {
                *object_pointer = TaggedValue::Undefined().GetRawData();
            }
        } else {
            /* Note: When it is in tenured GC, we check whether the referent has been marked. */
            if (!gc->IsMarked(object)) {
                *object_pointer = TaggedValue::Undefined().GetRawData();
            }
        }
    }
}

void GCStaticObjectHelpers::TraverseAllObjects(ObjectHeader *object_header,
                                               const std::function<void(ObjectHeader *, ObjectHeader *)> &obj_visitor)
{
    StdFunctionAdapter handler(obj_visitor);
    TraverseAllObjectsWithInfo(object_header, handler);
}

class StaticUpdateHandler {
public:
    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, bool is_volatile)
    {
        GCStaticObjectHelpers::UpdateRefToMovedObject(object, field, offset, is_volatile);
        return true;
    }
};

void GCStaticObjectHelpers::UpdateRefsToMovedObjects(ObjectHeader *object)
{
    StaticUpdateHandler handler;
    TraverseAllObjectsWithInfo(object, handler);
}

ObjectHeader *GCStaticObjectHelpers::UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset,
                                                            bool is_volatile)
{
    MarkWord mark_word = ref->GetMark();  // no need atomic because stw
    if (mark_word.GetState() != MarkWord::ObjectState::STATE_GC) {
        return ref;
    }
    // update instance field without write barrier
    MarkWord::markWordSize addr = mark_word.GetForwardingAddress();
    LOG(DEBUG, GC) << "update obj ref of object " << object << " from " << ref << " to " << ToVoidPtr(addr);
    auto forwarded_object = reinterpret_cast<ObjectHeader *>(addr);
    if (is_volatile) {
        object->SetFieldObject<true, false>(offset, forwarded_object);
    } else {
        object->SetFieldObject<false, false>(offset, forwarded_object);
    }
    return forwarded_object;
}

class DynamicUpdateHandler {
public:
    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, bool is_volatile)
    {
        GCDynamicObjectHelpers::UpdateRefToMovedObject(object, field, offset, is_volatile);
        return true;
    }
};

void GCDynamicObjectHelpers::UpdateRefsToMovedObjects(ObjectHeader *object)
{
    ASSERT(object->ClassAddr<HClass>()->IsDynamicClass());
    DynamicUpdateHandler handler;
    TraverseAllObjectsWithInfo(object, handler);
}

ObjectHeader *GCDynamicObjectHelpers::UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset,
                                                             [[maybe_unused]] bool is_volatile)
{
    MarkWord mark_word = ref->AtomicGetMark();
    if (mark_word.GetState() != MarkWord::ObjectState::STATE_GC) {
        return ref;
    }
    MarkWord::markWordSize addr = mark_word.GetForwardingAddress();
    LOG(DEBUG, GC) << "update obj ref for object " << object << " from "
                   << ObjectAccessor::GetDynValue<ObjectHeader *>(object, offset) << " to " << ToVoidPtr(addr);
    auto *forwarded_object = reinterpret_cast<ObjectHeader *>(addr);
    if (ObjectAccessor::GetDynValue<TaggedValue>(object, offset).IsWeak()) {
        forwarded_object = TaggedValue(forwarded_object).CreateAndGetWeakRef().GetRawHeapObject();
    }
    ObjectAccessor::SetDynValueWithoutBarrier(object, offset, TaggedValue(forwarded_object).GetRawData());
    return forwarded_object;
}

}  // namespace panda::mem
