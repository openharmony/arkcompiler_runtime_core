/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_WORKER_THREAD_LOCAL_QUEUE_H
#define LIBPANDABASE_TASKMANAGER_UTILS_WORKER_THREAD_LOCAL_QUEUE_H

#include "libpandabase/taskmanager/task.h"
#include "libpandabase/taskmanager/utils/sp_mc_lock_free_queue.h"
#include "libpandabase/os/mutex.h"
#include <optional>
#include <unordered_map>

namespace ark::taskmanager::internal {

template <size_t WORKER_QUEUE_SIZE>
class WorkerThreadLocalQueue {
public:
    WorkerThreadLocalQueue()
    {
        for (TaskType taskType : ALL_TASK_TYPES) {
            for (VMType vmType : ALL_VM_TYPES) {
                for (TaskExecutionMode executionMode : ALL_TASK_EXECUTION_MODES) {
                    TaskProperties priority(taskType, vmType, executionMode);
                    perPropertiesQueue_[priority];
                }
            }
        }
    }
    ~WorkerThreadLocalQueue() = default;
    NO_COPY_SEMANTIC(WorkerThreadLocalQueue);
    NO_MOVE_SEMANTIC(WorkerThreadLocalQueue);

    size_t RegisterConsumer()
    {
        os::memory::LockHolder<os::memory::Mutex> lockHolder(registerLock_);
        size_t id = registerNumber_;
        for (TaskType taskType : ALL_TASK_TYPES) {
            for (VMType vmType : ALL_VM_TYPES) {
                for (TaskExecutionMode executionMode : ALL_TASK_EXECUTION_MODES) {
                    TaskProperties priority(taskType, vmType, executionMode);
                    [[maybe_unused]] auto idInQueue = perPropertiesQueue_.at(priority).RegisterConsumer();
                    ASSERT(id == idInQueue);
                }
            }
        }
        registerNumber_++;
        return id;
    }

    void Push(Task &&task)
    {
        auto properties = task.GetTaskProperties();
        ASSERT(!task.IsInvalid());
        perPropertiesQueue_.at(properties).Push(std::move(task));
        // Atomic with acq_rel order reason: other threads should be correct value
        size_.fetch_add(1, std::memory_order_acq_rel);
    }

    std::optional<Task> Pop(size_t id)
    {
        std::optional<Task> result = Pop(id, TaskExecutionMode::FOREGROUND);
        if (result.has_value()) {
            return result;
        }
        return Pop(id, TaskExecutionMode::BACKGROUND);
    }

    std::optional<Task> Pop(size_t id, TaskExecutionMode mode)
    {
        for (TaskType taskType : ALL_TASK_TYPES) {
            for (VMType vmType : ALL_VM_TYPES) {
                TaskProperties prop(taskType, vmType, mode);
                auto task = Pop(id, prop);
                if (task.has_value()) {
                    return task;
                }
            }
        }
        return std::nullopt;
    }

    std::optional<Task> Pop(size_t id, TaskProperties priority)
    {
        LocalTaskQueue &queue = perPropertiesQueue_.at(priority);
        auto task = queue.Pop(id);
        if (task.has_value()) {
            // Atomic with acq_rel order reason: other threads should be correct value
            size_.fetch_sub(1, std::memory_order_acq_rel);
        }
        return task;
    }

    void TryDeleteRetiredPtrs()
    {
        for (TaskType taskType : ALL_TASK_TYPES) {
            for (VMType vmType : ALL_VM_TYPES) {
                perPropertiesQueue_[{taskType, vmType, TaskExecutionMode::BACKGROUND}].TryDeleteRetiredPtrs();
                perPropertiesQueue_[{taskType, vmType, TaskExecutionMode::FOREGROUND}].TryDeleteRetiredPtrs();
            }
        }
    }

    bool IsEmpty() const
    {
        return Size() == 0;
    }

    size_t Size() const
    {
        // Atomic with acquire order reason: need to load last value
        return size_.load(std::memory_order_acquire);
    }

    size_t CountOfTasksWithProperties(TaskProperties properties) const
    {
        return perPropertiesQueue_.at(properties).Size();
    }

private:
    using LocalTaskQueue = internal::SPMCLockFreeQueue<Task, WORKER_QUEUE_SIZE>;
    std::unordered_map<TaskProperties, LocalTaskQueue, TaskProperties::Hash> perPropertiesQueue_;
    std::atomic_size_t size_ {0};

    os::memory::Mutex registerLock_;
    size_t registerNumber_ {0};
};

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_WORKER_THREAD_LOCAL_QUEUE_H
