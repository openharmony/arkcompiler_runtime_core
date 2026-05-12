/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "common_components/heap/collector/collector_resources.h"

#include "common_components/base/time_utils.h"
#include "common_components/base/sys_call.h"
#include "common_components/common/run_type.h"
#include "common_components/common/scoped_object_access.h"

#include "include/mutator_status.h"
#include "libarkbase/utils/logger.h"

#ifdef ENABLE_QOS
#include "qos.h"
#endif

#include <thread>

namespace ark::common_vm {

void *CollectorResources::GCMainThreadEntry(void *arg)
{
#ifdef __APPLE__
    int ret = pthread_setname_np("OS_GC_Thread");
    LOG_IF(UNLIKELY(ret != 0), ERROR, GC)
        << "pthread setname in CollectorResources::StartGCThreads() return " << ret << " rather than 0";
#elif defined(__linux__) || defined(PANDA_TARGET_OHOS)
    int ret = prctl(PR_SET_NAME, "OS_GC_Thread");
    LOG_IF(UNLIKELY(ret != 0), ERROR, GC)
        << "pthread setname in CollectorResources::StartGCThreads() return " << ret << " rather than 0";
#endif

    ASSERT_PRINT(arg != nullptr, "GCMainThreadEntry arg=nullptr");
    // set current thread as a gc thread.
    ThreadLocal::SetThreadType(ThreadType::GC_THREAD);

    LOG(DEBUG, GC) << "CollectorResources Thread begin.";

#ifdef ENABLE_QOS
    OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INITIATED, GetTid());
#endif

    // run event loop in this thread.
    CollectorResources *collectorResources = reinterpret_cast<CollectorResources *>(arg);
    collectorResources->RunTaskLoop();

    LOG(DEBUG, GC) << "CollectorResources Thread end.";
    return nullptr;
}

void CollectorResources::Init()
{
    taskQueue_ = new GCTaskQueue<GCRunner>;
    taskQueue_->Init();
    // Atomic with seq_cst order reason: initialization of finishedGcIndex_ with requirement for sequentially
    // consistent order where threads observe all modifications in the same order
    finishedGcIndex_.store(0, std::memory_order_seq_cst);
    gcCompletedCount_.store(0, std::memory_order_seq_cst);
    StartGCThreads();
    gcStats_.Init();
    hasRelease = false;
}

void CollectorResources::Fini()
{
    if (hasRelease == false) {
        StopGCWork();
        // Atomic with relaxed order reason: data race with gcThreadRunning_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        ASSERT_PRINT(!gcThreadRunning_.load(std::memory_order_relaxed), "Invalid GC thread status");
        taskQueue_->Finish();
        delete taskQueue_;
        taskQueue_ = nullptr;
        hasRelease = true;
    }
}

void CollectorResources::StopGCWork()
{
    TerminateGCTask();
    StopGCThreads();
}

void CollectorResources::StartRuntimeThreads()
{
    // For Postfork.
    Init();
}

void CollectorResources::StopRuntimeThreads()
{
    // For Prefork.
    Fini();
}

// Send terminate task to gc thread.
void CollectorResources::TerminateGCTask()
{
    // Atomic with acquire order reason: data race with gcThreadRunning_ with dependecies on reads after the load
    if (gcThreadRunning_.load(std::memory_order_acquire) == false) {
        return;
    }

    GCTaskQueue<GCRunner>::GCTaskFilter filter = [](GCRunner &, GCRunner &) { return false; };
    GCRunner task(GCTask::GCTaskType::GC_TASK_TERMINATE_GC);
    (void)taskQueue_->EnqueueSync(task, filter);  // enqueue to sync queue
}

// Usually called from main thread, wait for collector thread to exit.
void CollectorResources::StopGCThreads()
{
    // Atomic with acquire order reason: data race with gcThreadRunning_ with dependecies on reads after the load
    if (gcThreadRunning_.load(std::memory_order_acquire) == false) {
        // NOTE(ivagin): #33823 Introduce "IsGCRunning" interface for common_runtime
        // and insert log fatal + unreachable here
        return;
    }
    {
        // Enter saferegion to avoid blocking gc stw
        ark::Mutator *mutator = ark::Mutator::GetCurrent();
        if (mutator != nullptr) {
            mutator->UpdateStatus(MutatorStatus::NATIVE);
        }
        int ret = ::pthread_join(gcMainThread_, nullptr);
        LOG_IF(UNLIKELY(ret != 0), ERROR, GC) << "::pthread_join() in StopGCThreads() return " << ret;
    }
    // wait the thread pool stopped.
    if (gcThreadPool_ != nullptr) {
        gcThreadPool_->Destroy(0);
        gcThreadPool_ = nullptr;
        Taskpool::GetCurrentTaskpool().reset();
    }
    // Atomic with release order reason: data race with gcThreadRunning_ with dependecies on writes before the store
    gcThreadRunning_.store(false, std::memory_order_release);
}

