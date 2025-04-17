/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H

#include <cstdint>

#include "libpandabase/macros.h"
#include "libpandabase/mem/object_pointer.h"
#include "runtime/include/thread.h"
#include "runtime/include/managed_thread.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/entrypoints/entrypoints.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

namespace test {
class EtsEscompatArrayMembers;
}  // namespace test

// Mirror class for Array<T> from ETS stdlib
class EtsEscompatArray : public EtsObject {
public:
    EtsEscompatArray() = delete;
    ~EtsEscompatArray() = delete;

    NO_COPY_SEMANTIC(EtsEscompatArray);
    NO_MOVE_SEMANTIC(EtsEscompatArray);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsEscompatArray *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsEscompatArray *>(etsObj);
    }

    EtsObjectArray *GetData()
    {
        auto *buff = ObjectAccessor::GetObject(this, GetBufferOffset());
        return EtsObjectArray::FromCoreType(buff);
    }

    uint32_t GetActualLength()
    {
        return actualLength_;
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArray, buffer_);
    }

    static constexpr size_t GetActualLengthOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArray, actualLength_);
    }

    static EtsEscompatArray *Create(size_t length)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro->HasPendingException() == false);

        EtsHandleScope scope(coro);

        const EtsPlatformTypes *platformTypes = PlatformTypes();

        // Create escompat array
        EtsClass *klass = platformTypes->escompatArray;
        EtsHandle<EtsEscompatArray> arrayHandle(coro, FromEtsObject(EtsObject::Create(klass)));

        // Create fixed array, field of escompat
        EtsClass *cls = platformTypes->coreObject;
        auto buffer = EtsObjectArray::Create(cls, length);
        ObjectAccessor::SetObject(coro, arrayHandle.GetPtr(), GetBufferOffset(), buffer->GetCoreType());

        // Set length
        arrayHandle->actualLength_ = length;

        return arrayHandle.GetPtr();
    }

    /// @return Returns a status code of bool indicating success or failure.
    bool SetRef(size_t index, EtsObject *ref)
    {
        if (index >= GetActualLength()) {
            return false;
        }

        GetData()->Set(index, ref);
        return true;
    }

    /// @return Returns a status code of bool indicating success or failure.
    bool GetRef(size_t index, EtsObject **ref)
    {
        ASSERT(ref != nullptr);
        if (index >= GetActualLength()) {
            return false;
        }

        *ref = GetData()->Get(index);
        return true;
    }

    void Push(EtsObject *ref)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro->HasPendingException() == false);

        EtsHandleScope scope(coro);

        auto *pushMethod = PlatformTypes()->escompatArrayPush;
        ASSERT(pushMethod != nullptr);

        std::array args {Value(GetCoreType()), Value(ref->GetCoreType())};
        pushMethod->GetPandaMethod()->Invoke(coro, args.data());
    }

    EtsObject *Pop()
    {
        auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro->HasPendingException() == false);

        EtsHandleScope scope(coro);

        auto *popMethod = PlatformTypes()->escompatArrayPop;
        ASSERT(popMethod != nullptr);

        std::array args {Value(GetCoreType())};
        auto res = popMethod->GetPandaMethod()->Invoke(coro, args.data());
        return FromCoreType(res.GetAs<ObjectHeader *>());
    }

private:
    ObjectPointer<EtsObjectArray> buffer_;
    EtsInt actualLength_;

    friend class test::EtsEscompatArrayMembers;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H
