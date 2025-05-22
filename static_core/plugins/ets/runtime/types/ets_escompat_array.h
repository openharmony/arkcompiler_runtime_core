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
template <class ElementType>
class EtsArrayObjectMembers;
}  // namespace test

// Mirror class for Array<T> from ETS stdlib
template <class ElementType>
class EtsArrayObject : public EtsObject {
public:
    EtsArrayObject() = delete;
    ~EtsArrayObject() = delete;

    NO_COPY_SEMANTIC(EtsArrayObject);
    NO_MOVE_SEMANTIC(EtsArrayObject);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsArrayObject *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsArrayObject *>(etsObj);
    }

    EtsTypedObjectArray<ElementType> *GetData()
    {
        return reinterpret_cast<EtsTypedObjectArray<ElementType> *>(GetFieldObject(GetBufferOffset()));
    }

    uint32_t GetActualLength()
    {
        return helpers::ToUnsigned(GetFieldPrimitive<EtsInt>(GetActualLengthOffset()));
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsArrayObject, buffer_);
    }

    static constexpr size_t GetActualLengthOffset()
    {
        return MEMBER_OFFSET(EtsArrayObject, actualLength_);
    }

    static EtsArrayObject *Create(size_t length)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro->HasPendingException() == false);

        EtsHandleScope scope(coro);

        const EtsPlatformTypes *platformTypes = PlatformTypes(coro->GetPandaVM());
        EtsClass *klass = platformTypes->escompatArray;
        EtsHandle<EtsArrayObject> arrayHandle(coro, FromEtsObject(EtsObject::Create(klass)));

        auto *ctor = klass->GetDirectMethod(coro->GetLanguageContext().GetCtorName(), "I:V");
        ASSERT(ctor != nullptr);

        std::array args {Value(arrayHandle->GetCoreType()), Value(length)};
        ctor->GetPandaMethod()->InvokeVoid(coro, args.data());

        if (UNLIKELY(coro->HasPendingException())) {
            if (coro->GetPandaVM()->GetOOMErrorObject() == nullptr ||
                (coro->GetException()->ClassAddr<Class>() ==
                 coro->GetPandaVM()->GetOOMErrorObject()->ClassAddr<Class>())) {
                coro->ClearException();
                return nullptr;
            }
            // We do not expect any other exception than OOM
            UNREACHABLE();
        }

        return arrayHandle.GetPtr();
    }

    /// @return Returns a status code of bool indicating success or failure.
    bool SetRef(size_t index, ElementType *ref)
    {
        if (index >= GetActualLength()) {
            return false;
        }

        GetData()->Set(index, ref);
        return true;
    }

    /// @return Returns a status code of bool indicating success or failure.
    bool GetRef(size_t index, ElementType **ref)
    {
        ASSERT(ref != nullptr);
        if (index >= GetActualLength()) {
            return false;
        }

        *ref = GetData()->Get(index);
        return true;
    }

private:
    ObjectPointer<EtsTypedObjectArray<ElementType>> buffer_;
    EtsInt actualLength_;

    friend class test::EtsArrayObjectMembers<ElementType>;
};

using EtsBoxedBooleanArray = EtsArrayObject<EtsBoxPrimitive<EtsBoolean>>;
using EtsBoxedByteArray = EtsArrayObject<EtsBoxPrimitive<EtsByte>>;
using EtsBoxedCharArray = EtsArrayObject<EtsBoxPrimitive<EtsChar>>;
using EtsBoxedShortArray = EtsArrayObject<EtsBoxPrimitive<EtsShort>>;
using EtsBoxedIntArray = EtsArrayObject<EtsBoxPrimitive<EtsInt>>;
using EtsBoxedLongArray = EtsArrayObject<EtsBoxPrimitive<EtsLong>>;
using EtsBoxedFloatArray = EtsArrayObject<EtsBoxPrimitive<EtsFloat>>;
using EtsBoxedDoubleArray = EtsArrayObject<EtsBoxPrimitive<EtsDouble>>;

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H
