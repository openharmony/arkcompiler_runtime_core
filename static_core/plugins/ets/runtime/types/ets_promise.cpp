/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_callback.h"

namespace ark::ets {

/*static*/
EtsPromise *EtsPromise::Create(EtsCoroutine *etsCoroutine)
{
    EtsClass *klass = etsCoroutine->GetPandaVM()->GetClassLinker()->GetPromiseClass();
    EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
    return reinterpret_cast<EtsPromise *>(etsObject);
}

void EtsPromise::OnPromiseCompletion(EtsCoroutine *coro)
{
    PandaUnorderedMap<EtsCoroutine *, PandaList<PandaUniquePtr<Callback>>> readyCallbacks;
    auto *cbQueue = GetCallbackQueue(coro);
    auto *coroPtrQueue = GetCoroPtrQueue(coro);
    auto queueSize = GetQueueSize();

    for (int idx = 0; idx < queueSize; ++idx) {
        auto *callbackCoro = reinterpret_cast<EtsCoroutine *>(coroPtrQueue->Get(idx));
        auto *thenCallback = cbQueue->Get(idx);
        auto callback = EtsCallback::Create(coro, thenCallback);
        readyCallbacks[callbackCoro].push_back(std::move(callback));
    }

    ClearQueues(coro);
    for (auto &[callbackCoro, callbacks] : readyCallbacks) {
        callbackCoro->AcceptAnnouncedCallbacks(std::move(callbacks));
    }
}

void EtsPromise::EnsureCapacity(EtsCoroutine *coro)
{
    int queueLength = GetCallbackQueue(coro) == nullptr ? 0 : GetCallbackQueue(coro)->GetLength();
    if (queueSize_ > queueLength) {
        return;
    }
    auto newQueueLength = queueLength * 2U + 1U;
    auto *objectClass = coro->GetPandaVM()->GetClassLinker()->GetObjectClass();
    auto *tmpCbQueue = EtsObjectArray::Create(objectClass, newQueueLength);
    if (queueSize_ != 0) {
        GetCallbackQueue(coro)->CopyDataTo(tmpCbQueue);
    }
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, callbackQueue_), tmpCbQueue->GetCoreType());
    auto *newCoroPtrQueue = EtsLongArray::Create(newQueueLength);
    if (queueSize_ != 0) {
        auto *coroPtrQueueData = GetCoroPtrQueue(coro)->GetData<EtsCoroutine *>();
        [[maybe_unused]] auto err =
            memcpy_s(newCoroPtrQueue->GetData<EtsCoroutine *>(), newQueueLength * sizeof(EtsLong), coroPtrQueueData,
                     queueLength * sizeof(EtsCoroutine *));
        ASSERT(err == EOK);
    }
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, coroPtrQueue_), newCoroPtrQueue->GetCoreType());
}

}  // namespace ark::ets
