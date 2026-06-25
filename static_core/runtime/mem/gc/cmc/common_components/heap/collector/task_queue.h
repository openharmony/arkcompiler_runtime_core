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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_TASK_QUEUE_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_TASK_QUEUE_H

#include <cstdint>
#include <list>

#include "common_components/common/page_allocator.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/heap/collector/ark_mutator_container.h"
#include "common_components/heap/heap.h"

#include "libarkbase/utils/logger.h"
#include "libarkbase/os/mutex.h"

// gc task and task queue implementation
namespace ark::common_vm {
class GCTask {
public:
    enum class GCTaskType : uint32_t {
        GC_TASK_INVALID = 0,
        GC_TASK_TERMINATE_GC = 1,   // terminate gc
        GC_TASK_INVOKE_GC = 2,      // invoke gc
        GC_TASK_DUMP_HEAP = 3,      // dump heap
        GC_TASK_DUMP_HEAP_OOM = 4,  // dump heap after oom
        GC_TASK_DUMP_HEAP_IDE = 5,  // dump heap for IDE
    };

    enum GCTaskIndex : uint64_t {
        INVALID_TASK_INDEX = 0,
        TASK_INDEX_ASYNC_GC = 1,

        // sync task index is among range [TASK_INDEX_SYNC_GC_MIN, TASK_INDEX_SYNC_GC_MAX).
        TASK_INDEX_SYNC_GC_MIN = 2,
        TASK_INDEX_SYNC_GC_MAX = std::numeric_limits<uint64_t>::max(),
        TASK_INDEX_GC_EXIT = TASK_INDEX_SYNC_GC_MAX,
    };

    explicit GCTask(GCTaskType type) : taskType_(type), taskIndex_(TASK_INDEX_ASYNC_GC) {}
    virtual ~GCTask() = default;

    GCTaskType GetTaskType() const
    {
        return taskType_;
    }

    void SetTaskType(GCTaskType type)
    {
        taskType_ = type;
    }

    GCTaskIndex GetTaskIndex() const
    {
        return taskIndex_;
    }

    void SetTaskIndex(GCTaskIndex index)
    {
        taskIndex_ = index;
    }

    virtual bool NeedFilter() const
    {
        return false;
    }

    virtual bool Execute(void *owner) = 0;

protected:
    GCTask(const GCTask &task) = default;
    virtual GCTask &operator=(const GCTask &) = default;
    GCTaskType taskType_;
    GCTaskIndex taskIndex_;
};

class GCRunner : public GCTask {
public:
    // For a task, we give it a priority based on schedule type and gc reason.
    // Termination and timeout events get highest prio, and override lower-prio tasks.
    // Each gc invocation task gets its prio relative to its reason.
    // This prio is used by the async task queue.
    static constexpr uint32_t PRIO_TERMINATE = 0;
    static constexpr uint32_t PRIO_TIMEOUT = 1;
    static constexpr uint32_t PRIO_INVOKE_GC = 2;

    static constexpr uint32_t GC_CAUSE_END = static_cast<uint32_t>(GCTaskCause::CROSSREF_CAUSE);

    static_assert(PRIO_INVOKE_GC + GC_CAUSE_END < std::numeric_limits<uint32_t>::digits,
                  "task queue reached max capacity");

    GCRunner() : GCTask(GCTaskType::GC_TASK_INVALID), gcReason_(GCTaskCause::INVALID_CAUSE) {}

    explicit GCRunner(GCTaskType type) : GCTask(type), gcReason_(GCTaskCause::INVALID_CAUSE)
    {
        ASSERT_PRINT(type != GCTaskType::GC_TASK_INVOKE_GC, "invalid gc task!");
    }

    GCRunner(GCTaskType type, GCTaskCause reason, GCCollectionType gcType = GCCollectionType::FULL)
        : GCTask(type), gcReason_(reason), gcType_(gcType)
    {
        ASSERT_PRINT(gcReason_ > GCTaskCause::INVALID_CAUSE && gcReason_ <= GCTaskCause::CROSSREF_CAUSE,
                     "invalid reason");
        ASSERT_PRINT(gcType_ >= GCCollectionType::NONE && gcType_ <= GCCollectionType::FULL, "invalid gc type");
    }

    GCRunner(const GCRunner &task) = default;
    ~GCRunner() override = default;
    GCRunner &operator=(const GCRunner &) = default;

