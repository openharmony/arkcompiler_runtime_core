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

#include <atomic>
#include <algorithm>
#include <thread>
#include "plugins/ets/runtime/ets_execution_context.h"
#include "libarkbase/os/time.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_taskpool.h"
#include "runtime/execution/affinity_mask.h"
#include "runtime/execution/coroutines/coroutine_manager.h"
#include "runtime/execution/coroutines/coroutine_worker.h"
#include "runtime/execution/dfx/async_stack_helper.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_manager.h"
#include "runtime/include/thread.h"
#include "runtime/include/runtime.h"

namespace ark::ets::intrinsics::taskpool {
namespace {

dfx::AsyncStackHelper *GetAsyncStackHelper()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    if (executionCtx == nullptr || executionCtx->GetManager() == nullptr) {
        return nullptr;
    }
    return &executionCtx->GetManager()->GetAsyncStackHelper();
}

}  // namespace

static std::atomic<EtsInt> g_taskId = 1;
static std::atomic<EtsInt> g_taskGroupId = 1;
static std::atomic<EtsInt> g_seqRunnerId = 1;
static std::atomic<EtsInt> g_asyncRunnerId = 1;

uint32_t GetDefaultTaskPoolEWorkersLimit(const RuntimeOptions &options)
{
    const auto lang = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);

    // Hardware parallelism provides the desired taskpool size. One CPU is kept for
    // the main execution flow, and small machines still get at least one worker.
    const uint32_t cpuCount = std::thread::hardware_concurrency();
    const uint32_t hardwareLimit = cpuCount > 1 ? cpuCount - 1 : 1;  // 1: default number
    const uint32_t requestedTaskpoolLimit = std::max(TASKPOOL_MIN_EWORKERS_LIMIT, hardwareLimit);

    // The coroutine manager has one global worker id space. Taskpool workers must
    // therefore leave room for the user EAWorker limit and common coroutine workers.
    const uint32_t userEWorkersLimit =
        std::min(static_cast<uint32_t>(AffinityMask::MAX_WORKERS_COUNT - 1U), options.GetCoroutineEWorkersLimit(lang));
    const uint32_t configuredCommonWorkers = options.GetCoroutineWorkersCount(lang);
    const uint32_t commonWorkersBudget =
        configuredCommonWorkers == CoroutineManagerConfig::WORKERS_COUNT_AUTO
            ? std::max(cpuCount / 4U, JobManager::MIN_COMMON_WORKERS_COUNT)
            : std::min(static_cast<uint32_t>(AffinityMask::MAX_WORKERS_COUNT), configuredCommonWorkers);

    uint32_t availableTaskpoolWorkers = TASKPOOL_INITIAL_EWORKERS_COUNT;
    // Use the current common-worker configuration when calculating the default
    // taskpool size. This makes explicit --coroutine-workers-count values reduce
    // the taskpool budget instead of overflowing AffinityMask::MAX_WORKERS_COUNT.
    const uint32_t reservedWorkers =
        userEWorkersLimit + commonWorkersBudget + TASKPOOL_MANAGER_EWORKERS_COUNT + MAIN_EWORKER_RESERVED_COUNT;
    if (AffinityMask::MAX_WORKERS_COUNT > reservedWorkers) {
        availableTaskpoolWorkers = std::max(TASKPOOL_INITIAL_EWORKERS_COUNT,
                                            static_cast<uint32_t>(AffinityMask::MAX_WORKERS_COUNT) - reservedWorkers);
    }

    // Runtime keeps at least MIN_COMMON_WORKERS_COUNT common workers. Keep a
    // second upper bound with that minimum for explicit configurations below it.
    const uint32_t minimumReservedWorkers = userEWorkersLimit + TASKPOOL_MANAGER_EWORKERS_COUNT +
                                            MAIN_EWORKER_RESERVED_COUNT + JobManager::MIN_COMMON_WORKERS_COUNT;
    uint32_t maxTaskpoolWorkers = TASKPOOL_INITIAL_EWORKERS_COUNT;
    if (AffinityMask::MAX_WORKERS_COUNT > minimumReservedWorkers) {
        maxTaskpoolWorkers = static_cast<uint32_t>(AffinityMask::MAX_WORKERS_COUNT) - minimumReservedWorkers;
    }

    // Final default = hardware request, capped by both the configured budget and
    // the absolute budget that protects the shared worker id space.
    return std::min({requestedTaskpoolLimit, availableTaskpoolWorkers, maxTaskpoolWorkers});
}

