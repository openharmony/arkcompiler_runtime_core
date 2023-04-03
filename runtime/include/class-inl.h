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
#ifndef PANDA_RUNTIME_CLASS_INL_H_
#define PANDA_RUNTIME_CLASS_INL_H_

#include "runtime/include/class.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/field.h"
#include "runtime/include/object_header.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_accessor-inl.h"

namespace panda {

template <typename Item>
struct NameComp {
    bool equal(const Item &m, const panda_file::File::StringData &name) const
    {
        return m.GetName() == name;
    }
    bool operator()(const Method &m, const panda_file::File::StringData &name) const
    {
        return m.GetName() < name;
    }
};

template <typename Item>
struct EntityIdComp {
    bool equal(const Item &m, panda_file::File::EntityId id) const
    {
        return m.GetFileId().GetOffset() == id.GetOffset();
    }
    bool operator()(const Item &m, panda_file::File::EntityId id) const
    {
        return m.GetFileId().GetOffset() < id.GetOffset();
    }
};

using MethodNameComp = NameComp<Method>;
using MethodIdComp = EntityIdComp<Method>;

template <typename Item>
ALWAYS_INLINE inline bool PredComp([[maybe_unused]] const Item &item)
{
    return true;
}

template <typename Item, typename FirstPred, typename... Pred>
ALWAYS_INLINE inline bool PredComp(const Item &item, const FirstPred &pred, const Pred &... preds)
{
    return pred(item) && PredComp(item, preds...);
}

template <typename KeyComp, typename Key, typename Item, typename... Pred>
ALWAYS_INLINE inline Item *BinSearch(Span<Item> items, Key key, const Pred &... preds)
{
    auto it = std::lower_bound(items.begin(), items.end(), key, KeyComp());
    while (it != items.end()) {
        auto &item = *it;
        if (!KeyComp().equal(item, key)) {
            break;
        }
        if (PredComp(item, preds...)) {
            return &item;
        }
        ++it;
    }
    return nullptr;
}

inline uint32_t Class::GetTypeSize(panda_file::Type type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::U8:
            return sizeof(uint8_t);
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16:
            return sizeof(uint16_t);
        case panda_file::Type::TypeId::I32:
        case panda_file::Type::TypeId::U32:
        case panda_file::Type::TypeId::F32:
            return sizeof(uint32_t);
        case panda_file::Type::TypeId::I64:
        case panda_file::Type::TypeId::U64:
        case panda_file::Type::TypeId::F64:
            return sizeof(uint64_t);
        case panda_file::Type::TypeId::TAGGED:
            return coretypes::TaggedValue::TaggedTypeSize();
        case panda_file::Type::TypeId::REFERENCE:
            return ClassHelper::OBJECT_POINTER_SIZE;
        default:
            UNREACHABLE();
    }
}

inline uint32_t Class::GetComponentSize() const
{
    if (component_type_ == nullptr) {
        return 0;
    }

    return GetTypeSize(component_type_->GetType());
}

inline bool Class::IsClassClass() const
{
    return GetManagedObject()->ClassAddr<Class>() == this;
}

inline bool Class::IsSubClassOf(const Class *klass) const
{
    const Class *current = this;

    do {
        if (current == klass) {
            return true;
        }

        current = current->GetBase();
    } while (current != nullptr);

    return false;
}

inline bool Class::IsAssignableFrom(const Class *klass) const
{
    if (klass == this) {
        return true;
    }
    if (IsObjectClass()) {
        return !klass->IsPrimitive();
    }
    if (IsInterface()) {
        return klass->Implements(this);
    }
    if (klass->IsArrayClass()) {
        return IsArrayClass() && GetComponentType()->IsAssignableFrom(klass->GetComponentType());
    }
    return !klass->IsInterface() && klass->IsSubClassOf(this);
}

inline bool Class::Implements(const Class *klass) const
{
    for (const auto &elem : itable_.Get()) {
        if (elem.GetInterface() == klass) {
            return true;
        }
    }

    return false;
}

