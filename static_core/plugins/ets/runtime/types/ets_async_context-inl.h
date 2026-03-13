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

namespace ark::ets {

void EtsAsyncContext::SetReturnValue(EtsCoroutine *coro, EtsPromise *returnValue)
{
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, returnValue_), returnValue->GetCoreType());
}

EtsPromise *EtsAsyncContext::GetReturnValue(EtsCoroutine *coro) const
{
    return EtsPromise::FromCoreType(
        ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, returnValue_)));
}

void EtsAsyncContext::SetAwaitee(EtsCoroutine *coro, EtsPromise *awaitee)
{
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, awaitee_), awaitee->GetCoreType());
}

EtsPromise *EtsAsyncContext::GetAwaitee(EtsCoroutine *coro) const
{
    return EtsPromise::FromCoreType(ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, awaitee_)));
}

void EtsAsyncContext::SetVRegsRefs(EtsCoroutine *coro, EtsObjectArray *vregsRefs)
{
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, vregsRefs_), vregsRefs->GetCoreType());
}

EtsObjectArray *EtsAsyncContext::GetVRegsRefs(EtsCoroutine *coro) const
{
    return EtsObjectArray::FromCoreType(
        ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, vregsRefs_)));
}

void EtsAsyncContext::SetVRegsPrimitives(EtsCoroutine *coro, EtsLongArray *vregsPrimitives)
{
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, vregsPrimitives_),
                              vregsPrimitives->GetCoreType());
}

EtsLongArray *EtsAsyncContext::GetVRegsPrimitives(EtsCoroutine *coro) const
{
    return EtsLongArray::FromCoreType(
        ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, vregsPrimitives_)));
}

void EtsAsyncContext::SetVRegsMask(EtsCoroutine *coro, EtsByteArray *vregsMask)
{
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, vregsMask_), vregsMask->GetCoreType());
}

EtsByteArray *EtsAsyncContext::GetVRegsMask(EtsCoroutine *coro) const
{
    return EtsByteArray::FromCoreType(
        ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsAsyncContext, vregsMask_)));
}

void EtsAsyncContext::SetAwaitId(EtsLong awaitId)
{
    awaitId_ = awaitId;
}

EtsLong EtsAsyncContext::GetAwaitId()
{
    return awaitId_;
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_INL_H
