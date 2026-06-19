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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_RESOURCES_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_RESOURCES_H

#include "common_components/heap/collector/task_queue.h"
#include "common_components/taskpool/taskpool.h"

namespace ark::common_vm {
class CollectorProxy;
// CollectorResources provides the resources that a functional collector need,
// such as gc thread/threadPool, gc task queue...
class CollectorResources {
public:
    void SetCollector(Collector *collector)
    {
        collector_ = collector;
    }
    void Init();
    void Fini();
    void StopGCWork();
    void WaitForGCFinish();

    // Returns the number of completed GC cycles (both sync and async)
    uint64_t GetGcCompletedCount() const
    {
        // Atomic with acquire order reason: acquire/release atomic flags ensures
        // that after the new counter value is read all memory changes made before
        // the counter update in another thread would be actual and visible now
        return gcCompletedCount_.load(std::memory_order_acquire);
    }

    uint32_t GetGCThreadCount(const bool isConcurrent) const;

    Taskpool *GetThreadPool() const
    {
        return gcThreadPool_;
    }

    bool IsHeapMarked() const
    {
        return isHeapMarked_;
    }

    void SetHeapMarked(bool value)
    {
        isHeapMarked_ = value;
    }

    bool IsGcStarted() const
    {
        // Atomic with relaxed order reason: data race with isGcStarted_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        return isGcStarted_.load(std::memory_order_relaxed);
    }

    void SetGcStarted(bool val)
    {
        // Atomic with relaxed order reason: data race with isGcStarted_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        isGcStarted_.store(val, std::memory_order_relaxed);
    }

    bool IsNativeGCInvoked() const
    {
        // Atomic with relaxed order reason: data race with isNativeGCInvoked_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        return isNativeGCInvoked_.load(std::memory_order_relaxed);
    }

    void SetIsNativeGCInvoked(bool val)
    {
        // Atomic with relaxed order reason: data race with isNativeGCInvoked_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        isNativeGCInvoked_.store(val, std::memory_order_relaxed);
    }

    bool IsGCActive() const
    {
        return Heap::GetHeap().IsGCEnabled();
    }

    void BroadcastGCFinished();
    GCStats &GetGCStats()
    {
        return gcStats_;
    }
    const GCStats &GetGCStats() const
    {
        return gcStats_;
    }

    void StartRuntimeThreads();
    void StopRuntimeThreads();

    void MarkGCStart();
    void MarkGCFinish(uint64_t gcIndex = 0);

private:
    // Notify that GC has finished.
    void NotifyGCFinished(uint64_t gcIndex);

    void StartGCThreads();
    void TerminateGCTask();
    void StopGCThreads();

    // the thread pool for parallel tracing.
    Taskpool *gcThreadPool_ = nullptr;
    uint32_t gcThreadCount_ = 1;
    GCTaskQueue<GCRunner> *taskQueue_ = nullptr;

    std::atomic<bool> gcThreadRunning_ = {false};
    // finishedGcIndex records the currently finished gcIndex
    // may be read by mutator but only be written by gc thread sequentially
    std::atomic<uint64_t> finishedGcIndex_ = {0};
    // Counts ALL completed GC cycles (both sync and async), used by StdGCWaitForFinishGC
    std::atomic<uint64_t> gcCompletedCount_ = {0};
    // protect condition_variable gcFinishedCondVar's status.
    ark::os::memory::Mutex gcFinishedCondMutex_;
    // notified when GC finished, requires gcFinishedCondMutex
    ark::os::memory::ConditionVariable gcFinishedCondVar_;

    // Indicate whether GC is already started.
    // NOTE: When GC finishes, it clears isGcStarted, must be over-written only by gc thread.
    std::atomic<bool> isGcStarted_ = {false};
    std::atomic<bool> isNativeGCInvoked_ = {false};

    // only gc thread can access it, so we don't use atomic type
    bool isHeapMarked_ = false;
    int gcWorking_ = 0;
#if defined(_WIN64) || defined(__APPLE__)
    ark::os::memory::ConditionVariable gcWorkingCV;
    ark::os::memory::Mutex gcWorkingMtx;
    __attribute__((always_inline)) inline void WaitUntilGCDone()
    {
        ark::os::memory::LockHolder gcWorkingLck(gcWorkingMtx);
        gcWorkingCV.Wait(&gcWorkingMtx);
    }

    __attribute__((always_inline)) inline void WakeWhenGCDone()
    {
        ark::os::memory::LockHolder gcWorkingLck(gcWorkingMtx);
        gcWorkingCV.SignalAll();
    }
#endif
    Collector *collector_;
    GCStats gcStats_;
    bool hasRelease = false;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_COLLECTOR_RESOURCES_H