template <Class::FindFilter filter>
inline Span<Field> Class::GetFields() const
{
    switch (filter) {
        case FindFilter::STATIC:
            return GetStaticFields();
        case FindFilter::INSTANCE:
            return GetInstanceFields();
        case FindFilter::ALL:
            return GetFields();
        default:
            UNREACHABLE();
    }
}

template <Class::FindFilter filter, class Pred>
inline Field *Class::FindDeclaredField(Pred pred) const
{
    auto fields = GetFields<filter>();
    auto it = std::find_if(fields.begin(), fields.end(), pred);
    if (it != fields.end()) {
        return &*it;
    }
    return nullptr;
}

ALWAYS_INLINE inline Field *BinarySearchField(Span<Field> fields, panda_file::File::EntityId id)
{
    auto comp = [](const Field &field, panda_file::File::EntityId field_id) { return field.GetFileId() < field_id; };
    auto it = std::lower_bound(fields.begin(), fields.end(), id, comp);
    if (it != fields.end() && (*it).GetFileId() == id) {
        return &*it;
    }
    return nullptr;
}

template <Class::FindFilter filter>
inline Field *Class::FindDeclaredField(panda_file::File::EntityId id) const
{
    if (filter == FindFilter::ALL) {
        auto static_fields = GetStaticFields();
        auto *static_field = BinarySearchField(static_fields, id);
        if (static_field != nullptr) {
            return static_field;
        }
        auto instance_fields = GetInstanceFields();
        auto *instance_field = BinarySearchField(instance_fields, id);
        if (instance_field != nullptr) {
            return instance_field;
        }
    } else {
        auto fields = GetFields<filter>();
        auto *field = BinarySearchField(fields, id);
        if (field != nullptr) {
            return field;
        }
    }

    return nullptr;
}

template <Class::FindFilter filter, class Pred>
inline Field *Class::FindField(Pred pred) const
{
    auto *cls = this;
    while (cls != nullptr) {
        auto *field = cls->FindDeclaredField<filter>(pred);
        if (field != nullptr) {
            return field;
        }

        cls = cls->GetBase();
    }

    if (filter == FindFilter::STATIC || filter == FindFilter::ALL) {
        auto *kls = this;
        while (kls != nullptr) {
            for (auto *iface : kls->GetInterfaces()) {
                auto *field = iface->FindField<filter>(pred);
                if (field != nullptr) {
                    return field;
                }
            }

            kls = kls->GetBase();
        }
    }

    return nullptr;
}

template <Class::FindFilter filter>
inline Span<Method> Class::GetMethods() const
{
    switch (filter) {
        case FindFilter::STATIC:
            return GetStaticMethods();
        case FindFilter::INSTANCE:
            return GetVirtualMethods();
        case FindFilter::ALL:
            return GetMethods();
        case FindFilter::COPIED:
            return GetCopiedMethods();
        default:
            UNREACHABLE();
    }
}

