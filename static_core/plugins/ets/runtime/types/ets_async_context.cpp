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

#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_platform_types.h"

namespace ark::ets {

EtsAsyncContext *EtsAsyncContext::Create(EtsCoroutine *coro)
{
    EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->arkruntimeAsyncContext;
    auto *ctx = EtsAsyncContext::FromEtsObject(EtsObject::Create(klass));
    if (UNLIKELY(ctx == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    EtsHandle<EtsAsyncContext> asyncCtx(coro, ctx);
    auto *returnValue = EtsPromise::Create(coro);
    if (UNLIKELY(returnValue == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetReturnValue(coro, returnValue);

    auto *vregsRefs = EtsObjectArray::Create(PlatformTypes(coro)->coreObject, INITIAL_VREGS_LENGTH);
    if (UNLIKELY(vregsRefs == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetVRegsRefs(coro, vregsRefs);

    auto *vregsPrimitives = EtsLongArray::Create(INITIAL_VREGS_LENGTH);
    if (UNLIKELY(vregsPrimitives == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetVRegsPrimitives(coro, vregsPrimitives);

    auto *vregsMask = EtsByteArray::Create(INITIAL_VREGS_LENGTH);
    if (UNLIKELY(vregsMask == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetVRegsMask(coro, vregsMask);
    return asyncCtx.GetPtr();
}

void EtsAsyncContext::AddReference(EtsCoroutine *coro, uint32_t idx, EtsObject *ref)
{
    GetVRegsRefs(coro)->Set(idx, ref);
    GetVRegsMask(coro)->Set(idx, VRegType::REFERENCE_TYPE);
}

void EtsAsyncContext::AddPrimitive(EtsCoroutine *coro, uint32_t idx, EtsLong primitive)
{
    GetVRegsPrimitives(coro)->Set(idx, primitive);
    GetVRegsMask(coro)->Set(idx, VRegType::PRIMITIVE_TYPE);
}

EtsAsyncContext::VRegType EtsAsyncContext::GetVRegType(EtsCoroutine *coro, uint32_t idx)
{
    return static_cast<VRegType>(GetVRegsMask(coro)->Get(idx));
}

}  // namespace ark::ets