    static inline GCRunner GetGCRunner(uint32_t prio)
    {
        if (prio == PRIO_TERMINATE) {
            return GCRunner(GCTaskType::GC_TASK_TERMINATE_GC);
        } else if (prio - PRIO_INVOKE_GC <= GC_CAUSE_END) {
            auto reason = static_cast<GCTaskCause>(prio - PRIO_INVOKE_GC);
            auto gcType = reason == GCTaskCause::YOUNG_GC_CAUSE ? GCCollectionType::YOUNG : GCCollectionType::FULL;
            return GCRunner(GCTaskType::GC_TASK_INVOKE_GC, reason, gcType);
        } else {  // LCOV_EXCL_BR_LINE
            LOG(FATAL, COMMON) << "Invalid priority in GetGCRequestByPrio function";
            UNREACHABLE();
            return GCRunner();
        }
    }

    inline uint32_t GetPriority() const
    {
        if (taskType_ == GCTaskType::GC_TASK_TERMINATE_GC) {
            return PRIO_TERMINATE;
        } else if (taskType_ == GCTaskType::GC_TASK_INVOKE_GC) {
            return PRIO_INVOKE_GC + static_cast<uint32_t>(gcReason_);
        }
        LOG(FATAL, COMMON) << "Invalid task in GetPriority function";
        UNREACHABLE();
        return 0;
    }

    static inline GCRunner GetInvalidExecutor()
    {
        return GCRunner();
    }

    inline bool IsInvalid() const
    {
        return (taskType_ == GCTaskType::GC_TASK_INVALID) && (gcReason_ == GCTaskCause::INVALID_CAUSE);
    }

    // Only for asyn gc task queues,
    // the TaskType::GC_TASK_TERMINATE_GC gc task will remove all others
    inline bool IsOverriding() const
    {
        return (taskType_ != GCTaskType::GC_TASK_INVOKE_GC);
    }

    inline GCTaskCause GetGCReason() const
    {
        return gcReason_;
    }

    inline void SetGCReason(GCTaskCause reason)
    {
        gcReason_ = reason;
    }

    inline GCCollectionType GetGCType() const
    {
        return gcType_;
    }

    inline void SetGCType(GCCollectionType type)
    {
        gcType_ = type;
    }

    bool NeedFilter() const override
    {
        return true;
    }

    bool Execute(void *owner) override;

private:
    GCTaskCause gcReason_ {GCTaskCause::INVALID_CAUSE};
    GCCollectionType gcType_ {GCCollectionType::FULL};
};

// Lockless async task queue implementation.
// This queue manages a list of deduplicated tasks.
// Each bit of the queueWord indicates the corresponding priority task.
// Lower bit indicates higher priority task.
template <typename Type>
class GCLocklessTaskQueue {
public:
    // Add one async task to asyncTaskQueue, one higher priority task might erase all lower-priority tasks in queueWord
    void Push(const Type &task)
    {
        uint32_t nextWord {0};
        bool overriding {task.IsOverriding()};
        uint32_t taskMask {(1U << task.GetPriority())};
        // Atomic with relaxed order reason: data race with queueWord_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        uint32_t curuentWord {queueWord_.load(std::memory_order_relaxed)};
        do {
            if (overriding) {
                nextWord = taskMask | ((taskMask - 1) & curuentWord);
            } else {
                nextWord = taskMask | curuentWord;
            }
            // Atomic with relaxed order reason: data race with queueWord_ with no synchronization or ordering
            // constraints imposed on other reads or writes
        } while (!queueWord_.compare_exchange_weak(curuentWord, nextWord, std::memory_order_relaxed));
    }

    // Get the highest priority task in queueWord
    // Or get one invalid task if queueWord is empty
    Type Pop()
    {
        uint32_t nextWord {0};
        // Atomic with relaxed order reason: data race with queueWord_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        uint32_t currentWord {queueWord_.load(std::memory_order_relaxed)};
        uint32_t dequeued {currentWord};
        do {
            nextWord = currentWord & (currentWord - 1);
            dequeued = currentWord;
            // Atomic with relaxed order reason: data race with queueWord_ with no synchronization or ordering
            // constraints imposed on other reads or writes
        } while (!queueWord_.compare_exchange_weak(currentWord, nextWord, std::memory_order_relaxed));

        if (currentWord == 0) {
            return Type::GetInvalidExecutor();
        }
        // get the count of trailing zeros
        return Type::GetGCRunner(__builtin_ctz(dequeued));
    }