template <Class::FindFilter filter, typename KeyComp, typename Key, typename... Pred>
inline Method *Class::FindDirectMethod(Key key, const Pred &... preds) const
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (filter == FindFilter::ALL || filter == FindFilter::STATIC) {
        auto methods = GetMethods<FindFilter::STATIC>();
        auto *method = BinSearch<KeyComp>(methods, key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (filter == FindFilter::ALL || filter == FindFilter::INSTANCE) {
        auto methods = GetMethods<FindFilter::INSTANCE>();
        auto *method = BinSearch<KeyComp>(methods, key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    // Copied methods come from implemented interfaces default methods and unsorted,
    // they can't be sorted by both id and name, so just visit method one by one now
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (filter == FindFilter::COPIED) {
        auto methods = GetMethods<FindFilter::COPIED>();
        for (auto &method : methods) {
            if (KeyComp().equal(method, key) && PredComp(method, preds...)) {
                return &method;
            }
        }
    }

    return nullptr;
}

template <Class::FindFilter filter, typename KeyComp, typename Key, typename... Pred>
inline Method *Class::FindClassMethod(Key key, const Pred &... preds) const
{
    auto *cls = this;
    while (cls != nullptr) {
        auto *method = cls->FindDirectMethod<filter, KeyComp>(key, preds...);
        if (method != nullptr) {
            return method;
        }
        cls = cls->GetBase();
    }

    if (filter == FindFilter::ALL || filter == FindFilter::INSTANCE) {
        return FindClassMethod<FindFilter::COPIED, KeyComp>(key, preds...);
    }

    return nullptr;
}

template <Class::FindFilter filter, typename KeyComp, typename Key, typename... Pred>
inline Method *Class::FindInterfaceMethod(Key key, const Pred &... preds) const
{
    static_assert(filter != FindFilter::COPIED, "interfaces don't have copied methods");

    if (LIKELY(IsInterface())) {
        auto *method = FindDirectMethod<filter, KeyComp>(key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    if (filter == FindFilter::STATIC) {
        return nullptr;
    }

    for (const auto &entry : itable_.Get()) {
        auto *iface = entry.GetInterface();
        auto *method = iface->FindDirectMethod<FindFilter::INSTANCE, KeyComp>(key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    if (LIKELY(IsInterface())) {
        return GetBase()->FindDirectMethod<FindFilter::INSTANCE, KeyComp>(
            key, [&](const Method &method) { return method.IsPublic() && PredComp(method, preds...); });
    }

    return nullptr;
}

inline Method *Class::GetVirtualInterfaceMethod(panda_file::File::EntityId id) const
{
    return FindInterfaceMethod<FindFilter::INSTANCE, MethodIdComp>(id);
}

inline Method *Class::GetStaticInterfaceMethod(panda_file::File::EntityId id) const
{
    return FindInterfaceMethod<FindFilter::STATIC, MethodIdComp>(id);
}

template <class Pred>
inline Field *Class::FindInstanceField(Pred pred) const
{
    return FindField<FindFilter::INSTANCE>(pred);
}

inline Field *Class::FindInstanceFieldById(panda_file::File::EntityId id) const
{
    return FindField<FindFilter::INSTANCE>(id);
}

template <class Pred>
inline Field *Class::FindStaticField(Pred pred) const
{
    return FindField<FindFilter::STATIC>(pred);
}

inline Field *Class::FindStaticFieldById(panda_file::File::EntityId id) const
{
    return FindField<FindFilter::STATIC>(id);
}

template <class Pred>
inline Field *Class::FindField(Pred pred) const
{
    return FindField<FindFilter::ALL>(pred);
}

template <class Pred>
inline Field *Class::FindDeclaredField(Pred pred) const
{
    return FindDeclaredField<FindFilter::ALL>(pred);
}

inline Field *Class::GetInstanceFieldByName(const uint8_t *mutf8_name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return FindInstanceField([sd](const Field &field) { return field.GetName() == sd; });
}

inline Field *Class::GetStaticFieldByName(const uint8_t *mutf8_name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return FindStaticField([sd](const Field &field) { return field.GetName() == sd; });
}

inline size_t Class::GetStaticFieldsOffset() const
{
    return ComputeClassSize(vtable_size_, imt_size_, 0, 0, 0, 0, 0, 0);
}

inline Field *Class::GetDeclaredFieldByName(const uint8_t *mutf8_name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return FindDeclaredField([sd](const Field &field) { return field.GetName() == sd; });
}

inline Method *Class::GetVirtualClassMethod(panda_file::File::EntityId id) const
{
    return FindClassMethod<FindFilter::INSTANCE, MethodIdComp>(id);
}

inline Method *Class::GetStaticClassMethod(panda_file::File::EntityId id) const
{
    return FindClassMethod<FindFilter::STATIC, MethodIdComp>(id);
}

inline Method *Class::GetDirectMethod(const uint8_t *mutf8_name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return FindDirectMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetClassMethod(const uint8_t *mutf8_name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return GetClassMethod(sd, proto);
}

inline Method *Class::GetClassMethod(const panda_file::File::StringData &sd, const Method::Proto &proto) const
{
    return FindClassMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetStaticClassMethodByName(const panda_file::File::StringData &sd,
                                                 const Method::Proto &proto) const
{
    return FindClassMethod<FindFilter::STATIC, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetVirtualClassMethodByName(const panda_file::File::StringData &sd,
                                                  const Method::Proto &proto) const
{
    return FindClassMethod<FindFilter::INSTANCE, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetInterfaceMethod(const uint8_t *mutf8_name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return GetInterfaceMethod(sd, proto);
}

inline Method *Class::GetInterfaceMethod(const panda_file::File::StringData &sd, const Method::Proto &proto) const
{
    return FindInterfaceMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetStaticInterfaceMethodByName(const panda_file::File::StringData &sd,
                                                     const Method::Proto &proto) const
{
    return FindInterfaceMethod<FindFilter::STATIC, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetVirtualInterfaceMethodByName(const panda_file::File::StringData &sd,
                                                      const Method::Proto &proto) const
{
    return FindInterfaceMethod<FindFilter::INSTANCE, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetDirectMethod(const uint8_t *mutf8_name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return FindDirectMethod<FindFilter::ALL, MethodNameComp>(sd);
}

inline Method *Class::GetClassMethod(const uint8_t *mutf8_name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(mutf8_name)), mutf8_name};
    return FindClassMethod<FindFilter::ALL, MethodNameComp>(sd);
}

inline Method *Class::ResolveVirtualMethod(const Method *method) const
{
    Method *resolved = nullptr;

    ASSERT(!IsInterface());

    if (method->GetClass()->IsInterface() && !method->IsDefaultInterfaceMethod()) {
        // find method in imtable
        auto imtable_size = GetIMTSize();
        if (LIKELY(imtable_size != 0)) {
            auto imtable = GetIMT();
            auto method_id = GetIMTableIndex(method->GetFileId().GetOffset());
            resolved = imtable[method_id];
            if (resolved != nullptr) {
                return resolved;
            }
        }

        // find method in itable
        auto *iface = method->GetClass();
        auto itable = GetITable();
        for (size_t i = 0; i < itable.Size(); i++) {
            auto &entry = itable[i];
            if (entry.GetInterface() != iface) {
                continue;
            }

            resolved = entry.GetMethods()[method->GetVTableIndex()];
        }
    } else {
        // find method in vtable
        auto vtable = GetVTable();
        ASSERT(method->GetVTableIndex() < vtable.size());
        resolved = vtable[method->GetVTableIndex()];
    }

    return resolved;
}

constexpr size_t Class::ComputeClassSize(size_t vtable_size, size_t imt_size, size_t num_8bit_sfields,
                                         size_t num_16bit_sfields, size_t num_32bit_sfields, size_t num_64bit_sfields,
                                         size_t num_ref_sfields, size_t num_tagged_sfields)
{
    size_t size = sizeof(Class);
    size = AlignUp(size, ClassHelper::OBJECT_POINTER_SIZE);
    size += vtable_size * ClassHelper::POINTER_SIZE;
    size += imt_size * ClassHelper::POINTER_SIZE;
    size += num_ref_sfields * ClassHelper::OBJECT_POINTER_SIZE;

    constexpr size_t SIZE_64 = sizeof(uint64_t);
    constexpr size_t SIZE_32 = sizeof(uint32_t);
    constexpr size_t SIZE_16 = sizeof(uint16_t);
    constexpr size_t SIZE_8 = sizeof(uint8_t);

    // Try to fill alignment gaps with fields that have smaller size from largest to smallests
    static_assert(coretypes::TaggedValue::TaggedTypeSize() == SIZE_64,
                  "Please fix alignment of the fields of type \"TaggedValue\"");
    if (!IsAligned<SIZE_64>(size) && (num_64bit_sfields > 0 || num_tagged_sfields > 0)) {
        size_t padding = AlignUp(size, SIZE_64) - size;
        size += padding;

        Pad(SIZE_32, &padding, &num_32bit_sfields);
        Pad(SIZE_16, &padding, &num_16bit_sfields);
        Pad(SIZE_8, &padding, &num_8bit_sfields);
    }

    if (!IsAligned<SIZE_32>(size) && num_32bit_sfields > 0) {
        size_t padding = AlignUp(size, SIZE_32) - size;
        size += padding;

        Pad(SIZE_16, &padding, &num_16bit_sfields);
        Pad(SIZE_8, &padding, &num_8bit_sfields);
    }

    if (!IsAligned<SIZE_16>(size) && num_16bit_sfields > 0) {
        size_t padding = AlignUp(size, SIZE_16) - size;
        size += padding;

        Pad(SIZE_8, &padding, &num_8bit_sfields);
    }

    size += num_64bit_sfields * SIZE_64 + num_32bit_sfields * SIZE_32 + num_16bit_sfields * SIZE_16 +
            num_8bit_sfields * SIZE_8 + num_tagged_sfields * coretypes::TaggedValue::TaggedTypeSize();

    return size;
}

constexpr void Class::Pad(size_t size, size_t *padding, size_t *n)
{
    while (*padding >= size && *n > 0) {
        *padding -= size;
        *n -= 1;
    }
}

constexpr size_t Class::GetVTableOffset()
{
    return ComputeClassSize(0, 0, 0, 0, 0, 0, 0, 0);
}

inline Span<Method *> Class::GetVTable()
{
    return GetClassSpan().SubSpan<Method *>(GetVTableOffset(), vtable_size_);
}

inline Span<Method *const> Class::GetVTable() const
{
    return GetClassSpan().SubSpan<Method *const>(GetVTableOffset(), vtable_size_);
}

inline size_t Class::GetIMTOffset() const
{
    return GetVTableOffset() + vtable_size_ * sizeof(uintptr_t);
}

template <class T, bool is_volatile /* = false */>
inline T Class::GetFieldPrimitive(size_t offset) const
{
    ASSERT_DO(IsInitializing() || IsInitialized(), LOG(ERROR, RUNTIME) << "class state: " << state_);
    return ObjectAccessor::GetPrimitive<T, is_volatile>(this, offset);
}

template <class T, bool is_volatile /* = false */>
inline void Class::SetFieldPrimitive(size_t offset, T value)
{
    ObjectAccessor::SetPrimitive<T, is_volatile>(this, offset, value);
}

template <bool is_volatile /* = false */, bool need_read_barrier /* = true */>
inline ObjectHeader *Class::GetFieldObject(size_t offset) const
{
    // TODO(alovkov): GC can skip classes which are IsErroneous #6458
    // GC can't easily check state & get fields because state can be changed concurrently and checking on every field
    // is too expensive and it should be atomic {check state, get field}
    ASSERT_DO(IsInitializing() || IsInitialized() || IsErroneous(), LOG(ERROR, RUNTIME) << "class state: " << state_);
    return ObjectAccessor::GetObject<is_volatile, need_read_barrier>(this, offset);
}

template <bool is_volatile /* = false */, bool need_write_barrier /* = true */>
inline void Class::SetFieldObject(size_t offset, ObjectHeader *value)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto new_offset = offset + (ToUintPtr(this) - ToUintPtr(object));
    ObjectAccessor::SetObject<is_volatile, need_write_barrier>(object, new_offset, value);
}

template <class T>
inline T Class::GetFieldPrimitive(const Field &field) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, field);
}

template <class T>
inline void Class::SetFieldPrimitive(const Field &field, T value)
{
    ObjectAccessor::SetFieldPrimitive(this, field, value);
}

template <bool need_read_barrier /* = true */>
inline ObjectHeader *Class::GetFieldObject(const Field &field) const
{
    return ObjectAccessor::GetFieldObject<need_read_barrier>(this, field);
}

template <bool need_write_barrier /* = true */>
inline void Class::SetFieldObject(const Field &field, ObjectHeader *value)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto offset = field.GetOffset() + (ToUintPtr(this) - ToUintPtr(object));
    if (UNLIKELY(field.IsVolatile())) {
        ObjectAccessor::SetObject<true, need_write_barrier>(object, offset, value);
    } else {
        ObjectAccessor::SetObject<false, need_write_barrier>(object, offset, value);
    }
}

template <bool need_read_barrier /* = true */>
inline ObjectHeader *Class::GetFieldObject(ManagedThread *thread, const Field &field) const
{
    return ObjectAccessor::GetFieldObject<need_read_barrier>(thread, this, field);
}

template <bool need_write_barrier /* = true */>
inline void Class::SetFieldObject(ManagedThread *thread, const Field &field, ObjectHeader *value)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto offset = field.GetOffset() + (ToUintPtr(this) - ToUintPtr(object));
    if (UNLIKELY(field.IsVolatile())) {
        ObjectAccessor::SetObject<true, need_write_barrier>(thread, object, offset, value);
    } else {
        ObjectAccessor::SetObject<false, need_write_barrier>(thread, object, offset, value);
    }
}

template <class T>
inline T Class::GetFieldPrimitive(size_t offset, std::memory_order memory_order) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, offset, memory_order);
}

template <class T>
inline void Class::SetFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    ObjectAccessor::SetFieldPrimitive(this, offset, value, memory_order);
}

template <bool need_read_barrier /* = true */>
inline ObjectHeader *Class::GetFieldObject(size_t offset, std::memory_order memory_order) const
{
    return ObjectAccessor::GetFieldObject<need_read_barrier>(this, offset, memory_order);
}

template <bool need_write_barrier /* = true */>
inline void Class::SetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memory_order)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto new_offset = offset + (ToUintPtr(this) - ToUintPtr(object));
    ObjectAccessor::SetFieldObject<need_write_barrier>(object, new_offset, value, memory_order);
}

template <typename T>
inline bool Class::CompareAndSetFieldPrimitive(size_t offset, T old_value, T new_value, std::memory_order memory_order,
                                               bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, offset, old_value, new_value, memory_order, strong).first;
}

template <bool need_write_barrier /* = true */>
inline bool Class::CompareAndSetFieldObject(size_t offset, ObjectHeader *old_value, ObjectHeader *new_value,
                                            std::memory_order memory_order, bool strong)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto new_offset = offset + (ToUintPtr(this) - ToUintPtr(object));
    return ObjectAccessor::CompareAndSetFieldObject<need_write_barrier>(object, new_offset, old_value, new_value,
                                                                        memory_order, strong).first;
}

