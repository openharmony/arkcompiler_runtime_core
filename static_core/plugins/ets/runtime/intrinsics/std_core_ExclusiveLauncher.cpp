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
#include "runtime/execution/job_events.h"
#include "runtime/execution/job_worker_thread-inl.h"
#include "runtime/execution/coroutines/stackful/stackful_coroutine_worker.h"
#include "runtime/execution/dfx/async_stack_scope.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
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

#include <algorithm>
#include <cstdint>
#include <thread>
#include <unordered_map>

namespace ark::ets::intrinsics {

static constexpr EtsInt INVALID_WORKER_ID = -1;

enum class ExclusiveScopeTaskStatus : uint8_t { PENDING, SUCCEEDED, FAILED, CANCELED };

class ScopedExclusiveScopeStack {
public:
    ScopedExclusiveScopeStack(EtsExecutionContext *executionCtx, mem::Reference *stackRef)
        : executionCtx_(executionCtx), previousStackRef_(executionCtx->SetExclusiveScopeStackTraceRef(stackRef))
    {
    }

    ~ScopedExclusiveScopeStack()
    {
        executionCtx_->SetExclusiveScopeStackTraceRef(previousStackRef_);
    }

    NO_COPY_SEMANTIC(ScopedExclusiveScopeStack);
    NO_MOVE_SEMANTIC(ScopedExclusiveScopeStack);

private:
    EtsExecutionContext *executionCtx_;
    mem::Reference *previousStackRef_;
};

class ExclusiveScopeTask {
public:
    ExclusiveScopeTask(JobManager *jobMan, mem::GlobalObjectStorage *storage, mem::Reference *callback)
        : completion_(jobMan), jobManager_(jobMan), refStorage_(storage), callbackRef_(callback)
    {
    }

    GenericEvent *GetCompletion()
    {
        return &completion_;
    }

    mem::GlobalObjectStorage *GetRefStorage() const
    {
        return refStorage_;
    }

    JobManager *GetJobManager() const
    {
        return jobManager_;
    }

    mem::Reference *GetCallbackRef() const
    {
        return callbackRef_;
    }

    mem::Reference *GetResultRef() const
    {
        return resultRef_;
    }

    void SetResultRef(mem::Reference *resultRef)
    {
        resultRef_ = resultRef;
    }

    mem::Reference *GetExceptionRef() const
    {
        return exceptionRef_;
    }

    mem::Reference *GetStackTraceRef() const
    {
        return stackTraceRef_;
    }

    void SetStackTraceRef(mem::Reference *stackTraceRef)
    {
        stackTraceRef_ = stackTraceRef;
    }

    void SetExceptionRef(mem::Reference *exceptionRef)
    {
        exceptionRef_ = exceptionRef;
    }

    ExclusiveScopeTaskStatus GetStatus() const
    {
        return status_;
    }

    void SetStatus(ExclusiveScopeTaskStatus status)
    {
        status_ = status;
    }

    uint64_t GetAsyncStackId() const
    {
        return asyncStackId_;
    }

