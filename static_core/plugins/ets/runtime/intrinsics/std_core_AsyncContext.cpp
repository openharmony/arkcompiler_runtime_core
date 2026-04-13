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
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/refstorage/reference.h"

namespace ark::ets::intrinsics {

EtsAsyncContext *EtsAsyncContextInit()
{
    // NOTE(panferovi): create new coroutine here and set AsyncContext to it
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *refStorage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();
    auto *asyncCtx = EtsAsyncContext::Create(executionCtx);
    if (UNLIKELY(asyncCtx == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    auto *ctxRef = refStorage->NewEtsRef(asyncCtx, mem::Reference::ObjectType::LOCAL);
    EtsCoroutine::CastFromThread(executionCtx->GetMT())->SetAsyncContext(ctxRef);
    ASSERT(Coroutine::GetCurrent() == executionCtx->GetMT());
    return asyncCtx;
}

EtsAsyncContext *EtsAsyncContextCurrent()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return EtsAsyncContext::GetCurrent(executionCtx);
}

EtsPromise *EtsAsyncContextResolve(EtsAsyncContext *asyncCtx, EtsObject *val)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *returnValue = asyncCtx->GetReturnValue(executionCtx);
    EtsHandleScope s(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, returnValue);
    EtsHandle<EtsObject> hval(executionCtx, val);
    EtsMutex::LockHolder lh(hpromise);
    hpromise->Resolve(executionCtx, hval.GetPtr());
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return hpromise.GetPtr();
}

EtsPromise *EtsAsyncContextReject(EtsAsyncContext *asyncCtx, EtsError *err)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *returnValue = asyncCtx->GetReturnValue(executionCtx);
    EtsHandleScope s(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, returnValue);
    EtsHandle<EtsError> herr(executionCtx, err);
    EtsMutex::LockHolder lh(hpromise);
    hpromise->Reject(executionCtx, herr.GetPtr());
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return hpromise.GetPtr();
}

EtsObject *EtsAsyncContextResult(EtsAsyncContext *asyncCtx)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *returnValue = asyncCtx->GetReturnValue(executionCtx);
    ASSERT(returnValue->IsResolved() || returnValue->IsRejected());
    return returnValue->GetValue(executionCtx);
}

EtsObject *EtsAsyncContextAwaitResult(EtsAsyncContext *asyncCtx)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *awaiteeP = asyncCtx->GetAwaitee(executionCtx);
    ASSERT(awaiteeP->IsResolved() || awaiteeP->IsRejected());
    if (awaiteeP->IsResolved()) {
        return awaiteeP->GetValue(executionCtx);
    }
    executionCtx->GetMT()->SetException(awaiteeP->GetValue(executionCtx)->GetCoreType());
    executionCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(awaiteeP, executionCtx);
    return nullptr;
}

}  // namespace ark::ets::intrinsics
