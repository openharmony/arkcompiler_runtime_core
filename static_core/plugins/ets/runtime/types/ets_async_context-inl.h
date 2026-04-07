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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_INL_H

#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"

namespace ark::ets {

void EtsAsyncContext::SetReturnValue(EtsExecutionContext *executionCtx, EtsPromise *returnValue)
{
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, returnValue_),
                              returnValue->GetCoreType());
}

EtsPromise *EtsAsyncContext::GetReturnValue(EtsExecutionContext *executionCtx) const
{
    return EtsPromise::FromCoreType(
        ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, returnValue_)));
}

void EtsAsyncContext::SetAwaitee(EtsExecutionContext *executionCtx, EtsPromise *awaitee)
{
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, awaitee_),
                              awaitee->GetCoreType());
}

EtsPromise *EtsAsyncContext::GetAwaitee(EtsExecutionContext *executionCtx) const
{
    return EtsPromise::FromCoreType(
        ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, awaitee_)));
}

void EtsAsyncContext::SetRefValues(EtsExecutionContext *executionCtx, EtsObjectArray *refValues)
{
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, refValues_),
                              refValues->GetCoreType());
}

EtsObjectArray *EtsAsyncContext::GetRefValues(EtsExecutionContext *executionCtx) const
{
    return EtsObjectArray::FromCoreType(
        ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, refValues_)));
}

void EtsAsyncContext::SetPrimValues(EtsExecutionContext *executionCtx, EtsLongArray *primValues)
{
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, primValues_),
                              primValues->GetCoreType());
}

EtsLongArray *EtsAsyncContext::GetPrimValues(EtsExecutionContext *executionCtx) const
{
    return EtsLongArray::FromCoreType(
        ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, primValues_)));
}

void EtsAsyncContext::SetRefCount(EtsLong refCount)
{
    ObjectAccessor::SetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, refCount_), refCount);
}

EtsLong EtsAsyncContext::GetRefCount() const
{
    return ObjectAccessor::GetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, refCount_));
}

void EtsAsyncContext::SetPrimCount(EtsLong primCount)
{
    ObjectAccessor::SetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, primCount_), primCount);
}

EtsLong EtsAsyncContext::GetPrimCount() const
{
    return ObjectAccessor::GetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, primCount_));
}

void EtsAsyncContext::SetFrameOffsets(EtsExecutionContext *executionCtx, EtsShortArray *frameOffsets)
{
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, frameOffsets_),
                              frameOffsets->GetCoreType());
}

EtsShortArray *EtsAsyncContext::GetFrameOffsets(EtsExecutionContext *executionCtx) const
{
    return EtsShortArray::FromCoreType(
        ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncContext, frameOffsets_)));
}

void EtsAsyncContext::SetCompiledCode(EtsLong compiledCode)
{
    ObjectAccessor::SetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, compiledCode_), compiledCode);
}

EtsLong EtsAsyncContext::GetCompiledCode() const
{
    return ObjectAccessor::GetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, compiledCode_));
}

void EtsAsyncContext::SetAwaitId(EtsLong awaitId)
{
    ObjectAccessor::SetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, awaitId_), awaitId);
}

EtsLong EtsAsyncContext::GetAwaitId() const
{
    return ObjectAccessor::GetPrimitive<EtsLong>(this, MEMBER_OFFSET(EtsAsyncContext, awaitId_));
}

void EtsAsyncContext::SetPc(EtsInt pc)
{
    ObjectAccessor::SetPrimitive<EtsInt>(this, MEMBER_OFFSET(EtsAsyncContext, pc_), pc);
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_INL_H