    void SetAsyncStackId(uint64_t asyncStackId)
    {
        asyncStackId_ = asyncStackId;
    }

private:
    GenericEvent completion_;
    JobManager *jobManager_;
    mem::GlobalObjectStorage *refStorage_;
    mem::Reference *callbackRef_;
    mem::Reference *resultRef_ {nullptr};
    mem::Reference *exceptionRef_ {nullptr};
    mem::Reference *stackTraceRef_ {nullptr};
    ExclusiveScopeTaskStatus status_ {ExclusiveScopeTaskStatus::PENDING};
    uint64_t asyncStackId_ {0U};
};

static void InvokeExclusiveScopeCallbackManaged(ExclusiveScopeTask *task)
{
    ASSERT_MANAGED_CODE();
    auto *thread = ManagedThread::GetCurrent();
    EtsHandleScope scope(EtsExecutionContext::FromMT(thread));
    auto *refStorage = task->GetRefStorage();
    auto *callback = EtsObject::FromCoreType(refStorage->Get(task->GetCallbackRef()));
    auto *result = EtsCall(thread, callback, Span<VMHandle<ObjectHeader>> {});
    if (thread->HasPendingException()) {
        task->SetExceptionRef(refStorage->Add(thread->GetException(), mem::Reference::ObjectType::GLOBAL));
        ASSERT(task->GetExceptionRef() != nullptr);
        thread->ClearException();
        task->SetStatus(ExclusiveScopeTaskStatus::FAILED);
        return;
    }
    if (result != nullptr) {
        VMHandle<EtsObject> resultHandle(thread, result->GetCoreType());
        task->SetResultRef(refStorage->Add(resultHandle.GetPtr()->GetCoreType(), mem::Reference::ObjectType::GLOBAL));
        ASSERT(task->GetResultRef() != nullptr);
    }
    task->SetStatus(ExclusiveScopeTaskStatus::SUCCEEDED);
}

static void ExecuteExclusiveScopeTask(void *data)
{
    auto *task = static_cast<ExclusiveScopeTask *>(data);
    auto *thread = ManagedThread::GetCurrent();
    auto *executionCtx = EtsExecutionContext::FromMT(thread);
    ScopedExclusiveScopeStack stackScope(executionCtx, task->GetStackTraceRef());
    dfx::AsyncStackScope asyncStackScope(task->GetAsyncStackId(), task->GetJobManager()->GetAsyncStackHelper());
    if (thread->IsInNativeCode()) {
        ScopedManagedCodeThread managedCode(thread);
        InvokeExclusiveScopeCallbackManaged(task);
        return;
    }
    InvokeExclusiveScopeCallbackManaged(task);
}

static void CompleteExclusiveScopeTask(void *data)
{
    static_cast<ExclusiveScopeTask *>(data)->GetCompletion()->Happen();
}

static void CancelExclusiveScopeTask(void *data)
{
    auto *task = static_cast<ExclusiveScopeTask *>(data);
    task->SetStatus(ExclusiveScopeTaskStatus::CANCELED);
    task->GetCompletion()->Happen();
}

static OsStackWorkItem CreateExclusiveScopeWorkItem(ExclusiveScopeTask *task)
{
    return OsStackWorkItem {ExecuteExclusiveScopeTask, CompleteExclusiveScopeTask, CancelExclusiveScopeTask, task};
}

static ExclusiveScopeTask *AllocateExclusiveScopeTask(EtsExecutionContext *executionCtx, EtsObject *callback,
                                                      JobManager *jobMan)
{
    auto *refStorage = executionCtx->GetPandaVM()->GetGlobalObjectStorage();
    auto *callbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(callbackRef != nullptr);
    auto *task =
        Runtime::GetCurrent()->GetInternalAllocator()->New<ExclusiveScopeTask>(jobMan, refStorage, callbackRef);
    ASSERT(task != nullptr);
    return task;
}

static void DestroyExclusiveScopeTask(ExclusiveScopeTask *task)
{
    auto *refStorage = task->GetRefStorage();
    refStorage->Remove(task->GetCallbackRef());
    if (task->GetResultRef() != nullptr) {
        refStorage->Remove(task->GetResultRef());
    }
    if (task->GetExceptionRef() != nullptr) {
        refStorage->Remove(task->GetExceptionRef());
    }
    if (task->GetStackTraceRef() != nullptr) {
        refStorage->Remove(task->GetStackTraceRef());
    }
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(task);
}

static EtsObject *ConsumeExclusiveScopeTask(EtsExecutionContext *executionCtx, ExclusiveScopeTask *task)
{
    EtsHandleScope scope(executionCtx);
    auto status = task->GetStatus();
    if (status == ExclusiveScopeTaskStatus::SUCCEEDED && task->GetResultRef() != nullptr) {
        auto *resultObject = task->GetRefStorage()->Get(task->GetResultRef());
        EtsHandle<EtsObject> result(executionCtx, EtsObject::FromCoreType(resultObject));
        DestroyExclusiveScopeTask(task);
        return result.GetPtr();
    }
    if (status == ExclusiveScopeTaskStatus::FAILED) {
        executionCtx->GetMT()->SetException(task->GetRefStorage()->Get(task->GetExceptionRef()));
    }
    DestroyExclusiveScopeTask(task);
    if (status == ExclusiveScopeTaskStatus::CANCELED) {
        ThrowRuntimeException("EAWorker:: exclusiveScope task was canceled during worker shutdown");
        return nullptr;
    }
    ASSERT(status == ExclusiveScopeTaskStatus::SUCCEEDED || status == ExclusiveScopeTaskStatus::FAILED);
    return nullptr;
}

static StackfulCoroutineWorker *ValidateExclusiveScopeCall(EtsExecutionContext *executionCtx, EtsObject *callback)
{
    if (EtsReferenceNullish(executionCtx, callback)) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNullPointerError,
                          "EAWorker:: exclusiveScope callback must not be null");
        return nullptr;
    }
    if (!callback->GetClass()->IsFunction()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreTypeError,
                          "EAWorker:: exclusiveScope callback must be a std.core.Function");
        return nullptr;
    }
    if (Runtime::GetCurrent()->GetOptions().GetCoroutineImpl() != "stackful") {
        ThrowRuntimeException("EAWorker:: exclusiveScope requires stackful coroutines");
        return nullptr;
    }

    auto *jobCtx = JobExecutionContext::CastFromMutator(executionCtx->GetMT());
    auto *worker = jobCtx->GetWorker();
    if (executionCtx->GetTaskpoolTaskId() != 0) {
        ThrowRuntimeException("EAWorker:: exclusiveScope is not supported in taskpool workers");
        return nullptr;
    }
    if (worker == nullptr || !worker->InExclusiveMode()) {
        ThrowRuntimeException("EAWorker:: exclusiveScope can only be called in EAWorker");
        return nullptr;
    }
    if (worker->GetLocalStorage().Get<JobWorkerThread::DataIdx::INTEROP_CTX_PTR, void *>() == nullptr) {
        ThrowRuntimeException("EAWorker:: exclusiveScope requires an EAWorker created with needInterop=true");
        return nullptr;
    }
    return StackfulCoroutineWorker::FromJobWorkerThread(worker);
}