void CollectorResources::RunTaskLoop()
{
    // Atomic with release order reason: data race with gcTid_ with dependecies on writes before the store
    gcTid_.store(GetTid(), std::memory_order_release);
    taskQueue_->DrainTaskQueue(collector_);
    NotifyGCFinished(GCTask::TASK_INDEX_GC_EXIT);
}

// For the ignored gc request, check whether need to wait for current gc finish
void CollectorResources::PostIgnoredGcRequest(GCTaskCause reason)
{
    GCRequest &request = g_gcRequests[GCRequestIndex(reason)];
    // Atomic with seq_cst order reason: data race with isGcStarted_ with requirement for sequentially consistent
    // order where threads observe all modifications in the same order
    if (request.IsSyncGC() && isGcStarted_.load(std::memory_order_seq_cst)) {
        ScopedEnterSaferegion safeRegion(false);
        WaitForGCFinish();
    }
}

void CollectorResources::RequestAsyncGC(GCTaskCause reason, GCCollectionType gcType)
{
    // The gc request must be none blocked
    ASSERT_PRINT(!g_gcRequests[GCRequestIndex(reason)].IsSyncGC(), "trigger from unsafe context must be none blocked");
    GCRunner gcTask(GCTask::GCTaskType::GC_TASK_INVOKE_GC, reason, gcType);
    // we use async enqueue because this doesn't have locks, lowering the risk
    // of timeouts when entering safe region due to thread scheduling
    taskQueue_->EnqueueAsync(gcTask);
}

void CollectorResources::RequestGCAndWait(GCTaskCause reason, GCCollectionType gcType)
{
    // Enter saferegion since current thread may blocked by locks.
    ScopedEnterSaferegion enterSaferegion(false);
    GCRunner gcTask(GCTask::GCTaskType::GC_TASK_INVOKE_GC, reason, gcType);

    GCTaskQueue<GCRunner>::GCTaskFilter filter = [](GCRunner &oldTask, GCRunner &newTask) {
        return oldTask.GetGCReason() == newTask.GetGCReason();
    };

    GCRequest &request = g_gcRequests[GCRequestIndex(reason)];
    // If this gcTask need not to block, just add to async queue
    if (!request.IsSyncGC()) {
        taskQueue_->EnqueueAsync(gcTask);
        return;
    }

    // If this gcTask need to block,
    // add gcTask to syncTaskQueue of gcTaskQueue and wait until this gcTask finished
    ark::os::memory::LockHolder lock(gcFinishedCondMutex_);
    uint64_t curThreadSyncIndex = taskQueue_->EnqueueSync(gcTask, filter);
    // wait until GC finished
    std::function<bool()> pred = [this, curThreadSyncIndex] {
        // Atomic with acquire order reason: data race with finishedGcIndex_ with dependecies on reads after the load
        // to observe GC completion state
        uint64_t idx = finishedGcIndex_.load(std::memory_order_acquire);
        return ((idx >= curThreadSyncIndex) || (idx == GCTask::TASK_INDEX_GC_EXIT));
    };

    while (!pred()) {
        gcFinishedCondVar_.Wait(&gcFinishedCondMutex_);
    }
}

void CollectorResources::RequestGC(GCTaskCause reason, bool async, GCCollectionType gcType, bool explicitRequest)
{
    if (!IsGCActive()) {
        return;
    }

    GCRequest &request = g_gcRequests[GCRequestIndex(reason)];
    uint64_t curTime = TimeUtil::NanoSeconds();
    request.SetPrevRequestTime(curTime);
    if ((!explicitRequest && collector_->ShouldIgnoreRequest(request)) ||
        (reason == GCTaskCause::NATIVE_ALLOC_CAUSE && IsNativeGCInvoked())) {
        LOG(DEBUG, GC) << "ignore gc request";
        PostIgnoredGcRequest(reason);
    } else if (async) {
        if (reason == GCTaskCause::NATIVE_ALLOC_CAUSE) {
            SetIsNativeGCInvoked(true);
        }
        RequestAsyncGC(reason, gcType);
    } else {
        RequestGCAndWait(reason, gcType);
    }
}

void CollectorResources::NotifyGCFinished(uint64_t gcIndex)
{
    ark::os::memory::LockHolder lock(gcFinishedCondMutex_);
    // Atomic with relaxed order reason: data race with isGcStarted_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    isGcStarted_.store(false, std::memory_order_relaxed);
    // Atomic with release order reason: acquire/release atomic flags ensures
    // that after the new counter value is read all memory changes made before
    // the counter update in another thread would be actual and visible now
    gcCompletedCount_.fetch_add(1U, std::memory_order_release);
    if (gcIndex >= GCTask::TASK_INDEX_SYNC_GC_MIN) {  // sync gc, need set taskIndex
        // Atomic with release order reason: data race with finishedGcIndex_ with dependecies on writes before
        // the store which should become visible to waiting mutators
        finishedGcIndex_.store(gcIndex, std::memory_order_release);
    }
    gcFinishedCondVar_.SignalAll();
    BroadcastGCFinished();
}

