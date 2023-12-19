/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_INTERFACE_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_INTERFACE_H

#include "libpandabase/taskmanager/task.h"
#include "libpandabase/os/mutex.h"
#include <atomic>
#include <queue>

namespace ark::taskmanager {

class TaskQueueId {
public:
    constexpr TaskQueueId() : TaskQueueId(TaskType::UNKNOWN, VMType::UNKNOWN) {}
    constexpr TaskQueueId(TaskType tt, VMType vt)
        : val_(static_cast<uint16_t>(tt) |
               static_cast<uint16_t>(static_cast<uint16_t>(vt) << (BITS_PER_BYTE * sizeof(TaskType))))
    {
        static_assert(sizeof(TaskType) == sizeof(TaskQueueId) / 2);
        static_assert(sizeof(VMType) == sizeof(TaskQueueId) / 2);
    }

    friend constexpr bool operator==(const TaskQueueId &lv, const TaskQueueId &rv)
    {
        return lv.val_ == rv.val_;
    }

    friend constexpr bool operator!=(const TaskQueueId &lv, const TaskQueueId &rv)
    {
        return lv.val_ != rv.val_;
    }

    friend constexpr bool operator<(const TaskQueueId &lv, const TaskQueueId &rv)
    {
        return lv.val_ < rv.val_;
    }

private:
    uint16_t val_;
};

constexpr TaskQueueId INVALID_TASKQUEUE_ID = TaskQueueId();

/**
 * @brief TaskQueueInteface is an interface of push-thread-safe queue for tasks. Queues can be registered in
 * TaskScheduler and used to execute tasks on workers. Also, queues can notify other threads when a new task is pushed.
 */
class TaskQueueInterface {
public:
    NO_COPY_SEMANTIC(TaskQueueInterface);
    NO_MOVE_SEMANTIC(TaskQueueInterface);

    static constexpr uint8_t MAX_PRIORITY = 10;
    static constexpr uint8_t MIN_PRIORITY = 1;
    static constexpr uint8_t DEFAULT_PRIORITY = 5;

    PANDA_PUBLIC_API TaskQueueInterface(TaskType taskType, VMType vmType, uint8_t priority)
        : taskType_(taskType), vmType_(vmType), priority_(priority)
    {
        ASSERT(priority >= MIN_PRIORITY);
        ASSERT(priority <= MAX_PRIORITY);
    }
    PANDA_PUBLIC_API virtual ~TaskQueueInterface() = default;

    /**
     * @brief Adds task in task queue. Operation is thread-safe.
     * @param task - task that will be added
     * @return the size of queue after @arg task was added to it.
     */
    PANDA_PUBLIC_API virtual size_t AddTask(Task &&task) = 0;

    [[nodiscard]] PANDA_PUBLIC_API virtual bool IsEmpty() const = 0;
    [[nodiscard]] PANDA_PUBLIC_API virtual size_t Size() const = 0;

    /**
     * @brief Method @returns true if queue does not have queue with specified execution mode
     * @param mode - execution mode of tasks
     */
    [[nodiscard]] PANDA_PUBLIC_API virtual bool HasTaskWithExecutionMode(TaskExecutionMode mode) const = 0;

    [[nodiscard]] PANDA_PUBLIC_API virtual size_t CountOfTasksWithExecutionMode(TaskExecutionMode mode) const = 0;

    uint8_t GetPriority() const
    {
        // Atomic with acquire order reason: data race with priority_ with dependencies on reads after the
        // load which should become visible
        return priority_.load(std::memory_order_acquire);
    }

    void SetPriority(uint8_t priority)
    {
        ASSERT(priority >= MIN_PRIORITY);
        ASSERT(priority <= MAX_PRIORITY);
        // Atomic with release order reason: data race with priority_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        priority_.store(priority, std::memory_order_release);
    }

    TaskType GetTaskType() const
    {
        return taskType_;
    }

    VMType GetVMType() const
    {
        return vmType_;
    }

private:
    TaskType taskType_;
    VMType vmType_;
    std::atomic_uint8_t priority_;
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_INTERFACE_H
