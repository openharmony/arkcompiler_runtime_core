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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_H

#include "libpandabase/macros.h"
#include "libpandabase/globals.h"
#include "libpandabase/utils/time.h"
#include "libpandabase/utils/logger.h"
#include <functional>
#include <ostream>
#include <array>

namespace ark::taskmanager {

/*
 * TaskType - represents all types of components that can use TaskManager.
 * UNKNOWN type - is a service type, it should be the last one in the list.
 */
enum class TaskType : uint8_t { GC, JIT, UNKNOWN };
constexpr auto ALL_TASK_TYPES = std::array {TaskType::GC, TaskType::JIT};
static_assert(ALL_TASK_TYPES.size() == static_cast<size_t>(TaskType::UNKNOWN));
static_assert(std::is_same<decltype(ALL_TASK_TYPES)::value_type, TaskType>::value);

/*
 * VMType - represents all types of VM that can use TaskManager.
 * UNKNOWN type - is a service type, it should be the last one in the list.
 */
enum class VMType : uint8_t { DYNAMIC_VM, STATIC_VM, UNKNOWN };
constexpr auto ALL_VM_TYPES = std::array {VMType::DYNAMIC_VM, VMType::STATIC_VM};
static_assert(ALL_VM_TYPES.size() == static_cast<size_t>(VMType::UNKNOWN));
static_assert(std::is_same<decltype(ALL_VM_TYPES)::value_type, VMType>::value);

/*
 * TaskExecutionMode - represents all possible modes of tasks execution.
 * UNKNOWN type - is a service type, it should be the last one in the list.
 */
enum class TaskExecutionMode : uint8_t { FOREGROUND, BACKGROUND, UNKNOWN };
constexpr auto ALL_TASK_EXECUTION_MODES = std::array {TaskExecutionMode::FOREGROUND, TaskExecutionMode::BACKGROUND};
static_assert(ALL_TASK_EXECUTION_MODES.size() == static_cast<size_t>(TaskExecutionMode::UNKNOWN));
static_assert(std::is_same<decltype(ALL_TASK_EXECUTION_MODES)::value_type, TaskExecutionMode>::value);

/**
 * @brief TaskProperties is class that consider all enums that are related to Task. It's used to parameterize task
 * creation.
 */
class TaskProperties {
public:
    constexpr TaskProperties(TaskType taskType, VMType vmType, TaskExecutionMode executionMode)
        : taskType_(taskType), vmType_(vmType), executionMode_(executionMode)
    {
    }

    constexpr TaskType GetTaskType() const
    {
        return taskType_;
    }

    constexpr VMType GetVMType() const
    {
        return vmType_;
    }

    constexpr TaskExecutionMode GetTaskExecutionMode() const
    {
        return executionMode_;
    }

    friend constexpr bool operator==(const TaskProperties &lv, const TaskProperties &rv)
    {
        return lv.taskType_ == rv.taskType_ && lv.vmType_ == rv.vmType_ && lv.executionMode_ == rv.executionMode_;
    }

    class Hash {
    public:
        constexpr Hash() = default;
        constexpr uint32_t operator()(const TaskProperties &properties) const
        {
            return (static_cast<uint32_t>(properties.taskType_) << (2U * sizeof(uint8_t) * BITS_PER_BYTE)) |
                   (static_cast<uint32_t>(properties.vmType_) << (sizeof(uint8_t) * BITS_PER_BYTE)) |
                   (static_cast<uint32_t>(properties.executionMode_));
        }
    };

private:
    TaskType taskType_;
    VMType vmType_;
    TaskExecutionMode executionMode_;
};

constexpr auto INVALID_TASK_PROPERTIES = TaskProperties(TaskType::UNKNOWN, VMType::UNKNOWN, TaskExecutionMode::UNKNOWN);

/**
 * @brief TaskLifeTimeStorage structure used to save and log life time of task and execution time.
 * It's methods doesn't works on release build.
 */
class TaskLifeTimeStorage {
public:
    NO_COPY_SEMANTIC(TaskLifeTimeStorage);
    DEFAULT_MOVE_SEMANTIC(TaskLifeTimeStorage);

    /// @brief Created structure and save creation time
    PANDA_PUBLIC_API TaskLifeTimeStorage()  // NOLINT(modernize-use-equals-default)
    {
#ifndef NDEBUG
        creationTime_ = time::GetCurrentTimeInMicros(true);
#endif
    }

    /// @brief Saves start time of execution. Use this method before task execution start
    PANDA_PUBLIC_API void IndicateTaskExecutionStart()
    {
#ifndef NDEBUG
        startExecutionTime_ = time::GetCurrentTimeInMicros(true);
#endif
    }

    /// @brief Logs life time and execution time of task. Use this method after thar execution finish
    PANDA_PUBLIC_API void IndicateTaskExecutionEnd()
    {
#ifndef NDEBUG
        ASSERT(startExecutionTime_ != 0)
        auto finishExecutionTime = time::GetCurrentTimeInMicros(true);
        LOG(DEBUG, TASK_MANAGER) << "Task life time: " << finishExecutionTime - creationTime_
                                 << "µs; Task execution time: " << finishExecutionTime - startExecutionTime_ << "µs; ";
#endif
    }

    PANDA_PUBLIC_API ~TaskLifeTimeStorage() = default;

private:
#ifndef NDEBUG
    // Time points for logs
    size_t creationTime_ {0};
    size_t startExecutionTime_ {0};
#endif
};

class Task {
public:
    PANDA_PUBLIC_API Task() = default;
    NO_COPY_SEMANTIC(Task);
    DEFAULT_MOVE_SEMANTIC(Task);

    using RunnerCallback = std::function<void()>;

    /**
     * @brief Tasks are created through this method with the specified arguments.
     * @param properties - properties of task, it contains TaskType, VMType and ExecutionMote.
     * @param runner - body of task, that will be executed.
     */
    [[nodiscard]] PANDA_PUBLIC_API static Task Create(TaskProperties properties, RunnerCallback runner);

    /// @brief Returns properties of task
    PANDA_PUBLIC_API TaskProperties GetTaskProperties() const;

    /// @brief Executes body of task
    PANDA_PUBLIC_API void RunTask();

    /**
     * @brief Makes task invalid, it should not be executed anymore.
     * @see INVALID_TASK_PROPERTIES
     */
    PANDA_PUBLIC_API void MakeInvalid();

    /**
     * @brief Checks if task is invalid
     * @see INVALID_TASK_PROPERTIES
     */
    PANDA_PUBLIC_API bool IsInvalid() const;

    PANDA_PUBLIC_API ~Task() = default;

private:
    Task(TaskProperties properties, RunnerCallback runner) : properties_(properties), runner_(std::move(runner)) {}

    TaskProperties properties_ {INVALID_TASK_PROPERTIES};
    RunnerCallback runner_ {nullptr};
    TaskLifeTimeStorage lifeTimeScope_;
};

PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskType type);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, VMType type);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskExecutionMode mode);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskProperties prop);

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_H