void CollectorResources::MarkGCStart()
{
    ark::os::memory::LockHolder lock(gcFinishedCondMutex_);

    // Wait for any existing GC to finish - inline the wait logic
    std::function<bool()> pred = [this] { return !IsGcStarted(); };
    while (!pred()) {
        gcFinishedCondVar_.Wait(&gcFinishedCondMutex_);
    }

    // Now claim GC ownership
    SetGcStarted(true);
}

void CollectorResources::MarkGCFinish(uint64_t gcIndex)
{
    NotifyGCFinished(gcIndex);
}

void CollectorResources::WaitForGCCompletionCount(uint64_t targetCount)
{
    ark::os::memory::LockHolder lock(gcFinishedCondMutex_);
    std::function<bool()> pred = [this, targetCount] {
        // Atomic with relaxed order reason: gcCompletedCount_ only needs uniqueness, not synchronization with other
        return gcCompletedCount_.load(std::memory_order_relaxed) >= targetCount ||
               finishedGcIndex_ == GCTask::TASK_INDEX_GC_EXIT;
    };
    while (!pred()) {
        gcFinishedCondVar_.Wait(&gcFinishedCondMutex_);
    }
}

void CollectorResources::WaitForGCFinish()
{
    uint64_t startTime = TimeUtil::MicroSeconds();
    ark::os::memory::LockHolder lock(gcFinishedCondMutex_);
    // Atomic with acquire order reason: data race with finishedGcIndex_ with dependecies on reads after the load
    // to observe GC completion state
    uint64_t curWaitGcIndex = finishedGcIndex_.load(std::memory_order_acquire);
    std::function<bool()> pred = [this, curWaitGcIndex] {
        // Atomic with acquire order reason: data race with finishedGcIndex_ with dependecies on reads after the load
        // to observe GC completion state
        uint64_t idx = finishedGcIndex_.load(std::memory_order_acquire);
        return (!IsGcStarted() || (curWaitGcIndex != idx) || (idx == GCTask::TASK_INDEX_GC_EXIT));
    };
    while (!pred()) {
        gcFinishedCondVar_.Wait(&gcFinishedCondMutex_);
    }
    uint64_t stopTime = TimeUtil::MicroSeconds();
    uint64_t diffTime = stopTime - startTime;
    LOG(DEBUG, GC) << "WaitForGCFinish cost " << diffTime << " us";
}

void CollectorResources::StartGCThreads()
{
    bool expected = false;
    // Atomic with acquire order reason: data race with gcThreadRunning_ with dependecies on reads after the load
    if (gcThreadRunning_.compare_exchange_strong(expected, true, std::memory_order_acquire) == false) {
        LOG(FATAL, COMMON) << "[GC] CollectorResources Thread already begin.";
        UNREACHABLE();
    }
    DCHECK(gcThreadPool_ == nullptr);
    Taskpool::CreateInstance();
    gcThreadPool_ = Taskpool::GetCurrentTaskpool().get();
    gcThreadPool_->Initialize();
    LOG_IF(UNLIKELY(gcThreadPool_ == nullptr), FATAL, GC) << "new GCThreadPool failed";
    uint32_t helperThreads = gcThreadPool_->GetTotalThreadNum();
    if (helperThreads > 0) {
        --helperThreads;  // gc task is exclusive, so keep one thread left
    }
    // 1 is for gc main thread.
    gcThreadCount_ = helperThreads + 1;

    LOG(DEBUG, GC) << "total gc thread count " << gcThreadCount_ << ", helper thread count " << helperThreads;
    // create the collector thread.
    if (::pthread_create(&gcMainThread_, nullptr, CollectorResources::GCMainThreadEntry, this) != 0) {
        ASSERT_PRINT(0, "pthread_create failed!");
    }
    // set thread name.
#ifdef __WIN64
    int ret = pthread_setname_np(gcMainThread_, "OS_GC_Thread");
    LOG_IF(UNLIKELY(ret != 0), ERROR, GC)
        << "pthread_setname_np() in CollectorResources::StartGCThreads() return " << ret << " rather than 0";
#endif
}

uint32_t CollectorResources::GetGCThreadCount(const bool isConcurrent) const
{
    if (GetThreadPool() == nullptr) {
        return 1;
    } else if (isConcurrent) {
        return gcThreadCount_;
    }
    // default to 2
    return 2;
}

void CollectorResources::BroadcastGCFinished()
{
    gcWorking_ = 0;
#if defined(_WIN64) || defined(__APPLE__)
    WakeWhenGCDone();
#else
    (void)Futex(&gcWorking_, FUTEX_WAKE_PRIVATE, INT_MAX);
#endif
}

void CollectorResources::RequestHeapDump(GCTask::GCTaskType gcTask)
{
    GCTaskQueue<GCRunner>::GCTaskFilter filter = [](GCRunner &, GCRunner &) { return false; };
    GCRunner dumpTask = GCRunner(gcTask);
    taskQueue_->EnqueueSync(dumpTask, filter);
}

}  // namespace ark::common_vm
