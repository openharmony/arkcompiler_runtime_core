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

#include "common_components/heap/collector/heuristic_gc_policy.h"

#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/heap.h"
#include "common_interfaces/base_runtime.h"

namespace ark::common_vm {
std::atomic<StartupStatus> StartupStatusManager::startupStatus_ = StartupStatus::BEFORE_STARTUP;

void StartupStatusManager::OnAppStartup()
{
    // Atomic with relaxed order reason: data race with startupStatus_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    startupStatus_.store(StartupStatus::COLD_STARTUP, std::memory_order_relaxed);
    auto *threadPool = Taskpool::GetCurrentTaskpool().get();
    threadPool->PostDelayedTask(MakePandaUnique<StartupTask>(0, threadPool, STARTUP_DURATION_MS), STARTUP_DURATION_MS);
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "SmartGC: app startup just finished, CMC FinishGCRestrainTask create", "");
}

void HeuristicGCPolicy::Init()
{
    const HeapParam &heapParam = Heap::GetHeap().GetHeapParam();
    heapSize_ = heapParam.heapSize * KB;
#ifndef PANDA_TARGET_32
    // 2: only half heapSize used allocate
    heapSize_ = heapSize_ / 2;
#endif
}

bool HeuristicGCPolicy::ShouldRestrainGCOnStartupOrSensitive()
{
    // Startup
    size_t allocated = Heap::GetHeap().GetAllocator().GetAllocatedBytes();
    StartupStatus currentStatus = StartupStatusManager::GetStartupStatus();
    if (UNLIKELY(currentStatus == StartupStatus::COLD_STARTUP &&
                 allocated < heapSize_ * COLD_STARTUP_PHASE1_GC_THRESHOLD_RATIO)) {
        return true;
    }
    if (currentStatus == StartupStatus::COLD_STARTUP_PARTIALLY_FINISH &&
        allocated < heapSize_ * COLD_STARTUP_PHASE2_GC_THRESHOLD_RATIO) {
        return true;
    }
    // Sensitive
    return ShouldRestrainGCInSensitive(allocated);
}

StartupStatus HeuristicGCPolicy::GetStartupStatus() const
{
    return StartupStatusManager::GetStartupStatus();
}

void HeuristicGCPolicy::TryHeuristicGC()
{
    if (UNLIKELY(ShouldRestrainGCOnStartupOrSensitive())) {
        return;
    }

    Collector &collector = Heap::GetHeap().GetCollector();
    size_t threshold = collector.GetGCStats().GetThreshold();
    size_t allocated = Heap::GetHeap().GetAllocator().GetAllocatedBytes();
    if (allocated >= threshold) {
        if (collector.GetGCStats().shouldRequestYoung) {
            LOG(DEBUG, GC) << "request heu gc: young " << allocated << ", threshold " << threshold;
            collector.RequestGC(GC_REASON_YOUNG, true, GC_TYPE_YOUNG);
        } else {
            LOG(DEBUG, GC) << "request heu gc: allocated " << allocated << ", threshold " << threshold;
            collector.RequestGC(GC_REASON_HEU, true, GC_TYPE_FULL);
        }
    }
}

void HeuristicGCPolicy::TryIdleGC()
{
    if (UNLIKELY(ShouldRestrainGCOnStartupOrSensitive())) {
        return;
    }

    if (aliveSizeAfterGC_ == 0) {
        return;
    }
    size_t allocated = Heap::GetHeap().GetAllocator().GetAllocatedBytes();
    size_t expectHeapSize = std::max(static_cast<size_t>(aliveSizeAfterGC_ * IDLE_SPACE_SIZE_MIN_INC_RATIO),
                                     aliveSizeAfterGC_ + IDLE_SPACE_SIZE_MIN_INC_STEP_FULL);
    if (allocated >= expectHeapSize) {
        LOG(DEBUG, GC) << "request idle gc: allocated " << allocated << ", expectHeapSize " << expectHeapSize
                       << ", aliveSizeAfterGC " << aliveSizeAfterGC_;
        Heap::GetHeap().GetCollector().RequestGC(GC_REASON_IDLE, true, GC_TYPE_FULL);
    }
}

bool HeuristicGCPolicy::ShouldRestrainGCInSensitive(size_t currentSize)
{
    AppSensitiveStatus current = GetSensitiveStatus();
    switch (current) {
        case AppSensitiveStatus::NORMAL_SCENE:
            return false;
        case AppSensitiveStatus::ENTER_HIGH_SENSITIVE: {
            if (GetRecordHeapObjectSizeBeforeSensitive() == 0) {
                SetRecordHeapObjectSizeBeforeSensitive(currentSize);
            }
            if (Heap::GetHeap().GetCollector().GetGCStats().shouldRequestYoung) {
                return false;
            }
            if (currentSize < (GetRecordHeapObjectSizeBeforeSensitive() + INC_OBJ_SIZE_IN_SENSITIVE)) {
                return true;
            }
            return false;
        }
        case AppSensitiveStatus::EXIT_HIGH_SENSITIVE: {
            if (CASSensitiveStatus(current, AppSensitiveStatus::NORMAL_SCENE)) {
                // Set record heap obj size 0 after exit high senstive
                SetRecordHeapObjectSizeBeforeSensitive(0);
            }
            return false;
        }
        default:
            return false;
    }
}