extern "C" EtsInt GenerateTaskId()
{
    return g_taskId++;
}

extern "C" EtsInt GetTaskId()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto id = executionCtx->GetTaskpoolTaskId();
    return id;
}

extern "C" void SetTaskId(EtsInt taskId)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    executionCtx->SetTaskpoolTaskId(taskId);
}

extern "C" EtsInt GetSubmissionId()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    return executionCtx->GetTaskpoolSubmissionId();
}

extern "C" void SetSubmissionId(EtsInt submissionId)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    executionCtx->SetTaskpoolSubmissionId(submissionId);
}

extern "C" EtsInt GenerateGroupId()
{
    return g_taskGroupId++;
}

extern "C" EtsInt GenerateSeqRunnerId()
{
    return g_seqRunnerId++;
}

extern "C" EtsInt GenerateAsyncRunnerId()
{
    return g_asyncRunnerId++;
}

extern "C" EtsBoolean IsUsingLaunchMode()
{
    const auto &taskpoolMode =
        Runtime::GetOptions().GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));

    bool res = (taskpoolMode == TASKPOOL_LAUNCH_MODE);
    return ark::ets::ToEtsBoolean(res);
}

extern "C" EtsBoolean IsSupportingInterop()
{
    bool res = false;
#ifdef PANDA_ETS_INTEROP_JS
    res = Runtime::GetOptions().IsTaskpoolSupportInterop(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));
#endif /* PANDA_ETS_INTEROP_JS */
    return ark::ets::ToEtsBoolean(res);
}

extern "C" void SetCurrentTaskpoolWorkerPriority(EtsInt priority)
{
    [[maybe_unused]] auto result = QosHelper::SetCurrentWorkerPriority(static_cast<::Priority>(priority));
    LOG(DEBUG, RUNTIME) << "QosHelper::SetCurrentWorkerPriority priority=" << priority << ", result=" << result;
}

extern "C" EtsInt GetHardwareTaskPoolWorkersLimit()
{
    const uint32_t cpuCount = std::thread::hardware_concurrency();
    return static_cast<EtsInt>(cpuCount > 1 ? cpuCount - 1 : 1);  // 1: default number
}

extern "C" EtsInt GetDefaultTaskPoolWorkersLimit()
{
    return static_cast<EtsInt>(GetDefaultTaskPoolEWorkersLimit(Runtime::GetOptions()));
}

extern "C" EtsBoolean CurrentWorkerHasPendingLocalJobs()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *worker = static_cast<CoroutineWorker *>(executionCtx->GetWorker());
    ASSERT(worker != nullptr);
    return ark::ets::ToEtsBoolean(worker->HasPendingLocalJobs());
}

extern "C" EtsLong CollectAsyncStack()
{
    auto *asyncStackHelper = GetAsyncStackHelper();
    if (asyncStackHelper == nullptr) {
        return 0;
    }
    return static_cast<EtsLong>(asyncStackHelper->CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH));
}

extern "C" EtsLong GetSubmitterStackId()
{
    auto *asyncStackHelper = GetAsyncStackHelper();
    if (asyncStackHelper == nullptr) {
        return 0;
    }
    return static_cast<EtsLong>(asyncStackHelper->GetStackId());
}

extern "C" void SetSubmitterStackId(EtsLong stackId)
{
    auto *asyncStackHelper = GetAsyncStackHelper();
    if (asyncStackHelper == nullptr) {
        return;
    }
    asyncStackHelper->SetStackId(static_cast<uint64_t>(stackId));
}

}  // namespace ark::ets::intrinsics::taskpool
