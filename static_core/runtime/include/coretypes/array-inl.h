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
#ifndef PANDA_RUNTIME_CORETYPES_ARRAY_INL_H_
#define PANDA_RUNTIME_CORETYPES_ARRAY_INL_H_

#include <type_traits>

#include "runtime/include/coretypes/array.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/object_accessor-inl.h"
#include "runtime/handle_scope.h"
#include "runtime/mem/vm_handle.h"

namespace ark::coretypes {

// IS_VOLATILE = false
template <class T, bool IS_VOLATILE>
inline T Array::GetPrimitive(size_t offset) const
{
    return ObjectAccessor::GetPrimitive<T, IS_VOLATILE>(this, GetDataOffset() + offset);
}

// IS_VOLATILE = false
template <class T, bool IS_VOLATILE>
inline void Array::SetPrimitive(size_t offset, T value)
{
    ObjectAccessor::SetPrimitive<T, IS_VOLATILE>(this, GetDataOffset() + offset, value);
}

//  IS_VOLATILE = false , NEED_READ_BARRIER = true , bool IS_DYN = false
template <bool IS_VOLATILE, bool NEED_READ_BARRIER, bool IS_DYN>
inline ObjectHeader *Array::GetObject(int offset) const
{
    return ObjectAccessor::GetObject<IS_VOLATILE, NEED_READ_BARRIER, IS_DYN>(this, GetDataOffset() + offset);
}

//  IS_VOLATILE = false , NEED_READ_BARRIER = true , bool IS_DYN = false
template <bool IS_VOLATILE, bool NEED_WRITE_BARRIER, bool IS_DYN>
inline void Array::SetObject(size_t offset, ObjectHeader *value)
{
    ObjectAccessor::SetObject<IS_VOLATILE, NEED_WRITE_BARRIER, IS_DYN>(this, GetDataOffset() + offset, value);
}

template <class T>
inline T Array::GetPrimitive(size_t offset, std::memory_order memoryOrder) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, GetDataOffset() + offset, memoryOrder);
}

template <class T>
inline void Array::SetPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    ObjectAccessor::SetFieldPrimitive(this, GetDataOffset() + offset, value, memoryOrder);
}

// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_READ_BARRIER, bool IS_DYN>
inline ObjectHeader *Array::GetObject(size_t offset, std::memory_order memoryOrder) const
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER, IS_DYN>(this, GetDataOffset() + offset, memoryOrder);
}

// NEED_WRITE_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
inline void Array::SetObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder)
{
    ObjectAccessor::SetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, GetDataOffset() + offset, value, memoryOrder);
}

template <typename T>
inline bool Array::CompareAndSetPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder,
                                          bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, GetDataOffset() + offset, oldValue, newValue, memoryOrder,
                                                       strong)
        .first;
}

// NEED_WRITE_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
inline bool Array::CompareAndSetObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                       std::memory_order memoryOrder, bool strong)
{
    auto fieldOffset = GetDataOffset() + offset;
    return ObjectAccessor::CompareAndSetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, fieldOffset, oldValue, newValue,
                                                                                memoryOrder, strong)
        .first;
}

template <typename T>
inline T Array::CompareAndExchangePrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder,
                                            bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, GetDataOffset() + offset, oldValue, newValue, memoryOrder,
                                                       strong)
        .second;
}

// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
inline ObjectHeader *Array::CompareAndExchangeObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                                     std::memory_order memoryOrder, bool strong)
{
    auto fieldOffset = GetDataOffset() + offset;
    return ObjectAccessor::CompareAndSetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, fieldOffset, oldValue, newValue,
                                                                                memoryOrder, strong)
        .second;
}

template <typename T>
inline T Array::GetAndSetPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndSetFieldPrimitive(this, GetDataOffset() + offset, value, memoryOrder);
}

// NEED_READ_BARRIER = true , IS_DYN = false
template <bool NEED_WRITE_BARRIER, bool IS_DYN>
inline ObjectHeader *Array::GetAndSetObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndSetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, GetDataOffset() + offset, value,
                                                                            memoryOrder);
}

template <typename T>
inline T Array::GetAndAddPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndAddFieldPrimitive(this, GetDataOffset() + offset, value, memoryOrder);
}

template <typename T>
inline T Array::GetAndBitwiseOrPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndBitwiseOrFieldPrimitive(this, GetDataOffset() + offset, value, memoryOrder);
}

template <typename T>
inline T Array::GetAndBitwiseAndPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndBitwiseAndFieldPrimitive(this, GetDataOffset() + offset, value, memoryOrder);
}

template <typename T>
inline T Array::GetAndBitwiseXorPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndBitwiseXorFieldPrimitive(this, GetDataOffset() + offset, value, memoryOrder);
}