template <typename T>
inline T Class::CompareAndExchangeFieldPrimitive(size_t offset, T old_value, T new_value,
                                                 std::memory_order memory_order, bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, offset, old_value, new_value, memory_order, strong).second;
}

template <bool need_write_barrier /* = true */>
inline ObjectHeader *Class::CompareAndExchangeFieldObject(size_t offset, ObjectHeader *old_value,
                                                          ObjectHeader *new_value, std::memory_order memory_order,
                                                          bool strong)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto new_offset = offset + (ToUintPtr(this) - ToUintPtr(object));
    return ObjectAccessor::CompareAndSetFieldObject<need_write_barrier>(object, new_offset, old_value, new_value,
                                                                        memory_order, strong).second;
}

template <typename T>
inline T Class::GetAndSetFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndSetFieldPrimitive(this, offset, value, memory_order);
}

template <bool need_write_barrier /* = true */>
inline ObjectHeader *Class::GetAndSetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memory_order)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto new_offset = offset + (ToUintPtr(this) - ToUintPtr(object));
    return ObjectAccessor::GetAndSetFieldObject<need_write_barrier>(object, new_offset, value, memory_order);
}

template <typename T>
inline T Class::GetAndAddFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndAddFieldPrimitive(this, offset, value, memory_order);
}

template <typename T>
inline T Class::GetAndBitwiseOrFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndBitwiseOrFieldPrimitive(this, offset, value, memory_order);
}

template <typename T>
inline T Class::GetAndBitwiseAndFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndBitwiseAndFieldPrimitive(this, offset, value, memory_order);
}

template <typename T>
inline T Class::GetAndBitwiseXorFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndBitwiseXorFieldPrimitive(this, offset, value, memory_order);
}

}  // namespace panda

#endif  // PANDA_RUNTIME_CLASS_INL_H_
