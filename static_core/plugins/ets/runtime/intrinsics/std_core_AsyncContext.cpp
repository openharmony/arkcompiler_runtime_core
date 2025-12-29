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

#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_error.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/refstorage/reference.h"

namespace ark::ets::intrinsics {

EtsAsyncContext *EtsAsyncContextInit()
{
    // NOTE(panferovi): create new coroutine here and set AsyncContext to it
    auto *newCoro = EtsCoroutine::GetCurrent();
    auto *refStorage = newCoro->GetPandaAniEnv()->GetEtsReferenceStorage();
    auto *asyncCtx = EtsAsyncContext::Create(newCoro);
    if (UNLIKELY(asyncCtx == nullptr)) {
        ASSERT(newCoro->HasPendingException());
        return nullptr;
    }
    auto *ctxRef = refStorage->NewEtsRef(asyncCtx, mem::Reference::ObjectType::LOCAL);
    newCoro->SetAsyncContext(ctxRef);
    ASSERT(Coroutine::GetCurrent() == newCoro);
    return asyncCtx;
}

EtsAsyncContext *EtsAsyncContextCurrent()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *refStorage = coro->GetPandaAniEnv()->GetEtsReferenceStorage();
    auto *ctxRef = coro->GetAsyncContext();
    return ctxRef == nullptr ? nullptr : EtsAsyncContext::FromEtsObject(refStorage->GetEtsObject(ctxRef));
}

EtsPromise *EtsAsyncContextResolve(EtsAsyncContext *asyncCtx, EtsObject *val)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *returnValue = asyncCtx->GetReturnValue(coro);
    returnValue->Resolve(coro, val);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return returnValue;
}

EtsPromise *EtsAsyncContextReject(EtsAsyncContext *asyncCtx, EtsError *err)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *returnValue = asyncCtx->GetReturnValue(coro);
    returnValue->Reject(coro, err);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return returnValue;
}

EtsObject *EtsAsyncContextResult(EtsAsyncContext *asyncCtx)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *returnValue = asyncCtx->GetReturnValue(coro);
    ASSERT(returnValue->IsResolved() || returnValue->IsRejected());
    return returnValue->GetValue(coro);
}

}  // namespace ark::ets::intrinsics