void SetCurrentWorkerPriority(int priority)
{
    QosHelper::SetCurrentWorkerPriority(static_cast<Priority>(priority));
}

EtsBoolean HandleProcessUncaughtError(EtsObject *error, EtsBoolean onlyIfRegistered)
{
    ASSERT(error != nullptr);
    static constexpr const char *HANDLE_UNCAUGHT_ERROR_IF_REGISTERED = "HandleUncaughtErrorIfRegistered";
    static constexpr const char *HANDLE_UNCAUGHT_ERROR_IF_REGISTERED_SIGNATURE = "Lstd/core/Object;:Z";

    auto *method = onlyIfRegistered != 0U
                       ? PlatformTypes()->coreStdProcess->GetStaticMethod(HANDLE_UNCAUGHT_ERROR_IF_REGISTERED,
                                                                          HANDLE_UNCAUGHT_ERROR_IF_REGISTERED_SIGNATURE)
                       : PlatformTypes()->coreStdProcessHandleUncaughtError;
    if (UNLIKELY(method == nullptr)) {
        LOG(ERROR, COROUTINES) << "StdProcess uncaught error handler method is not found";
        return ToEtsBoolean(false);
    }

    std::array args = {Value(error->GetCoreType())};
    auto *thread = ManagedThread::GetCurrent();
    if (onlyIfRegistered != 0U) {
        auto result = method->GetPandaMethod()->Invoke(thread, args.data());
        if (UNLIKELY(thread->HasPendingException())) {
            return ToEtsBoolean(false);
        }
        return result.GetAs<EtsBoolean>();
    }
    method->GetPandaMethod()->InvokeVoid(thread, args.data());
    return ToEtsBoolean(!thread->HasPendingException());
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

static constexpr uint64_t ASYNC_WORK_WAITING_TIME = 100 * 1000U;
static constexpr uint64_t MILLISECONDS_TO_MICROSECONDS = 1000U;
static constexpr uint32_t INTEROP_PUMP_IMMEDIATE_LIMIT = 16U;

static uint64_t ConvertEventLoopTimeoutToMicroseconds(int64_t timeoutMs)
{
    ASSERT(timeoutMs > 0);
    return std::min(static_cast<uint64_t>(timeoutMs) * MILLISECONDS_TO_MICROSECONDS, ASYNC_WORK_WAITING_TIME);
}

static thread_local bool g_gInteropPumpStopRequested = false;
static thread_local bool g_gInteropPumpRunning = false;
static thread_local JobEvent *g_gInteropPumpExitWaiter = nullptr;

static void SetInteropPumpWakeEvent(JobWorkerThread *worker, JobEvent *event)
{
    worker->GetLocalStorage().Set<JobWorkerThread::DataIdx::INTEROP_PUMP_EVENT>(event);
}

static void AwaitInteropPumpEvent(JobWorkerThread *worker, JobEvent *event)
{
    event->Lock();
    SetInteropPumpWakeEvent(worker, event);
    JobExecutionContext::GetCurrent()->GetManager()->Await(event);
    SetInteropPumpWakeEvent(worker, nullptr);
}

static void AwaitInteropPumpDelay(JobWorkerThread *worker, JobManager *jobMan, uint64_t waitingTimeUs)
{
    TimerEvent timerEvt(jobMan, 0);
    timerEvt.SetExpirationTime(jobMan->GetCurrentTime() + waitingTimeUs);
    AwaitInteropPumpEvent(worker, &timerEvt);
}

static void WakeInteropPump(JobWorkerThread *worker)
{
    auto *event = worker->GetLocalStorage().Get<JobWorkerThread::DataIdx::INTEROP_PUMP_EVENT, JobEvent *>();
    if (event != nullptr) {
        event->Happen();
    }
}

static void WaitInteropPumpExit(JobWorkerThread *worker, JobManager *jobMan)
{
    if (!g_gInteropPumpRunning) {
        return;
    }
    GenericEvent exitEvent(jobMan);
    exitEvent.Lock();
    g_gInteropPumpExitWaiter = &exitEvent;
    WakeInteropPump(worker);
    jobMan->Await(&exitEvent);
}

static void StopInteropEventLoopPump(JobWorkerThread *worker, JobManager *jobMan)
{
    g_gInteropPumpStopRequested = true;
    WaitInteropPumpExit(worker, jobMan);
    worker->DestroyCallbackPoster();
}

static void InteropEventLoopPumpEntrypoint([[maybe_unused]] void *param)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *worker = executionCtx->GetWorker();
    auto *jobMan = executionCtx->GetManager();
    auto *etsVM = EtsExecutionContext::FromMT(executionCtx)->GetPandaVM();
    uint32_t immediatePumpCount = 0;

    g_gInteropPumpRunning = true;
    while (worker->IsExternalSchedulingEnabled() && !g_gInteropPumpStopRequested) {
        [[maybe_unused]] auto hasEventLoopWork = etsVM->RunEventLoop(ark::EventLoopRunMode::RUN_NOWAIT);
        if (g_gInteropPumpStopRequested) {
            break;
        }
        auto timeoutMs = etsVM->GetEventLoopBackendTimeout();
        if (timeoutMs == 0 && immediatePumpCount++ < INTEROP_PUMP_IMMEDIATE_LIMIT) {
            continue;
        }
        immediatePumpCount = 0;
        if (timeoutMs < 0) {
            AwaitInteropPumpDelay(worker, jobMan, ASYNC_WORK_WAITING_TIME);
        } else {
            AwaitInteropPumpDelay(worker, jobMan,
                                  timeoutMs == 0 ? ASYNC_WORK_WAITING_TIME
                                                 : ConvertEventLoopTimeoutToMicroseconds(timeoutMs));
        }
    }
    SetInteropPumpWakeEvent(worker, nullptr);
    g_gInteropPumpRunning = false;
    if (g_gInteropPumpExitWaiter != nullptr) {
        g_gInteropPumpExitWaiter->Happen();
        g_gInteropPumpExitWaiter = nullptr;
    }
}

