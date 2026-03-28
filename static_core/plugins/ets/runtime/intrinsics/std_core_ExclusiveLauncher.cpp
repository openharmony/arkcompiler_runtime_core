/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "runtime/execution/job_priority.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "intrinsics.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/reference.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"

#include <cstdint>
#include <thread>

namespace ark::ets::intrinsics {

static constexpr EtsInt INVALID_WORKER_ID = -1;

void SetCurrentWorkerPriority(int priority)
{
    QosHelper::SetCurrentWorkerPriority(static_cast<Priority>(priority));
}

static void RunExclusiveTask(mem::Reference *taskRef, mem::GlobalObjectStorage *refStorage)
{
    ScopedManagedCodeThread managedCode(ManagedThread::GetCurrent());
    auto *taskObj = EtsObject::FromCoreType(refStorage->Get(taskRef));
    refStorage->Remove(taskRef);
    LambdaUtils::InvokeVoid(ManagedThread::GetCurrent(), taskObj);
}

static JobExecutionContext *TryCreateEAWorker(PandaEtsVM *etsVM, bool needInterop, bool &limitIsReached,
                                              bool &jsEnvEmpty)
{
    auto *runtime = Runtime::GetCurrent();
    auto *jobMan = etsVM->GetJobManager();
    auto *ifaceTable = EtsExecutionContext::FromMT(jobMan->GetMainThread())->GetExternalIfaceTable();
    if (needInterop && !ifaceTable->AreInteropInterfacesAvailable()) {
        jsEnvEmpty = true;
        LOG(ERROR, COROUTINES) << "Cannot create EAWorker support interop without JsEnv";
        return nullptr;
    }
    auto *eaExecCtx = jobMan->AttachExclusiveWorker(runtime, etsVM);
    // eaExecCtx == nullptr means that we reached the limit of eaworkers count or memory resources
    if (eaExecCtx == nullptr) {
        limitIsReached = true;
        LOG(ERROR, COROUTINES) << "The limit of Exclusive Workers has been reached";
        return nullptr;
    }

    return eaExecCtx;
}

static constexpr uint64_t ASYNC_WORK_WAITING_TIME = 100 * 1000U;

void RunTaskOnEAWorker(PandaEtsVM *etsVM, bool needInterop, mem::Reference *taskRef)
{
    RunExclusiveTask(taskRef, etsVM->GetGlobalObjectStorage());
    if (needInterop) {
        auto *w = Coroutine::GetCurrent()->GetWorker();
        while (w->IsExternalSchedulingEnabled()) {
            w->ProcessAsyncWork();
            auto *jobMan = JobExecutionContext::GetCurrent()->GetManager();
            TimerEvent timerEvt(jobMan, 0);
            timerEvt.Lock();
            timerEvt.SetExpirationTime(jobMan->GetCurrentTime() + ASYNC_WORK_WAITING_TIME);
            jobMan->Await(&timerEvt);
        }
    }
}

bool HasPendingError(bool limitIsReached, bool jsEnvEmpty)
{
    if (limitIsReached) {
        ThrowCoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
        return true;
    }
    if (jsEnvEmpty) {
        ThrowRuntimeException("Cannot create EAWorker support interop without JsEnv");
        return true;
    }
    return false;
}

static bool PrepareInteropEnv(PandaEtsVM *etsVM, JobExecutionContext *eaExecCtx)
{
    auto *coroMan = etsVM->GetJobManager();
    auto *ifaceTable = EtsExecutionContext::FromMT(coroMan->GetMainThread())->GetExternalIfaceTable();
    void *jsEnv = nullptr;
    jsEnv = ifaceTable->CreateJSRuntime();
    if (jsEnv == nullptr) {
        LOG(ERROR, COROUTINES) << "Cannot create EAWorker support interop without JsEnv";
        return false;
    }

    ifaceTable->CreateInteropCtx(EtsExecutionContext::FromMT(eaExecCtx), jsEnv);
    return true;
}

static void HandleInteropEnvError()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *method = PlatformTypes()->coreEAWorkerHandleInteropEnvError;
    if (method == nullptr) {
        LOG(ERROR, COROUTINES) << "EAWorker handleInteropEnvError method is not found";
        executionCtx->GetManager()->DetachExclusiveWorker();
        return;
    }
    method->GetPandaMethod()->Invoke(executionCtx, nullptr);
    executionCtx->GetManager()->DetachExclusiveWorker();
}