template <class T, bool NEED_WRITE_BARRIER, bool IS_DYN>
inline void Array::Set(ArraySizeT idx, T elem)
{
    constexpr bool IS_REF = std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>;

    static_assert(std::is_arithmetic_v<T> || IS_REF, "T should be arithmetic type or pointer to managed object type");

    size_t elemSize = (IS_REF && !IS_DYN) ? sizeof(ObjectPointerType) : sizeof(T);
    size_t offset = elemSize * idx;

    // Disable checks due to clang-tidy bug https://bugs.llvm.org/show_bug.cgi?id=32203
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_REF) {
        ObjectAccessor::SetObject<false, NEED_WRITE_BARRIER, IS_DYN>(this, GetDataOffset() + offset, elem);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        ObjectAccessor::SetPrimitive(this, GetDataOffset() + offset, elem);
    }
}

// NEED_READ_BARRIER = true , IS_DYN = false
template <class T, bool NEED_READ_BARRIER, bool IS_DYN>
inline T Array::Get(ArraySizeT idx) const
{
    constexpr bool IS_REF = std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>;

    static_assert(std::is_arithmetic_v<T> || IS_REF, "T should be arithmetic type or pointer to managed object type");

    size_t offset = GetElementSize<T, IS_DYN>() * idx;

    if constexpr (IS_REF) {
        return ObjectAccessor::GetObject<false, NEED_READ_BARRIER, IS_DYN>(this, GetDataOffset() + offset);
    }

    return ObjectAccessor::GetPrimitive<T>(this, GetDataOffset() + offset);
}

template <class T, bool IS_DYN>
inline constexpr size_t Array::GetElementSize()
{
    constexpr bool IS_REF = std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>;
    static_assert(std::is_arithmetic_v<T> || IS_REF, "T should be arithmetic type or pointer to managed object type");
    return (IS_REF && !IS_DYN) ? sizeof(ObjectPointerType) : sizeof(T);
}

template <class T>
T Array::GetBase()
{
    return reinterpret_cast<T>(ToUintPtr(this) + GetDataOffset());
}

// NEED_WRITE_BARRIER = true , IS_DYN = false
template <class T, bool NEED_WRITE_BARRIER, bool IS_DYN>
inline void Array::Set([[maybe_unused]] const ManagedThread *thread, ArraySizeT idx, T elem)
{
    constexpr bool IS_REF = std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>;

    static_assert(std::is_arithmetic_v<T> || IS_REF, "T should be arithmetic type or pointer to managed object type");

    size_t elemSize = (IS_REF && !IS_DYN) ? sizeof(ObjectPointerType) : sizeof(T);
    size_t offset = elemSize * idx;

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (IS_REF) {
        ObjectAccessor::SetObject<false, NEED_WRITE_BARRIER, IS_DYN>(thread, this, GetDataOffset() + offset, elem);
    } else {  // NOLINTNEXTLINE(readability-misleading-indentation)
        ObjectAccessor::SetPrimitive(this, GetDataOffset() + offset, elem);
    }
}

// NEED_READ_BARRIER = true , IS_DYN = false
template <class T, bool NEED_READ_BARRIER, bool IS_DYN>
inline T Array::Get([[maybe_unused]] const ManagedThread *thread, ArraySizeT idx) const
{
    constexpr bool IS_REF = std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>;

    static_assert(std::is_arithmetic_v<T> || IS_REF, "T should be arithmetic type or pointer to managed object type");

    size_t elemSize = (IS_REF && !IS_DYN) ? sizeof(ObjectPointerType) : sizeof(T);
    size_t offset = elemSize * idx;

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (IS_REF) {
        return ObjectAccessor::GetObject<false, NEED_READ_BARRIER, IS_DYN>(thread, this, GetDataOffset() + offset);
    }
    return ObjectAccessor::GetPrimitive<T>(this, GetDataOffset() + offset);
}

/* static */
template <class DimIterator>
Array *Array::CreateMultiDimensionalArray(ManagedThread *thread, ark::Class *klass, uint32_t nargs,
                                          const DimIterator &iter, size_t dimIdx)
{
    auto arrSize = iter.Get(dimIdx);
    if (arrSize < 0) {
        ark::ThrowNegativeArraySizeException(arrSize);
        return nullptr;
    }
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    // Will be added later special rule for CheckObjHeaderTypeRef and VMHandler.
    // SUPPRESS_CSA_NEXTLINE(alpha.core.CheckObjHeaderTypeRef)
    VMHandle<Array> handle(thread, Array::Create(klass, arrSize));

    // avoid recursive OOM.
    if (handle.GetPtr() == nullptr) {
        return nullptr;
    }
    auto *component = klass->GetComponentType();

    if (component->IsArrayClass() && dimIdx + 1 < nargs) {
        for (int32_t idx = 0; idx < arrSize; idx++) {
            auto *array = CreateMultiDimensionalArray(thread, component, nargs, iter, dimIdx + 1);

            if (array == nullptr) {
                return nullptr;
            }

            handle.GetPtr()->template Set<Array *>(idx, array);
        }
    }

    return handle.GetPtr();
}
}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_ARRAY_INL_H_