static bool StartInteropEventLoopPump()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *jobMan = executionCtx->GetManager();
    g_gInteropPumpStopRequested = false;
    auto epInfo = Job::NativeEntrypointInfo {InteropEventLoopPumpEntrypoint, nullptr};
    auto *job =
        jobMan->CreateJob("interop event loop pump", epInfo, JobPriority::DEFAULT_PRIORITY, Job::Type::MUTATOR, true);
    auto groupId = JobWorkerThreadGroup::GenerateExactWorkerId(executionCtx->GetWorker()->GetId());
    auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId});
    if (launchResult != LaunchResult::OK) {
        LOG(ERROR, COROUTINES) << "Failed to start interop event loop pump, launch result: "
                               << static_cast<int>(launchResult);
        jobMan->DestroyJob(job);
        return false;
    }
    return true;
}

static void EAWorkerLoop(PandaEtsVM *etsVM, mem::Reference *taskRef, [[maybe_unused]] mem::Reference *joiningPromiseRef,
                         bool supportInterop)
{
    auto *refStorage = etsVM->GetGlobalObjectStorage();
    RunExclusiveTask(taskRef, refStorage);

    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *worker = executionCtx->GetWorker();
    // Pump shutdown awaits its exit and can let an already-admitted user coroutine run. Publish OS-stack queue
    // readiness first so exclusiveScope cannot observe an initialized worker whose executor is not serving yet.
    StackfulCoroutineWorker::FromJobWorkerThread(worker)->StartServingOsStackWork();

    if (supportInterop) {
        StopInteropEventLoopPump(worker, executionCtx->GetManager());
    }

    worker->ExecuteJobsUntilIdle();

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
    if (supportInterop) {
        if (!StartInteropEventLoopPump()) {
            LOG(ERROR, COROUTINES) << "EAWorker failed to start interop event loop pump, periodic scheduling remains "
                                      "active";
        }
    }
    EAWorkerLoop(etsVM, taskRef, joiningPromiseRef, supportInterop);
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
    auto *jobMan = EtsExecutionContext::GetCurrent()->GetPandaVM()->GetJobManager();
    if (Runtime::GetCurrent()->GetOptions().GetCoroutineImpl() == "stackful") {
        jobMan->EnumerateWorkers([workerId](JobWorkerThread *worker) {
            if (worker->GetId() != workerId) {
                return true;
            }
            if (worker->InExclusiveMode()) {
                StackfulCoroutineWorker::FromJobWorkerThread(worker)->StopAcceptingOsStackWork();
            }
            return false;
        });
    }
    g_eaWorkerHelper.StopPeriodicScheduling(workerId);
}

