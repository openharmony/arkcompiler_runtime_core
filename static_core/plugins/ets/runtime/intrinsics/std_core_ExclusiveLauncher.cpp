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
    auto *taskObj = EtsObject::FromCoreType(refStorage->Get(taskRef));
    refStorage->Remove(taskRef);
    LambdaUtils::InvokeVoid(EtsCoroutine::GetCurrent(), taskObj);
}

void ExclusiveLaunch(EtsObject *task, uint8_t needInterop)
{
    auto *runtime = Runtime::GetCurrent();
    auto *coro = EtsCoroutine::GetCurrent();
    auto *etsVM = coro->GetPandaVM();
    if (etsVM->GetCoroutineManager()->IsExclusiveWorkersLimitReached()) {
        ThrowCoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
        return;
    }
    auto *refStorage = etsVM->GetGlobalObjectStorage();
    auto *taskRef = refStorage->Add(task->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(taskRef != nullptr);

    ScopedNativeCodeThread nativeScope(coro);
    auto event = os::memory::Event();
    auto limitIsReached = false;
    auto t = std::thread([&limitIsReached, &event, runtime, etsVM, taskRef, refStorage, needInterop] {
        auto *coroMan = etsVM->GetCoroutineManager();
        auto *exclusiveCoro = coroMan->CreateExclusiveWorkerForThread(runtime, etsVM);
        // exclusiveCoro == nullptr means that we reached the limit of eaworkers count or memory resources
        if (exclusiveCoro == nullptr) {
            limitIsReached = true;
            event.Fire();
            return;
        }
        event.Fire();

        if (needInterop != 0U) {
            auto *ifaceTable = EtsCoroutine::CastFromThread(coroMan->GetMainThread())->GetExternalIfaceTable();
            auto *jsEnv = ifaceTable->CreateJSRuntime();
            ASSERT(jsEnv != nullptr);
            ifaceTable->CreateInteropCtx(exclusiveCoro, jsEnv);
            auto poster = etsVM->CreateCallbackPoster(exclusiveCoro);
            ASSERT(poster != nullptr);
            poster->Post(RunExclusiveTask, taskRef, refStorage);
            exclusiveCoro->GetWorker()->SetCallbackPoster(std::move(poster));
            etsVM->RunEventLoop(exclusiveCoro);
        } else {
            ScopedManagedCodeThread ss(exclusiveCoro);
            RunExclusiveTask(taskRef, refStorage);
        }
        coroMan->DestroyExclusiveWorker();
    });
    event.Wait();
    t.detach();

    if (limitIsReached) {
        ThrowCoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
    }
}

int64_t TaskPosterCreate()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto poster = coro->GetPandaVM()->CreateCallbackPoster(coro);
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
