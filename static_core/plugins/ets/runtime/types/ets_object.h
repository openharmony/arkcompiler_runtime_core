/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "mark_word.h"
#include "runtime/include/object_header-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_field.h"

namespace ark::ets {

class EtsCoroutine;

// Private inheritance, because need to disallow implicit conversion to core type
class EtsObject : private ObjectHeader {
public:
    PANDA_PUBLIC_API static EtsObject *Create(EtsCoroutine *etsCoroutine, EtsClass *klass);
    PANDA_PUBLIC_API static EtsObject *CreateNonMovable(EtsClass *klass);

    PANDA_PUBLIC_API static EtsObject *Create(EtsClass *klass);

    PANDA_PUBLIC_API EtsClass *GetClass() const
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

    EtsObject *GetAndSetFieldObject(size_t offset, EtsObject *value, std::memory_order memoryOrder)
    {
        return FromCoreType(GetCoreType()->GetAndSetFieldObject(offset, value->GetCoreType(), memoryOrder));
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
    T GetFieldPrimitive(int32_t fieldOffset, bool isVolatile)
    {
        if (isVolatile) {
            return GetCoreType()->GetFieldPrimitive<T, true>(fieldOffset);
        }
        return GetCoreType()->GetFieldPrimitive<T, false>(fieldOffset);
    }

    template <class T>
    void SetFieldPrimitive(EtsField *field, T value)
    {
        GetCoreType()->SetFieldPrimitive<T>(*field->GetRuntimeField(), value);
    }

    template <class T>
    void SetFieldPrimitive(int32_t fieldOffset, bool isVolatile, T value)
    {
        if (isVolatile) {
            GetCoreType()->SetFieldPrimitive<T, true>(fieldOffset, value);
        }
        GetCoreType()->SetFieldPrimitive<T, false>(fieldOffset, value);
    }

    template <class T, bool IS_VOLATILE = false>
    void SetFieldPrimitive(size_t offset, T value)
    {
        GetCoreType()->SetFieldPrimitive<T, IS_VOLATILE>(offset, value);
    }

    template <bool NEED_READ_BARRIER = true>
    PANDA_PUBLIC_API EtsObject *GetFieldObject(EtsField *field) const
    {
        return reinterpret_cast<EtsObject *>(
            GetCoreType()->GetFieldObject<NEED_READ_BARRIER>(*field->GetRuntimeField()));
    }

    EtsObject *GetFieldObject(int32_t fieldOffset, bool isVolatile) const
    {
        if (isVolatile) {
            return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<true>(fieldOffset));
        }
        return reinterpret_cast<EtsObject *>(GetCoreType()->GetFieldObject<false>(fieldOffset));
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
    void SetFieldObject(int32_t fieldOffset, bool isVolatile, EtsObject *value)
    {
        if (isVolatile) {
            GetCoreType()->SetFieldObject<true, NEED_WRITE_BARRIER>(fieldOffset,
                                                                    reinterpret_cast<ObjectHeader *>(value));
        } else {
            GetCoreType()->SetFieldObject<false, NEED_WRITE_BARRIER>(fieldOffset,
                                                                     reinterpret_cast<ObjectHeader *>(value));
        }
    }

    template <bool IS_VOLATILE = false>
    void SetFieldObject(size_t offset, EtsObject *value)
    {
        GetCoreType()->SetFieldObject<IS_VOLATILE>(offset, reinterpret_cast<ObjectHeader *>(value));
    }

    void SetFieldObject(size_t offset, EtsObject *value, std::memory_order memoryOrder)
    {
        GetCoreType()->SetFieldObject(offset, value->GetCoreType(), memoryOrder);
    }

    template <typename T>
    bool CompareAndSetFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder, bool strong)
    {
        return GetCoreType()->CompareAndSetFieldPrimitive(offset, oldValue, newValue, memoryOrder, strong);
    }

    bool CompareAndSetFieldObject(size_t offset, EtsObject *oldValue, EtsObject *newValue,
                                  std::memory_order memoryOrder, bool strong)
    {
        return GetCoreType()->CompareAndSetFieldObject(offset, reinterpret_cast<ObjectHeader *>(oldValue),
                                                       reinterpret_cast<ObjectHeader *>(newValue), memoryOrder, strong);
    }

    EtsObject *Clone() const
    {
        return FromCoreType(ObjectHeader::Clone(GetCoreType()));
    }

    ObjectHeader *GetCoreType() const
    {
        return static_cast<ObjectHeader *>(const_cast<EtsObject *>(this));
    }

    static constexpr EtsObject *FromCoreType(ObjectHeader *objectHeader)
    {
        return static_cast<EtsObject *>(objectHeader);
    }

    static constexpr const EtsObject *FromCoreType(const ObjectHeader *objectHeader)
    {
        return static_cast<const EtsObject *>(objectHeader);
    }

    PANDA_PUBLIC_API bool IsStringClass()
    {
        return GetClass()->IsStringClass();
    }

    PANDA_PUBLIC_API bool IsArrayClass()
    {
        return GetClass()->IsArrayClass();
    }

    // NOTE(ipetrov, #20886): Support separated Interop and Hash states
    /**
     * Get hash code if object has been already hashed,
     * or generate and set hash code in the object header and return it
     * @return hash code for the object
     */
    uint32_t GetHashCode();

    inline bool IsHashed() const
    {
        return AtomicGetMark().GetState() == MarkWord::STATE_HASHED;
    }

    inline uint32_t GetInteropHash() const
    {
        ASSERT(IsHashed());
        return GetMark().GetHash();
    }

    inline void SetInteropHash(uint32_t hash)
    {
        MarkWord oldMark = AtomicGetMark();
        ASSERT(oldMark.GetState() == ark::MarkWord::STATE_UNLOCKED);
        MarkWord newMark = oldMark.DecodeFromHash(hash);
        ASSERT(newMark.GetState() == MarkWord::STATE_HASHED);
        [[maybe_unused]] bool res = AtomicSetMark(oldMark, newMark);
        ASSERT(res);  // NOTE(vpukhov): something went wrong
    }

    inline void DropInteropHash()
    {
        ASSERT_MANAGED_CODE();
        MarkWord oldMark = AtomicGetMark();
        ASSERT(oldMark.GetState() == MarkWord::STATE_HASHED);
        MarkWord newMark = oldMark.DecodeFromUnlocked();
        [[maybe_unused]] bool res = AtomicSetMark(oldMark, newMark);
        ASSERT(res);  // NOTE(vpukhov): something went wrong
    }

    EtsObject() = delete;
    ~EtsObject() = delete;

protected:
    // Use type alias to allow using into derived classes
    using ObjectHeader = ::ark::ObjectHeader;

private:
    NO_COPY_SEMANTIC(EtsObject);
    NO_MOVE_SEMANTIC(EtsObject);
};

// Size of EtsObject must be equal size of ObjectHeader
static_assert(sizeof(EtsObject) == sizeof(ObjectHeader));

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_OBJECT_H_