static bool PrepareExclusiveScopeStackTrace(ExclusiveScopeTask *task)
{
    auto *jobMan = task->GetJobManager();
    // A zero ID means that DFX is unavailable or exclusive-scope collection is disabled.
    // Use the same switch for the managed requester snapshot to keep the default path cheap.
    task->SetAsyncStackId(jobMan->GetAsyncStackHelper().CollectAsyncStack(dfx::StackType::STACK_TYPE_EXCLUSIVE_SCOPE,
                                                                          dfx::AsyncStackHelper::DEFAULT_STACK_DEPTH));
    if (task->GetAsyncStackId() == 0U) {
        return true;
    }
    auto *requesterStackTrace = EtsObjectArray::FromCoreType(ArkRuntimeStackTraceProvisionStackTrace());
    if (requesterStackTrace == nullptr) {
        return false;
    }
    task->SetStackTraceRef(
        task->GetRefStorage()->Add(requesterStackTrace->GetCoreType(), mem::Reference::ObjectType::GLOBAL));
    ASSERT(task->GetStackTraceRef() != nullptr);
    return true;
}

static EtsObject *ExecuteQueuedExclusiveScope(EtsExecutionContext *executionCtx, StackfulCoroutineWorker *worker,
                                              ExclusiveScopeTask *task)
{
    auto *completion = task->GetCompletion();
    completion->Lock();
    if (!PrepareExclusiveScopeStackTrace(task)) {
        completion->Unlock();
        DestroyExclusiveScopeTask(task);
        return nullptr;
    }
    auto submitResult = worker->SubmitOsStackWork(CreateExclusiveScopeWorkItem(task));
    if (submitResult != SubmitOsStackWorkResult::ACCEPTED) {
        completion->Unlock();
        DestroyExclusiveScopeTask(task);
        if (submitResult == SubmitOsStackWorkResult::QUEUE_NOT_READY) {
            ThrowRuntimeException("EAWorker:: exclusiveScope cannot be called before the initial task completes");
            return nullptr;
        }
        ThrowRuntimeException("EAWorker:: exclusiveScope cannot be started while the worker is shutting down");
        return nullptr;
    }
    {
        ScopedNativeCodeThread nativeCode(executionCtx->GetMT());
        task->GetJobManager()->Await(completion);
    }
    return ConsumeExclusiveScopeTask(executionCtx, task);
}

