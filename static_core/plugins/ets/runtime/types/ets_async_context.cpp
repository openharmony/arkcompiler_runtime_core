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

namespace {

template <typename ObjType, auto SETTER, typename... ConstructorArgs>
bool CreateAndSetCtxObject(EtsExecutionContext *const executionCtx, const EtsHandle<EtsAsyncContext> &asyncCtx,
                           ConstructorArgs... constructorArgs)
{
    auto *obj = ObjType::Create(constructorArgs...);
    if (UNLIKELY(obj == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return false;
    }
    (asyncCtx.GetPtr()->*SETTER)(executionCtx, obj);
    return true;
}

}  // namespace

// CC-OFFNXT(G.FUN.01-CPP) solid logic
EtsAsyncContext *EtsAsyncContext::Create(EtsExecutionContext *executionCtx, int32_t refSize, int32_t primSize,
                                         int32_t pc)
{
    EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->arkruntimeAsyncContext;
    auto *ctx = EtsAsyncContext::FromEtsObject(EtsObject::Create(klass));
    EtsHandle<EtsAsyncContext> asyncCtx(executionCtx, ctx);
    if (UNLIKELY(asyncCtx.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    auto *mt = executionCtx->GetMT();
    Method *method = nullptr;
    uint32_t frameSize = 0;
    if (mt->IsCurrentFrameCompiled()) {
        auto stackWalker = StackWalker::Create(mt);
        method = stackWalker.GetMethod();
        ASSERT(method != nullptr);
    } else {
        auto *frame = executionCtx->GetMT()->GetCurrentFrame();
        ASSERT(frame != nullptr);
        method = frame->GetMethod();
        ASSERT(method != nullptr);
        frameSize = method->GetNumVregs() + method->GetNumArgs();
        ASSERT(frameSize <= frame->GetSize());
        ASSERT(frameSize <= static_cast<uint32_t>(std::numeric_limits<EtsShort>::max()));
    }

    if (!CreateAndSetCtxObject<EtsPromise, &EtsAsyncContext::SetReturnValue>(executionCtx, asyncCtx, executionCtx) ||
        !CreateAndSetCtxObject<EtsObjectArray, &EtsAsyncContext::SetRefValues>(
            executionCtx, asyncCtx, PlatformTypes(executionCtx)->coreObject, refSize >= 0 ? refSize : frameSize) ||
        !CreateAndSetCtxObject<EtsLongArray, &EtsAsyncContext::SetPrimValues>(executionCtx, asyncCtx,
                                                                              primSize >= 0 ? primSize : frameSize)) {
        return nullptr;
    }

    asyncCtx->SetRefCount(0);
    asyncCtx->SetPrimCount(0);
    // Default value is ok, since it is ignored by interpreter,
    // and should be right value in compiler
    asyncCtx->SetPc(pc);
    if (pc >= 0) {
        asyncCtx->SetCompiledCode(bit_cast<uintptr_t>(method->GetCompiledEntryPoint()));
    } else {
        asyncCtx->SetCompiledCode(0);
    }

    if (!CreateAndSetCtxObject<EtsShortArray, &EtsAsyncContext::SetFrameOffsets>(
            executionCtx, asyncCtx, (primSize >= 0 || refSize >= 0) ? refSize + primSize : frameSize)) {
        return nullptr;
    }

    return asyncCtx.GetPtr();
}

/* static */
EtsAsyncContext *EtsAsyncContext::GetCurrent(EtsExecutionContext *executionCtx)
{
    if LIKELY (!StackWalker::Create(executionCtx->GetMT()).IsResumed()) {
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
