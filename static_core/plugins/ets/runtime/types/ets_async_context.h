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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_H

#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_promise.h"

namespace ark::ets {

class EtsCoroutine;

namespace test {
class EtsAsyncContextTest;
}  // namespace test

class EtsAsyncContext final : public EtsObject {
public:
    static EtsAsyncContext *Create(EtsCoroutine *coro);

    inline void SetReturnValue(EtsCoroutine *coro, EtsPromise *returnValue);
    inline void SetAwaitee(EtsCoroutine *coro, EtsPromise *awaitee);
    inline void SetVRegsRefs(EtsCoroutine *coro, EtsObjectArray *vregsRefs);
    inline void SetVRegsPrimitives(EtsCoroutine *coro, EtsLongArray *vregsPrimitives);
    inline void SetVRegsMask(EtsCoroutine *coro, EtsByteArray *vregsMask);
    inline void SetAwaitId(EtsLong awaitId);
    inline EtsPromise *GetReturnValue(EtsCoroutine *coro) const;
    inline EtsPromise *GetAwaitee(EtsCoroutine *coro) const;
    inline EtsObjectArray *GetVRegsRefs(EtsCoroutine *coro) const;
    inline EtsLongArray *GetVRegsPrimitives(EtsCoroutine *coro) const;
    inline EtsByteArray *GetVRegsMask(EtsCoroutine *coro) const;
    inline EtsLong GetAwaitId();

    enum VRegType : EtsByte { REFERENCE_TYPE, PRIMITIVE_TYPE };

    void AddReference(EtsCoroutine *coro, uint32_t idx, EtsObject *ref);
    void AddPrimitive(EtsCoroutine *coro, uint32_t idx, EtsLong primitive);
    VRegType GetVRegType(EtsCoroutine *coro, uint32_t idx);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsAsyncContext *FromCoreType(ObjectHeader *asyncCtx)
    {
        return reinterpret_cast<EtsAsyncContext *>(asyncCtx);
    }

    static EtsAsyncContext *FromEtsObject(EtsObject *asyncCtx)
    {
        return reinterpret_cast<EtsAsyncContext *>(asyncCtx);
    }

private:
    static constexpr uint32_t INITIAL_VREGS_LENGTH = 16;

    ObjectPointer<EtsPromise> awaitee_;
    ObjectPointer<EtsPromise> returnValue_;
    ObjectPointer<EtsByteArray> vregsMask_;
    ObjectPointer<EtsObjectArray> vregsRefs_;
    ObjectPointer<EtsLongArray> vregsPrimitives_;
    EtsLong awaitId_;

    friend class test::EtsAsyncContextTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_H
