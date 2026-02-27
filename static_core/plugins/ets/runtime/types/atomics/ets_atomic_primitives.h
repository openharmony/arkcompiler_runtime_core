/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_PRIMITIVES_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_PRIMITIVES_H

#include <type_traits>
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

namespace test {
class EtsAtomicPrimitiveMembers;
}  // namespace test

template <class PrimitiveType>
class EtsAtomicPrimitive : public EtsObject {
    template <class Type>
    using IsIntegral = std::enable_if_t<std::is_integral_v<Type>, bool>;
    template <class Type>
    using IsNonIntegral = std::enable_if_t<!std::is_integral_v<Type>, bool>;

public:
    static EtsAtomicPrimitive<PrimitiveType> *FromCoreType(ObjectHeader *v)
    {
        return reinterpret_cast<EtsAtomicPrimitive<PrimitiveType> *>(v);
    }

    static const EtsAtomicPrimitive<PrimitiveType> *FromCoreType(const ObjectHeader *v)
    {
        return reinterpret_cast<const EtsAtomicPrimitive<PrimitiveType> *>(v);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    const ObjectHeader *GetCoreType() const
    {
        return reinterpret_cast<const ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    static EtsAtomicPrimitive<PrimitiveType> *FromEtsObject(EtsObject *v)
    {
        return reinterpret_cast<EtsAtomicPrimitive<PrimitiveType> *>(v);
    }

    static const EtsAtomicPrimitive<PrimitiveType> *FromEtsObject(const EtsObject *v)
    {
        return reinterpret_cast<const EtsAtomicPrimitive<PrimitiveType> *>(v);
    }

    void StoreValue(PrimitiveType v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        ObjectAccessor::SetFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v, std::memory_order_seq_cst);
    }

    PrimitiveType LoadValue() const
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return ObjectAccessor::GetFieldPrimitive<PrimitiveType>(this, GetValueOffset(), std::memory_order_seq_cst);
    }

    PrimitiveType ExchangeValue(PrimitiveType v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return ObjectAccessor::GetAndSetFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v,
                                                                      std::memory_order_seq_cst);
    }

    PrimitiveType CompareAndSwapValue(PrimitiveType expected, PrimitiveType newValue)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        auto result = ObjectAccessor::CompareAndSetFieldPrimitive<PrimitiveType>(
            this, GetValueOffset(), expected, newValue, std::memory_order_seq_cst, true);
        return static_cast<PrimitiveType>(result.second);
    }

    static EtsBoolean IsLockFreeCheck()
    {
        return static_cast<EtsBoolean>(std::atomic<decltype(value_)>::is_always_lock_free);
    }

    template <class Type = PrimitiveType, IsIntegral<Type> = true>
    PrimitiveType FetchAddValue(Type v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        static_assert(std::is_same_v<Type, PrimitiveType>);
        return ObjectAccessor::GetAndAddFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v,
                                                                      std::memory_order_seq_cst);
    }

    template <class Type = PrimitiveType, IsNonIntegral<Type> = true>
    PrimitiveType FetchAddValue(Type v) = delete;

    template <class Type = PrimitiveType, IsIntegral<Type> = true>
    PrimitiveType FetchSubValue(Type v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        static_assert(std::is_same_v<Type, PrimitiveType>);
        return ObjectAccessor::GetAndSubFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v,
                                                                      std::memory_order_seq_cst);
    }
    template <class Type = PrimitiveType, IsNonIntegral<Type> = true>
    PrimitiveType FetchSubValue(Type v) = delete;

    template <class Type = PrimitiveType, IsIntegral<Type> = true>
    PrimitiveType FetchAndValue(Type v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        static_assert(std::is_same_v<Type, PrimitiveType>);
        return ObjectAccessor::GetAndBitwiseAndFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v,
                                                                             std::memory_order_seq_cst);
    }

    template <class Type = PrimitiveType, IsNonIntegral<Type> = true>
    PrimitiveType FetchAndValue(PrimitiveType v) = delete;

    template <class Type = PrimitiveType, IsIntegral<Type> = true>
    PrimitiveType FetchOrValue(Type v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        static_assert(std::is_same_v<Type, PrimitiveType>);
        return ObjectAccessor::GetAndBitwiseOrFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v,
                                                                            std::memory_order_seq_cst);
    }

    template <class Type = PrimitiveType, IsNonIntegral<Type> = true>
    PrimitiveType FetchOrValue(PrimitiveType v) = delete;

    template <class Type = PrimitiveType, IsIntegral<Type> = true>
    PrimitiveType FetchXorValue(Type v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        static_assert(std::is_same_v<Type, PrimitiveType>);
        return ObjectAccessor::GetAndBitwiseXorFieldPrimitive<PrimitiveType>(this, GetValueOffset(), v,
                                                                             std::memory_order_seq_cst);
    }

    template <class Type = PrimitiveType, IsNonIntegral<Type> = true>
    PrimitiveType FetchXorValue(Type v) = delete;

    static constexpr size_t GetValueOffset()
    {
        return MEMBER_OFFSET(EtsAtomicPrimitive<PrimitiveType>, value_);
    }

    EtsAtomicPrimitive<PrimitiveType>() = delete;
    ~EtsAtomicPrimitive<PrimitiveType>() = delete;

private:
    NO_COPY_SEMANTIC(EtsAtomicPrimitive<PrimitiveType>);
    NO_MOVE_SEMANTIC(EtsAtomicPrimitive<PrimitiveType>);

    PrimitiveType value_;
    friend class test::EtsAtomicPrimitiveMembers;
};

using EtsAtomicBoolean = EtsAtomicPrimitive<EtsBoolean>;
using EtsAtomicChar = EtsAtomicPrimitive<EtsChar>;
using EtsAtomicInt = EtsAtomicPrimitive<EtsInt>;
using EtsAtomicByte = EtsAtomicPrimitive<EtsByte>;
using EtsAtomicShort = EtsAtomicPrimitive<EtsShort>;
using EtsAtomicLong = EtsAtomicPrimitive<EtsLong>;
using EtsAtomicFloat = EtsAtomicPrimitive<EtsFloat>;
using EtsAtomicDouble = EtsAtomicPrimitive<EtsDouble>;

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_LONG_H