void DestroyExclusiveWorker(PandaEtsVM *etsVM, bool supportInterop)
{
    auto *jobMan = etsVM->GetJobManager();
    if (supportInterop) {
        auto *ifaceTable = EtsExecutionContext::FromMT(jobMan->GetMainThread())->GetExternalIfaceTable();
        auto *jsEnv = ifaceTable->GetJSEnv();
        jobMan->DetachExclusiveWorker();
        ifaceTable->CleanUpJSEnv(jsEnv);
    } else {
        jobMan->DetachExclusiveWorker();
    }
}

static void SetupAndRunExclusiveWorker(PandaEtsVM *etsVM, JobExecutionContext *eaExecCtx, bool supportInterop,
                                       mem::Reference *taskRef)
{
    bool res = true;
    if (supportInterop) {
        res = PrepareInteropEnv(etsVM, eaExecCtx);
    }
    if (!res) {
        HandleInteropEnvError();
        etsVM->GetGlobalObjectStorage()->Remove(taskRef);
        return;
    }
    RunTaskOnEAWorker(etsVM, supportInterop, taskRef);
    DestroyExclusiveWorker(etsVM, supportInterop);
}

EtsInt ExclusiveLaunch(EtsObject *task, uint8_t needInterop, EtsString *name)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *etsVM = executionCtx->GetPandaVM();
    if (etsVM->GetJobManager()->IsExclusiveWorkersLimitReached()) {
        ThrowCoroutinesLimitExceedError("The limit of Exclusive Workers has been reached");
        return INVALID_WORKER_ID;
    }
    auto *refStorage = etsVM->GetGlobalObjectStorage();
    auto *taskRef = refStorage->Add(task->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(taskRef != nullptr);
    auto limitIsReached = false;
    auto jsEnvEmpty = false;
    bool supportInterop = static_cast<bool>(needInterop);
    int32_t workerId = 0;

    {
        PandaVector<uint8_t> nameBuf;
        EtsHandleScope handleScope(executionCtx);
        EtsHandle<EtsString> nameHandle(executionCtx, name);
        PandaString nameStr(nameHandle->ConvertToStringView(&nameBuf));
        ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
        auto event = os::memory::Event();
        auto t = std::thread([&jsEnvEmpty, &limitIsReached, &event, &workerId, etsVM, taskRef, supportInterop]() {
            auto *eaExecCtx = TryCreateEAWorker(etsVM, supportInterop, limitIsReached, jsEnvEmpty);
            if (eaExecCtx == nullptr) {
                LOG(ERROR, COROUTINES) << "Cannot create EAWorker";
                event.Fire();
                return;
            }
            workerId = eaExecCtx->GetWorker()->GetId();
            event.Fire();
            SetupAndRunExclusiveWorker(etsVM, eaExecCtx, supportInterop, taskRef);
        });
        os::thread::SetThreadName(t.native_handle(), nameStr.c_str());
        event.Wait();
        t.detach();
    }
    if (HasPendingError(limitIsReached, jsEnvEmpty)) {
        refStorage->Remove(taskRef);
        return INVALID_WORKER_ID;
    }
    return workerId;
}

void JoinExclusiveWorker(EtsInt workerId, EtsObject *finalTask)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *jobMan = executionCtx->GetManager();
    auto *taskRef = executionCtx->GetVM()->GetGlobalObjectStorage()->Add(finalTask->GetCoreType(),
                                                                         mem::Reference::ObjectType::GLOBAL);
    auto eaGroup = JobWorkerThreadGroup::GenerateExactWorkerId(workerId);
    auto joiningFunc = [](void *param) {
        auto *ref = static_cast<mem::Reference *>(param);
        auto *eaExecutionCtx = JobExecutionContext::GetCurrent();
        RunExclusiveTask(ref, eaExecutionCtx->GetVM()->GetGlobalObjectStorage());
        eaExecutionCtx->GetWorker()->DestroyCallbackPoster();
    };
    auto epInfo = Job::NativeEntrypointInfo {joiningFunc, taskRef};
    // NOLINTNEXTLINE(performance-move-const-arg)
    auto *job = jobMan->CreateJob("joining ea-coro", std::move(epInfo), JobPriority::DEFAULT_PRIORITY,
                                  Job::Type::MUTATOR, true);
    auto lResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), eaGroup});
    if (UNLIKELY(lResult == LaunchResult::RESOURCE_LIMIT_EXCEED)) {
        jobMan->DestroyJob(job);
    }
}

}  // namespace ark::ets::intrinsics
