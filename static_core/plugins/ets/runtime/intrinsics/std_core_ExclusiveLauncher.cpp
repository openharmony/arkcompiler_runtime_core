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
#include "runtime/execution/affinity_mask.h"
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
#include "plugins/ets/runtime/intrinsics/helpers/intrinsic_timer_impl.h"

#include <cstdint>
#include <thread>
#include <unordered_map>

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

static void ResolveJoiningPromise(mem::Reference *joiningPromiseRef, mem::GlobalObjectStorage *refStorage)
{
    auto *jobCtx = JobExecutionContext::GetCurrent();
    ScopedManagedCodeThread managedCode(jobCtx);
    auto *promise = EtsPromise::FromCoreType(refStorage->Get(joiningPromiseRef));
    auto *execCtx = EtsExecutionContext::FromMT(jobCtx);
    refStorage->Remove(joiningPromiseRef);
    EtsHandleScope s(execCtx);
    EtsHandle<EtsPromise> joiningPromise(execCtx, promise);
    EtsMutex::LockHolder lh(joiningPromise);
    joiningPromise->Resolve(execCtx, nullptr);
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

class SchedulingHelper {
public:
    void StartPeriodicScheduling()
    {
        auto schedEntrypoint = [](void *) {
            auto *jobCtx = JobExecutionContext::GetCurrent();
            auto *jobMan = jobCtx->GetManager();

            jobMan->ExecuteJobs();
            EtsExecutionContext::FromMT(jobCtx)->GetPandaVM()->RunEventLoop(EventLoopRunMode::RUN_NOWAIT);
        };
        auto entrypoint = helpers::NativeEntrypoint {schedEntrypoint, nullptr};
        auto tid = helpers::CreateTimer(std::move(entrypoint), PERIODIC_SCHEDULING_DELAY, true);
        auto wid = JobExecutionContext::GetCurrent()->GetWorker()->GetId();
        {
            os::memory::LockHolder lh(idsLock_);
            [[maybe_unused]] auto [_, inserted] = schedulingJobIds_.insert({wid, tid});
            ASSERT(inserted);
        }
    }

    void StopPeriodicScheduling(JobWorkerThread::Id wid)
    {
        helpers::TimerId tid = 0;
        {
            os::memory::LockHolder lh(idsLock_);
            auto idIt = schedulingJobIds_.find(wid);
            ASSERT(idIt != schedulingJobIds_.end());
            tid = idIt->second;
            schedulingJobIds_.erase(idIt);
        }
        helpers::StopTimer(tid);
    }

private:
    static constexpr uint64_t PERIODIC_SCHEDULING_DELAY = 100U;

    os::memory::Mutex idsLock_;
    std::unordered_map<JobWorkerThread::Id, helpers::TimerId> schedulingJobIds_ GUARDED_BY(idsLock_);
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static SchedulingHelper g_eaWorkerHelper = {};

static void EAWorkerLoop(PandaEtsVM *etsVM, mem::Reference *taskRef, [[maybe_unused]] mem::Reference *joiningPromiseRef)
{
    auto *refStorage = etsVM->GetGlobalObjectStorage();
    RunExclusiveTask(taskRef, refStorage);

    JobExecutionContext::GetCurrent()->GetWorker()->ExecuteJobsUntilIdle();

    ResolveJoiningPromise(joiningPromiseRef, refStorage);
}

static bool HasPendingError(bool limitIsReached, bool jsEnvEmpty)
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

static void DestroyExclusiveWorker(PandaEtsVM *etsVM, bool supportInterop)
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
                                       mem::Reference *taskRef, mem::Reference *joiningPromiseRef)
{
    bool res = true;
    if (supportInterop) {
        res = PrepareInteropEnv(etsVM, eaExecCtx);
    }
    if (!res) {
        g_eaWorkerHelper.StopPeriodicScheduling(eaExecCtx->GetWorker()->GetId());
        HandleInteropEnvError();
        etsVM->GetGlobalObjectStorage()->Remove(taskRef);
        return;
    }
    EAWorkerLoop(etsVM, taskRef, joiningPromiseRef);
    DestroyExclusiveWorker(etsVM, supportInterop);
}

EtsInt ExclusiveLaunch(EtsObject *task, EtsPromise *joiningPromise, EtsString *name, uint8_t needInterop)
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
    auto *joiningPromiseRef = refStorage->Add(joiningPromise->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(joiningPromiseRef != nullptr);
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
        auto t = std::thread(
            [&jsEnvEmpty, &limitIsReached, &event, &workerId, etsVM, taskRef, joiningPromiseRef, supportInterop]() {
                auto *eaExecCtx = TryCreateEAWorker(etsVM, supportInterop, limitIsReached, jsEnvEmpty);
                if (eaExecCtx == nullptr) {
                    LOG(ERROR, COROUTINES) << "Cannot create EAWorker";
                    event.Fire();
                    return;
                }
                workerId = eaExecCtx->GetWorker()->GetId();
                g_eaWorkerHelper.StartPeriodicScheduling();
                event.Fire();
                SetupAndRunExclusiveWorker(etsVM, eaExecCtx, supportInterop, taskRef, joiningPromiseRef);
            });
        os::thread::SetThreadName(t.native_handle(), nameStr.c_str());
        event.Wait();
        t.detach();
    }
    if (HasPendingError(limitIsReached, jsEnvEmpty)) {
        refStorage->Remove(taskRef);
        refStorage->Remove(joiningPromiseRef);
        return INVALID_WORKER_ID;
    }
    return workerId;
}

void JoinExclusiveWorker(EtsInt workerId)
{
    g_eaWorkerHelper.StopPeriodicScheduling(workerId);
}

extern "C" EtsInt StdCoroutineGetExclusiveWorkersLimit()
{
    const auto lang = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    return static_cast<EtsInt>(std::min(static_cast<uint32_t>(AffinityMask::MAX_WORKERS_COUNT - 1U),
                                        Runtime::GetCurrent()->GetOptions().GetCoroutineEWorkersLimit(lang)));
}

}  // namespace ark::ets::intrinsics