void HeuristicGCPolicy::NotifyNativeAllocation(size_t bytes)
{
    // Atomic with relaxed order reason: data race with notifiedNativeSize_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    notifiedNativeSize_.fetch_add(bytes, std::memory_order_relaxed);
    // Atomic with relaxed order reason: data race with nativeHeapObjects_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    size_t currentObjects = nativeHeapObjects_.fetch_add(1, std::memory_order_relaxed);
    if (currentObjects % NOTIFY_NATIVE_INTERVAL == NOTIFY_NATIVE_INTERVAL - 1 || bytes > NATIVE_IMMEDIATE_THRESHOLD) {
        CheckGCForNative();
    }
}
void HeuristicGCPolicy::CheckGCForNative()
{
    // Atomic with relaxed order reason: data race with notifiedNativeSize_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    size_t currentNativeSize = notifiedNativeSize_.load(std::memory_order_relaxed);
    // Atomic with relaxed order reason: data race with nativeHeapThreshold_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    size_t currentThreshold = nativeHeapThreshold_.load(std::memory_order_relaxed);
    if (currentNativeSize > currentThreshold) {
        if (currentNativeSize > URGENCY_NATIVE_LIMIT) {
            // Native binding size is too large, should wait a sync finished.
            Heap::GetHeap().GetCollector().RequestGC(GC_REASON_NATIVE_SYNC, false, GC_TYPE_FULL);
            return;
        }
        Heap::GetHeap().GetCollector().RequestGC(GC_REASON_NATIVE, true, GC_TYPE_FULL);
    }
}
void HeuristicGCPolicy::NotifyNativeFree(size_t bytes)
{
    size_t allocated;
    size_t newFreedBytes;
    do {
        // Atomic with relaxed order reason: data race with notifiedNativeSize_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        allocated = notifiedNativeSize_.load(std::memory_order_relaxed);
        newFreedBytes = std::min(allocated, bytes);
        // We should not be registering more free than allocated bytes.
        // But correctly keep going in non-debug builds.
        DCHECK(newFreedBytes == bytes);
        // Atomic with relaxed order reason: data race with notifiedNativeSize_ with no synchronization or ordering
        // constraints imposed on other reads or writes
    } while (
        !notifiedNativeSize_.compare_exchange_weak(allocated, allocated - newFreedBytes, std::memory_order_relaxed));
}

void HeuristicGCPolicy::NotifyNativeReset(size_t oldBytes, size_t newBytes)
{
    NotifyNativeFree(oldBytes);
    NotifyNativeAllocation(newBytes);
}

size_t HeuristicGCPolicy::GetNotifiedNativeSize() const
{
    // Atomic with relaxed order reason: data race with notifiedNativeSize_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    return notifiedNativeSize_.load(std::memory_order_relaxed);
}

void HeuristicGCPolicy::SetNativeHeapThreshold(size_t newThreshold)
{
    // Atomic with relaxed order reason: data race with nativeHeapThreshold_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    nativeHeapThreshold_.store(newThreshold, std::memory_order_relaxed);
}

size_t HeuristicGCPolicy::GetNativeHeapThreshold() const
{
    // Atomic with relaxed order reason: data race with nativeHeapThreshold_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    return nativeHeapThreshold_.load(std::memory_order_relaxed);
}

void HeuristicGCPolicy::RecordAliveSizeAfterLastGC(size_t aliveBytes)
{
    aliveSizeAfterGC_ = aliveBytes;
}

void HeuristicGCPolicy::ChangeGCParams(bool isBackground)
{
    if (isBackground) {
        size_t allocated = Heap::GetHeap().GetAllocator().GetAllocatedBytes();
        if (allocated > aliveSizeAfterGC_ && (allocated - aliveSizeAfterGC_) > BACKGROUND_LIMIT &&
            allocated > MIN_BACKGROUND_GC_SIZE) {
            Heap::GetHeap().GetCollector().RequestGC(GC_REASON_BACKGROUND, true, GC_TYPE_FULL);
        }
        common_vm::Taskpool::GetCurrentTaskpool()->SetThreadPriority(common_vm::PriorityMode::BACKGROUND);
        Heap::GetHeap().GetGCParam().multiplier = 1;
    } else {
        Taskpool::GetCurrentTaskpool()->SetThreadPriority(PriorityMode::FOREGROUND);
        // 3: The front-end application waterline is 3 times
        Heap::GetHeap().GetGCParam().multiplier = 3;
    }
}

bool HeuristicGCPolicy::CheckAndTriggerHintGC(MemoryReduceDegree degree)
{
    if (UNLIKELY(ShouldRestrainGCOnStartupOrSensitive())) {
        return false;
    }
    size_t allocated = Heap::GetHeap().GetAllocator().GetAllocatedBytes();

    size_t stepAfterLastGC = 0;
    if (degree == MemoryReduceDegree::LOW) {
        stepAfterLastGC = LOW_DEGREE_STEP_IN_IDLE;
    } else {
        stepAfterLastGC = HIGH_DEGREE_STEP_IN_IDLE;
    }
    if (aliveSizeAfterGC_ == 0) {
        return false;
    }
    size_t expectHeapSize =
        std::max(static_cast<size_t>(aliveSizeAfterGC_ * IDLE_MIN_INC_RATIO), aliveSizeAfterGC_ + stepAfterLastGC);
    if (expectHeapSize < allocated) {
        LOG(DEBUG, GC) << "request heu gc by hint: allocated " << allocated << ", expectHeapSize " << expectHeapSize
                       << ", aliveSizeAfterGC " << aliveSizeAfterGC_;
        Heap::GetHeap().GetCollector().RequestGC(GC_REASON_HINT, true, GC_TYPE_FULL);
        return true;
    }
    return false;
}
}  // namespace ark::common_vm
