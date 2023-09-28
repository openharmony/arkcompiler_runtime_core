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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_

#include "runtime/include/object_header-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_field.h"

namespace panda::ets {

class EtsCoroutine;

// Private inheritance, because need to disallow implicit conversion to core type
class EtsObject : private ObjectHeader {
public:
    static EtsObject *Create(EtsCoroutine *ets_coroutine, EtsClass *klass);
    static EtsObject *CreateNonMovable(EtsClass *klass);

    PANDA_PUBLIC_API static EtsObject *Create(EtsClass *klass);

    EtsClass *GetClass() const
    {
        return EtsClass::FromRuntimeClass(GetCoreType()->ClassAddr<Class>());
    }

    void SetClass(EtsClass *cls)
    {
        GetCoreType()->SetClass(UNLIKELY(cls == nullptr) ? nullptr : cls->GetRuntimeClass());
    }

    bool IsInstanceOf(EtsClass *klass) const
    {
        return klass->IsAssignableFrom(GetClass());
    }

    EtsObject *GetAndSetFieldObject(size_t offset, EtsObject *value, std::memory_order memory_order)
    {
        return FromCoreType(GetCoreType()->GetAndSetFieldObject(offset, value->GetCoreType(), memory_order));
    }

    template <class T>
    T GetFieldPrimitive(EtsField *field)
    {
        return GetCoreType()->GetFieldPrimitive<T>(*field->GetRuntimeField());
    }

    template <class T, bool IS_VOLATILE = false>
    T GetFieldPrimitive(size_t offset)
    {
        return GetCoreType()->GetFieldPrimitive<T, IS_VOLATILE>(offset);
    }

    template <class T>
    T GetFieldPrimitive(int32_t field_offset, bool is_volatile)
    {
        if (is_volatile) {
            return GetCoreType()->GetFieldPrimitive<T, true>(field_offset);
        }
        return GetCoreType()->GetFieldPrimitive<T, false>(field_offset);
    }

    template <class T>
    void SetFieldPrimitive(EtsField *field, T value)
    {
        GetCoreType()->SetFieldPrimitive<T>(*field->GetRuntimeField(), value);
    }

    template <class T>
    void SetFieldPrimitive(int32_t field_offset, bool is_volatile, T value)
    {
        if (is_volatile) {
            GetCoreType()->SetFieldPrimitive<T, true>(field_offset, value);
        }
        GetCoreType()->SetFieldPrimitive<T, false>(field_offset, value);
    }

    template <class T, bool IS_VOLATILE = false>
    void SetFieldPrimitive(size_t offset, T value)
    {
        GetCoreType()->SetFieldPrimitive<T, IS_VOLATILE>(offset, value);
    }

    template <bool NEED_READ_BARRIER = true>
    EtsObject *GetFieldObject(EtsField *field) const
    {
        return reinterpret_cast<EtsObject *>(
            GetCoreType()->GetFieldObject<NEED_READ_BARRIER>(*field->GetRuntimeField()));
    }

    EtsObject *GetFieldObject(int32_t field_offset, bool is_volatile) const
    {
        if (is_volatile) {
            return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<true>(field_offset));
        }
        return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<false>(field_offset));
    }

    template <bool IS_VOLATILE = false>
    EtsObject *GetFieldObject(size_t offset)
    {
        return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<IS_VOLATILE>(offset));
    }

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(EtsField *field, EtsObject *value)
    {
        GetCoreType()->SetFieldObject<NEED_WRITE_BARRIER>(*field->GetRuntimeField(),
                                                          reinterpret_cast<ObjectHeader *>(value));
    }

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(int32_t field_offset, bool is_volatile, EtsObject *value)
    {
        if (is_volatile) {
            GetCoreType()->SetFieldObject<true, NEED_WRITE_BARRIER>(field_offset,
                                                                    reinterpret_cast<ObjectHeader *>(value));
        } else {
            GetCoreType()->SetFieldObject<false, NEED_WRITE_BARRIER>(field_offset,
                                                                     reinterpret_cast<ObjectHeader *>(value));
        }
    }

    template <bool IS_VOLATILE = false>
    void SetFieldObject(size_t offset, EtsObject *value)
    {
        GetCoreType()->SetFieldObject<IS_VOLATILE>(offset, reinterpret_cast<ObjectHeader *>(value));
    }

    void SetFieldObject(size_t offset, EtsObject *value, std::memory_order memory_order)
    {
        GetCoreType()->SetFieldObject(offset, value->GetCoreType(), memory_order);
    }

    template <typename T>
    bool CompareAndSetFieldPrimitive(size_t offset, T old_value, T new_value, std::memory_order memory_order,
                                     bool strong)
    {
        return GetCoreType()->CompareAndSetFieldPrimitive(offset, old_value, new_value, memory_order, strong);
    }

    bool CompareAndSetFieldObject(size_t offset, EtsObject *old_value, EtsObject *new_value,
                                  std::memory_order memory_order, bool strong)
    {
        return GetCoreType()->CompareAndSetFieldObject(offset, reinterpret_cast<ObjectHeader *>(old_value),
                                                       reinterpret_cast<ObjectHeader *>(new_value), memory_order,
                                                       strong);
    }

    EtsObject *Clone() const
    {
        return FromCoreType(ObjectHeader::Clone(GetCoreType()));
    }

    ObjectHeader *GetCoreType() const
    {
        return static_cast<ObjectHeader *>(const_cast<EtsObject *>(this));
    }

    static constexpr EtsObject *FromCoreType(ObjectHeader *object_header)
    {
        return static_cast<EtsObject *>(object_header);
    }

    static constexpr const EtsObject *FromCoreType(const ObjectHeader *object_header)
    {
        return static_cast<const EtsObject *>(object_header);
    }

    bool IsStringClass()
    {
        return GetClass()->IsStringClass();
    }

    bool IsArrayClass()
    {
        return GetClass()->IsArrayClass();
    }

    uint32_t GetHashCode() = delete;

    inline bool IsHashed() const
    {
        return GetMark().GetState() == MarkWord::STATE_HASHED;
    }

    inline uint32_t GetHash() const
    {
        ASSERT(IsHashed());
        return GetMark().GetHash();
    }

    inline void SetHash(uint32_t hash)
    {
        MarkWord mark = GetMark().DecodeFromHash(hash);
        ASSERT(mark.GetState() == MarkWord::STATE_HASHED);
        SetMark(mark);
    }

    EtsObject() = delete;
    ~EtsObject() = delete;

protected:
    // Use type alias to allow using into derived classes
    using ObjectHeader = ::panda::ObjectHeader;

private:
    NO_COPY_SEMANTIC(EtsObject);
    NO_MOVE_SEMANTIC(EtsObject);
};

// Size of EtsObject must be equal size of ObjectHeader
static_assert(sizeof(EtsObject) == sizeof(ObjectHeader));

}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_
