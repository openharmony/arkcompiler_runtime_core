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
#ifndef PANDA_RUNTIME_OBJECT_HEADER_INL_H_
#define PANDA_RUNTIME_OBJECT_HEADER_INL_H_

#include "runtime/include/class-inl.h"
#include "runtime/include/field.h"
#include "runtime/include/object_accessor-inl.h"
#include "runtime/include/object_header.h"

namespace panda {

template <MTModeT MT_MODE>
uint32_t ObjectHeader::GetHashCode()
{
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (MT_MODE == MT_MODE_SINGLE) {
        return GetHashCodeMTSingle();
    } else {  // NOLINT(readability-misleading-indentation)
        return GetHashCodeMTMulti();
    }
}

inline bool ObjectHeader::IsInstanceOf(const Class *klass) const
{
    ASSERT(klass != nullptr);
    return klass->IsAssignableFrom(ClassAddr<Class>());
}

template <class T, bool IS_VOLATILE /* = false */>
inline T ObjectHeader::GetFieldPrimitive(size_t offset) const
{
    return ObjectAccessor::GetPrimitive<T, IS_VOLATILE>(this, offset);
}

template <class T, bool IS_VOLATILE /* = false */>
inline void ObjectHeader::SetFieldPrimitive(size_t offset, T value)
{
    ObjectAccessor::SetPrimitive<T, IS_VOLATILE>(this, offset, value);
}

template <bool IS_VOLATILE /* = false */, bool NEED_READ_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectHeader::GetFieldObject(int offset) const
{
    return ObjectAccessor::GetObject<IS_VOLATILE, NEED_READ_BARRIER, IS_DYN>(this, offset);
}

template <bool IS_VOLATILE /* = false */, bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline void ObjectHeader::SetFieldObject(size_t offset, ObjectHeader *value)
{
    ObjectAccessor::SetObject<IS_VOLATILE, NEED_WRITE_BARRIER, IS_DYN>(this, offset, value);
}

template <class T>
inline T ObjectHeader::GetFieldPrimitive(const Field &field) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, field);
}

template <class T>
inline void ObjectHeader::SetFieldPrimitive(const Field &field, T value)
{
    ObjectAccessor::SetFieldPrimitive(this, field, value);
}

template <bool NEED_READ_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectHeader::GetFieldObject(const Field &field) const
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER, IS_DYN>(this, field);
}

template <bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline void ObjectHeader::SetFieldObject(const Field &field, ObjectHeader *value)
{
    ObjectAccessor::SetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, field, value);
}

template <bool NEED_READ_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectHeader::GetFieldObject(const ManagedThread *thread, const Field &field)
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER, IS_DYN>(thread, this, field);
}

template <bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline void ObjectHeader::SetFieldObject(const ManagedThread *thread, const Field &field, ObjectHeader *value)
{
    ObjectAccessor::SetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(thread, this, field, value);
}

template <bool IS_VOLATILE /* = false */, bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline void ObjectHeader::SetFieldObject(const ManagedThread *thread, size_t offset, ObjectHeader *value)
{
    ObjectAccessor::SetObject<IS_VOLATILE, NEED_WRITE_BARRIER, IS_DYN>(thread, this, offset, value);
}

template <class T>
inline T ObjectHeader::GetFieldPrimitive(size_t offset, std::memory_order memory_order) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, offset, memory_order);
}

template <class T>
inline void ObjectHeader::SetFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    ObjectAccessor::SetFieldPrimitive(this, offset, value, memory_order);
}

template <bool NEED_READ_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectHeader::GetFieldObject(size_t offset, std::memory_order memory_order) const
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER, IS_DYN>(this, offset, memory_order);
}

template <bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline void ObjectHeader::SetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memory_order)
{
    ObjectAccessor::SetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, offset, value, memory_order);
}

template <typename T>
inline bool ObjectHeader::CompareAndSetFieldPrimitive(size_t offset, T old_value, T new_value,
                                                      std::memory_order memory_order, bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, offset, old_value, new_value, memory_order, strong).first;
}

template <bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline bool ObjectHeader::CompareAndSetFieldObject(size_t offset, ObjectHeader *old_value, ObjectHeader *new_value,
                                                   std::memory_order memory_order, bool strong)
{
    return ObjectAccessor::CompareAndSetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, offset, old_value, new_value,
                                                                                memory_order, strong)
        .first;
}

template <typename T>
inline T ObjectHeader::CompareAndExchangeFieldPrimitive(size_t offset, T old_value, T new_value,
                                                        std::memory_order memory_order, bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, offset, old_value, new_value, memory_order, strong).second;
}

template <bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectHeader::CompareAndExchangeFieldObject(size_t offset, ObjectHeader *old_value,
                                                                 ObjectHeader *new_value,
                                                                 std::memory_order memory_order, bool strong)
{
    return ObjectAccessor::CompareAndSetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, offset, old_value, new_value,
                                                                                memory_order, strong)
        .second;
}

template <typename T>
inline T ObjectHeader::GetAndSetFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndSetFieldPrimitive(this, offset, value, memory_order);
}

template <bool NEED_WRITE_BARRIER /* = true */, bool IS_DYN /* = false */>
inline ObjectHeader *ObjectHeader::GetAndSetFieldObject(size_t offset, ObjectHeader *value,
                                                        std::memory_order memory_order)
{
    return ObjectAccessor::GetAndSetFieldObject<NEED_WRITE_BARRIER, IS_DYN>(this, offset, value, memory_order);
}

template <typename T>
inline T ObjectHeader::GetAndAddFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndAddFieldPrimitive(this, offset, value, memory_order);
}

template <typename T>
inline T ObjectHeader::GetAndBitwiseOrFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndBitwiseOrFieldPrimitive(this, offset, value, memory_order);
}

template <typename T>
inline T ObjectHeader::GetAndBitwiseAndFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndBitwiseAndFieldPrimitive(this, offset, value, memory_order);
}

template <typename T>
inline T ObjectHeader::GetAndBitwiseXorFieldPrimitive(size_t offset, T value, std::memory_order memory_order)
{
    return ObjectAccessor::GetAndBitwiseXorFieldPrimitive(this, offset, value, memory_order);
}

}  // namespace panda

#endif  // PANDA_RUNTIME_OBJECT_HEADER_INL_H_
