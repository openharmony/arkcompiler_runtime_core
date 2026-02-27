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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_REF_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_REF_H

#include <atomic>
#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

namespace test {
class EtsAtomicObjectMembers;
}  // namespace test

class EtsAtomicReference : public EtsObject {
public:
    static EtsAtomicReference *FromCoreType(ObjectHeader *v)
    {
        return reinterpret_cast<EtsAtomicReference *>(v);
    }

    static const EtsAtomicReference *FromCoreType(const ObjectHeader *v)
    {
        return reinterpret_cast<const EtsAtomicReference *>(v);
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

    static EtsAtomicReference *FromEtsObject(EtsObject *v)
    {
        return reinterpret_cast<EtsAtomicReference *>(v);
    }

    static const EtsAtomicReference *FromEtsObject(const EtsObject *v)
    {
        return reinterpret_cast<const EtsAtomicReference *>(v);
    }

    void StoreValue(EtsObject *v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        ObjectAccessor::SetFieldObject(this, GetValueOffset(), reinterpret_cast<ObjectHeader *>(v),
                                       std::memory_order_seq_cst);
    }

    EtsObject *LoadValue() const
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return EtsObject::FromCoreType(
            ObjectAccessor::GetFieldObject(this, GetValueOffset(), std::memory_order_seq_cst));
    }

    EtsObject *ExchangeValue(EtsObject *v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        return EtsObject::FromCoreType(ObjectAccessor::GetAndSetFieldObject(
            this, GetValueOffset(), reinterpret_cast<ObjectHeader *>(v), std::memory_order_seq_cst));
    }

    EtsObject *CompareAndSwapValue(EtsObject *expected, EtsObject *newValue)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(value_);
        auto result = ObjectAccessor::CompareAndSetFieldObject(
            this, GetValueOffset(), reinterpret_cast<ObjectHeader *>(expected),
            reinterpret_cast<ObjectHeader *>(newValue), std::memory_order_seq_cst, true);
        return reinterpret_cast<EtsObject *>(result.second);
    }

    static EtsBoolean IsLockFreeCheck()
    {
        return static_cast<EtsBoolean>(std::atomic<decltype(value_)>::is_always_lock_free);
    }

    static constexpr size_t GetValueOffset()
    {
        return MEMBER_OFFSET(EtsAtomicReference, value_);
    }

    EtsAtomicReference() = delete;
    ~EtsAtomicReference() = delete;

private:
    NO_COPY_SEMANTIC(EtsAtomicReference);
    NO_MOVE_SEMANTIC(EtsAtomicReference);

    ObjectPointer<EtsObject> value_;
    friend class test::EtsAtomicObjectMembers;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_REF_H
