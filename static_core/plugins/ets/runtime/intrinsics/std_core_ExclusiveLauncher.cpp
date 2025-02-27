/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "libpandabase/os/mutex.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/reference.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_utils.h"

#include <cstdint>
#include <thread>

namespace ark::ets::intrinsics {

static void RunExclusiveTask(mem::Reference *taskRef, mem::GlobalObjectStorage *refStorage)
{
    ScopedManagedCodeThread managedCode(EtsCoroutine::GetCurrent());
    auto *taskObj = EtsObject::FromCoreType(refStorage->Get(taskRef));
    refStorage->Remove(taskRef);
    LambdaUtils::InvokeVoid(EtsCoroutine::GetCurrent(), taskObj);
}

Coroutine *TryCreateEACoroutine(PandaEtsVM *etsVM, bool needInterop, bool &limitIsReached, bool &jsEnvEmpty,
                                os::memory::Event &event)
{
    auto *runtime = Runtime::GetCurrent();
    auto *coroMan = etsVM->GetCoroutineManager();
    auto *ifaceTable = EtsCoroutine::CastFromThread(coroMan->GetMainThread())->GetExternalIfaceTable();
    auto *jsEnv = ifaceTable->CreateJSRuntime();

    // current we cannot create JSVM instance without jsEnv
    // so we cannot create eaworker support interop withoutJSEnv
    if (jsEnv == nullptr && needInterop) {
        jsEnvEmpty = true;
        event.Fire();
        return nullptr;
    }

    auto *exclusiveCoro = coroMan->CreateExclusiveWorkerForThread(runtime, etsVM);
    // exclusiveCoro == nullptr means that we reached the limit of eaworkers count or memory resources
    if (exclusiveCoro == nullptr) {
        limitIsReached = true;
        event.Fire();
        return nullptr;
    }
    ifaceTable->CreateInteropCtx(exclusiveCoro, jsEnv);
    event.Fire();
    return exclusiveCoro;
}

void RunTaskOnEACoroutine(PandaEtsVM *etsVM, bool needInterop, mem::Reference *taskRef)
{
    auto *refStorage = etsVM->GetGlobalObjectStorage();
    auto *coroMan = etsVM->GetCoroutineManager();

    if (needInterop) {
        auto poster = etsVM->CreateCallbackPoster();
        ASSERT(poster != nullptr);
        poster->Post(RunExclusiveTask, taskRef, refStorage);
        etsVM->RunEventLoop();
    } else {
        RunExclusiveTask(taskRef, refStorage);
    }
    coroMan->DestroyExclusiveWorker();
}

void ExclusiveLaunch(EtsObject *task, uint8_t needInterop)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *etsVM = coro->GetPandaVM();
    if (etsVM->GetCoroutineManager()->IsExclusiveWorkersLimitReached()) {
        ThrowCoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
        return;
    }
    auto *refStorage = etsVM->GetGlobalObjectStorage();
    auto *taskRef = refStorage->Add(task->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(taskRef != nullptr);
    auto limitIsReached = false;
    auto jsEnvEmpty = false;
    bool supportInterop = static_cast<bool>(needInterop);
    auto *worker = coro->GetWorker();
    auto *interopCtx = worker->GetLocalStorage().Get<CoroutineWorker::DataIdx::INTEROP_CTX_PTR, void *>();

    if (supportInterop && interopCtx == nullptr) {
        ThrowRuntimeException("Cannot create EAWorker support interop without JsEnv");
        return;
    }
    {
        ScopedNativeCodeThread nativeScope(coro);
        auto event = os::memory::Event();
        auto t = std::thread([&jsEnvEmpty, &limitIsReached, &event, etsVM, taskRef, supportInterop]() {
            auto *eaCoro = TryCreateEACoroutine(etsVM, supportInterop, limitIsReached, jsEnvEmpty, event);
            if (eaCoro == nullptr) {
                return;
            }
            RunTaskOnEACoroutine(etsVM, supportInterop, taskRef);
        });
        event.Wait();
        t.detach();
    }
    if (limitIsReached) {
        ThrowCoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
        return;
    }
    if (jsEnvEmpty) {
        ThrowRuntimeException("Cannot create EAWorker support interop without JsEnv");
        return;
    }
}

int64_t TaskPosterCreate()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto poster = coro->GetPandaVM()->CreateCallbackPoster();
    ASSERT(poster != nullptr);
    return reinterpret_cast<int64_t>(poster.release());
}

void TaskPosterDestroy(int64_t poster)
{
    auto *taskPoster = reinterpret_cast<CallbackPoster *>(poster);
    ASSERT(taskPoster != nullptr);
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(taskPoster);
}

void TaskPosterPost(int64_t poster, EtsObject *task)
{
    auto *taskPoster = reinterpret_cast<CallbackPoster *>(poster);
    ASSERT(taskPoster != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    auto *refStorage = coro->GetPandaVM()->GetGlobalObjectStorage();
    auto *taskRef = refStorage->Add(task->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    taskPoster->Post(RunExclusiveTask, taskRef, refStorage);
}

}  // namespace ark::ets::intrinsics
