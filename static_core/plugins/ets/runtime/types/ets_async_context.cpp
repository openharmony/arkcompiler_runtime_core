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
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/stackless/suspendable_job.h"

namespace ark::ets {

EtsAsyncContext *EtsAsyncContext::Create(EtsExecutionContext *executionCtx)
{
    EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->arkruntimeAsyncContext;
    auto *ctx = EtsAsyncContext::FromEtsObject(EtsObject::Create(klass));
    if (UNLIKELY(ctx == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    auto *frame = executionCtx->GetMT()->GetCurrentFrame();
    ASSERT(frame != nullptr);
    auto *method = frame->GetMethod();
    ASSERT(method != nullptr);
    uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
    ASSERT(frameSize <= frame->GetSize());
    ASSERT(frameSize <= static_cast<uint32_t>(std::numeric_limits<EtsShort>::max()));

    EtsHandle<EtsAsyncContext> asyncCtx(executionCtx, ctx);
    auto *returnValue = EtsPromise::Create(executionCtx);
    if (UNLIKELY(returnValue == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetReturnValue(executionCtx, returnValue);

    auto *refValues = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, frameSize);
    if (UNLIKELY(refValues == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetRefValues(executionCtx, refValues);

    auto *primValues = EtsLongArray::Create(frameSize);
    if (UNLIKELY(primValues == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetPrimValues(executionCtx, primValues);

    asyncCtx->SetRefCount(0);
    asyncCtx->SetPrimCount(0);

    auto *frameOffsets = EtsShortArray::Create(frameSize);
    if (UNLIKELY(frameOffsets == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    asyncCtx->SetFrameOffsets(executionCtx, frameOffsets);

    return asyncCtx.GetPtr();
}

/* static */
EtsAsyncContext *EtsAsyncContext::GetCurrent(EtsExecutionContext *executionCtx)
{
    if LIKELY (!executionCtx->GetMT()->GetCurrentFrame()->IsResumed()) {
        return nullptr;
    }
    auto *job = SuspendableJob::FromExecutionContext(JobExecutionContext::CastFromMutator(executionCtx->GetMT()));
    auto *asyncCtxRef = EtsReference::CastFromReference(job->GetSuspensionContext());
    auto *refStorage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();
    return asyncCtxRef == nullptr ? nullptr : EtsAsyncContext::FromEtsObject(refStorage->GetEtsObject(asyncCtxRef));
}

void EtsAsyncContext::AddReference(EtsExecutionContext *executionCtx, uint32_t idx, EtsObject *ref)
{
    GetRefValues(executionCtx)->Set(idx, ref);
}

void EtsAsyncContext::AddPrimitive(EtsExecutionContext *executionCtx, uint32_t idx, EtsLong primitive)
{
    GetPrimValues(executionCtx)->Set(idx, primitive);
}

EtsShort EtsAsyncContext::GetVregOffset(EtsExecutionContext *executionCtx, uint32_t idx) const
{
    ASSERT(idx < 0U + GetRefCount() + GetPrimCount());
    return GetFrameOffsets(executionCtx)->Get(idx);
}

}  // namespace ark::ets