    // When gc thread exits, clear all tasks in queueWord
    void Clear()
    {
        // Atomic with relaxed order reason: data race with queueWord_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        queueWord_.store(0, std::memory_order_relaxed);
    }

private:
    std::atomic<uint32_t> queueWord_ = {};
};

template <typename Type>
class GCTaskQueue {
    static_assert(std::is_base_of<GCTask, Type>::value, "T is not a subclass of GCTask");

public:
    using GCTaskFilter = std::function<bool(Type &oldTask, Type &newTask)>;
    using GCTaskQueueType = std::list<Type, StdContainerAllocator<Type, GC_TASK_QUEUE>>;

    void Init()
    {
        syncTaskIndex_ = GCTask::TASK_INDEX_SYNC_GC_MIN;
    }

    void Finish()
    {
        ark::os::memory::LockHolder lock(taskQueueLock_);
        asyncTaskQueue_.Clear();
        syncTaskQueue_.clear();
    }

    // Add one task to syncTaskQueue
    // Return the accumulated gc times
    uint64_t EnqueueSync(Type &task, GCTaskFilter &filter)
    {
        ark::os::memory::LockHolder lock(taskQueueLock_);
        GCTaskQueueType &queue = syncTaskQueue_;

        if (!queue.empty() && task.NeedFilter()) {
            for (auto iter = queue.rbegin(); iter != queue.rend(); ++iter) {
                if (filter(*iter, task)) {
                    return (*iter).GetTaskIndex();
                }
            }
        }
        task.SetTaskIndex(static_cast<GCTask::GCTaskIndex>(++syncTaskIndex_));
        queue.push_back(task);
        taskQueueCondVar_.SignalAll();
        return task.GetTaskIndex();
    }

    // Add one task to asyncTaskQueue
    void EnqueueAsync(const Type &task)
    {
        asyncTaskQueue_.Push(task);
        ark::os::memory::LockHolder lock(taskQueueLock_);
        taskQueueCondVar_.SignalAll();
    }

    // Retrieve a garbage collection task from the task queue
    // Prioritize synchronous tasks from syncTaskQueue before asynchronous ones from asyncTaskQueue
    Type Dequeue()
    {
        std::chrono::nanoseconds waitTime(DEFAULT_GC_TASK_INTERVAL_TIMEOUT_NS);
        while (true) {
            ark::os::memory::LockHolder lock(taskQueueLock_);
            // Prioritize synchronous task queue first
            if (!syncTaskQueue_.empty()) {
                Type currentTask(syncTaskQueue_.front());
                syncTaskQueue_.pop_front();
                return currentTask;
            }

            // Retrieve the task and then process data with dfx
            Type task = asyncTaskQueue_.Pop();
            if (task.IsInvalid()) {
                LOG(DEBUG, GC) << "invalid gc task: type " << static_cast<size_t>(task.GetTaskType()) << ", reason "
                               << task.GetGCReason();
            } else {
                LOG(DEBUG, GC) << "dequeue gc task: type " << static_cast<size_t>(task.GetTaskType()) << ", reason "
                               << task.GetGCReason();
                return task;
            }

            taskQueueCondVar_.TimedWait(&taskQueueLock_, 0, waitTime.count());
        }
    }

    // GC thread poll task queue and execute gc task
    void DrainTaskQueue(void *owner)
    {
        while (true) {
            Type task = Dequeue();
            ArkMutatorContainer mutatorContainer;
            if (!task.Execute(owner)) {
                Finish();
                break;
            }
        }
    }

private:
    static constexpr uint64_t DEFAULT_GC_TASK_INTERVAL_TIMEOUT_NS = 1000L * 1000 * 1000;  // default 1s
    ark::os::memory::RecursiveMutex taskQueueLock_;
    ark::os::memory::ConditionVariable taskQueueCondVar_;
    uint64_t syncTaskIndex_ = 0;
    GCTaskQueueType syncTaskQueue_;
    GCLocklessTaskQueue<Type> asyncTaskQueue_;
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_COLLECTOR_TASK_QUEUE_H