EtsObject *EAWorkerExclusiveScope(EtsObject *callback)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *thread = executionCtx->GetMT();
    auto *stackfulWorker = ValidateExclusiveScopeCall(executionCtx, callback);
    if (stackfulWorker == nullptr) {
        return nullptr;
    }
    auto *executor = stackfulWorker->GetOsStackExecutorCoroutine();
    if (executor == nullptr) {
        ThrowRuntimeException("EAWorker:: exclusiveScope OS-stack executor is unavailable");
        return nullptr;
    }
    auto *current = Coroutine::GetCurrent();
    if (current == executor && stackfulWorker->IsOsStackWorkExecuting()) {
        ThrowRuntimeException("EAWorker:: nested exclusiveScope is not allowed");
        return nullptr;
    }

    auto *jobMan = JobExecutionContext::CastFromMutator(thread)->GetManager();
    if (current != executor && jobMan->IsJobSwitchDisabled()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreInvalidJobOperationError,
                          "EAWorker:: exclusiveScope cannot be called while coroutine switching is disabled");
        return nullptr;
    }
    auto *task = AllocateExclusiveScopeTask(executionCtx, callback, jobMan);

    if (current == executor) {
        auto submitResult = stackfulWorker->ExecuteOsStackWorkInline(CreateExclusiveScopeWorkItem(task));
        if (submitResult != SubmitOsStackWorkResult::ACCEPTED) {
            DestroyExclusiveScopeTask(task);
            ThrowRuntimeException("EAWorker:: exclusiveScope cannot be started while the worker is shutting down");
            return nullptr;
        }
        return ConsumeExclusiveScopeTask(executionCtx, task);
    }
    return ExecuteQueuedExclusiveScope(executionCtx, stackfulWorker, task);
}

extern "C" EtsInt StdCoroutineGetExclusiveWorkersLimit()
{
    const auto lang = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    return static_cast<EtsInt>(std::min(static_cast<uint32_t>(AffinityMask::MAX_WORKERS_COUNT - 1U),
                                        Runtime::GetCurrent()->GetOptions().GetCoroutineEWorkersLimit(lang)));
}

}  // namespace ark::ets::intrinsics
