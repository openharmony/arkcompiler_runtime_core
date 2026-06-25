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
    // wait the thread pool stopped.
    if (gcThreadPool_ != nullptr) {
        gcThreadPool_->Destroy(0);
        gcThreadPool_ = nullptr;
        Taskpool::GetCurrentTaskpool().reset();
    }
    // Atomic with release order reason: data race with gcThreadRunning_ with dependecies on writes before the store
    gcThreadRunning_.store(false, std::memory_order_release);
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
    // Now claim GC ownership
    SetGcStarted(true);
}

void CollectorResources::MarkGCFinish(uint64_t gcIndex)
{
    NotifyGCFinished(gcIndex);
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

    gcThreadCount_ = helperThreads;

    LOG(DEBUG, GC) << "total thread count " << gcThreadCount_;
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

}  // namespace ark::common_vm
