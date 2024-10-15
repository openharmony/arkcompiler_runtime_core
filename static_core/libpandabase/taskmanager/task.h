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
#include "libpandabase/utils/bit_utils.h"
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
constexpr uint8_t TASK_TYPE_NEEDED_BITS = MinimumBitsToStore(static_cast<size_t>(TaskType::UNKNOWN) - 1U);
static_assert(ALL_TASK_TYPES.size() == static_cast<size_t>(TaskType::UNKNOWN));
static_assert(std::is_same<decltype(ALL_TASK_TYPES)::value_type, TaskType>::value);

/*
 * VMType - represents all types of VM that can use TaskManager.
 * UNKNOWN type - is a service type, it should be the last one in the list.
 */
enum class VMType : uint8_t { DYNAMIC_VM, STATIC_VM, UNKNOWN };
constexpr auto ALL_VM_TYPES = std::array {VMType::DYNAMIC_VM, VMType::STATIC_VM};
constexpr uint8_t VM_TYPE_NEEDED_BITS = MinimumBitsToStore(static_cast<size_t>(VMType::UNKNOWN) - 1U);
static_assert(ALL_VM_TYPES.size() == static_cast<size_t>(VMType::UNKNOWN));
static_assert(std::is_same<decltype(ALL_VM_TYPES)::value_type, VMType>::value);

/*
 * TaskExecutionMode - represents all possible modes of tasks execution.
 * UNKNOWN type - is a service type, it should be the last one in the list.
 */
enum class TaskExecutionMode : uint8_t { FOREGROUND, BACKGROUND, UNKNOWN };
constexpr auto ALL_TASK_EXECUTION_MODES = std::array {TaskExecutionMode::FOREGROUND, TaskExecutionMode::BACKGROUND};
constexpr uint8_t EXECUTION_MODE_NEEDED_BITS = MinimumBitsToStore(static_cast<size_t>(TaskExecutionMode::UNKNOWN) - 1U);
static_assert(ALL_TASK_EXECUTION_MODES.size() == static_cast<size_t>(TaskExecutionMode::UNKNOWN));
static_assert(std::is_same<decltype(ALL_TASK_EXECUTION_MODES)::value_type, TaskExecutionMode>::value);

/**
 * @brief TaskProperties is class that consider all enums that are related to Task. It's used to parameterize task
 * creation.
 */
class TaskProperties {
    // NOLINTBEGIN(hicpp-signed-bitwise)
    /*
     *  TaskProperties representation:
     *
     * +-------+-------------------------------+-------+-------+-------+
     * | 0     |         zeros                 | Task  | VM    |Exec   |
     * |       |                               |  Type |  Type | Mode  |
     * +-------+-------------------------------+-------+-------+-------+
     * +-------+-------------------------------------------------------+
     * | 1     |         zeros     (invalid)                           |
     * |       |                                                       |
     * +-------+-------------------------------------------------------+
     */
    using StorageType = uint8_t;
    // Checking that we have enough bits for correct representation
    static_assert((TASK_TYPE_NEEDED_BITS + VM_TYPE_NEEDED_BITS + EXECUTION_MODE_NEEDED_BITS) <
                  (sizeof(StorageType) * BITS_PER_BYTE));
    StorageType val_;

    static constexpr StorageType TASK_TYPE_MASK = (1U << TASK_TYPE_NEEDED_BITS) - 1U;
    static constexpr StorageType TASK_TYPE_SHIFT = VM_TYPE_NEEDED_BITS + EXECUTION_MODE_NEEDED_BITS;

    static constexpr StorageType VM_TYPE_MASK = (1U << VM_TYPE_NEEDED_BITS) - 1U;
    static constexpr StorageType VM_TYPE_SHIFT = EXECUTION_MODE_NEEDED_BITS;

    static constexpr StorageType EXECUTION_MODE_MASK = (1U << EXECUTION_MODE_NEEDED_BITS) - 1U;

    static inline constexpr StorageType GetInternalRepresentation(StorageType taskType, StorageType vmType,
                                                                  StorageType executionMode)
    {
        return (taskType << TASK_TYPE_SHIFT) | (vmType << VM_TYPE_SHIFT) | executionMode;
    }

    static inline constexpr StorageType GetTaskTypeFromInternalRepresentation(StorageType val) noexcept
    {
        return (val >> TASK_TYPE_SHIFT) & TASK_TYPE_MASK;
    }

    static inline constexpr StorageType GetVMTypeFromInternalRepresentation(StorageType val) noexcept
    {
        return (val >> VM_TYPE_SHIFT) & VM_TYPE_MASK;
    }

    static inline constexpr StorageType GetExecutionModeFromInternalRepresentation(StorageType val) noexcept
    {
        return val & EXECUTION_MODE_MASK;
    }

    static constexpr StorageType INVALID_INTERNAL_REPRESENTATION = 1U << (sizeof(StorageType) * BITS_PER_BYTE - 1U);

public:
    constexpr TaskProperties() : val_(INVALID_INTERNAL_REPRESENTATION) {}

    constexpr TaskProperties(TaskType taskType, VMType vmType, TaskExecutionMode executionMode)
        : val_(GetInternalRepresentation(static_cast<StorageType>(taskType), static_cast<StorageType>(vmType),
                                         static_cast<StorageType>(executionMode)))
    {
    }

    constexpr TaskType GetTaskType() const
    {
        return static_cast<TaskType>(GetTaskTypeFromInternalRepresentation(val_));
    }

    constexpr VMType GetVMType() const
    {
        return static_cast<VMType>(GetVMTypeFromInternalRepresentation(val_));
    }

    constexpr TaskExecutionMode GetTaskExecutionMode() const
    {
        return static_cast<TaskExecutionMode>(GetExecutionModeFromInternalRepresentation(val_));
    }

    friend constexpr bool operator==(const TaskProperties &lv, const TaskProperties &rv)
    {
        return lv.val_ == rv.val_;
    }

    class Hash {
    public:
        static constexpr size_t MaxValue()
        {
            return GetInternalRepresentation(static_cast<StorageType>(TaskType::UNKNOWN) - 1U,
                                             static_cast<StorageType>(VMType::UNKNOWN) - 1U,
                                             static_cast<StorageType>(TaskExecutionMode::UNKNOWN) - 1U);
        }

        constexpr Hash() = default;
        constexpr size_t operator()(const TaskProperties &properties) const
        {
            return properties.val_;
        }
    };
    // NOLINTEND(hicpp-signed-bitwise)
};

constexpr auto INVALID_TASK_PROPERTIES = TaskProperties();

PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskType type);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, VMType type);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskExecutionMode mode);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskProperties prop);

/**
 * @brief TaskLifeTimeAggregator structure used to save and log life time of task and execution time.
 * It's methods doesn't works on release build.
 */
class TaskLifeTimeAggregator {
public:
    NO_COPY_SEMANTIC(TaskLifeTimeAggregator);
    DEFAULT_MOVE_SEMANTIC(TaskLifeTimeAggregator);

    PANDA_PUBLIC_API TaskLifeTimeAggregator() = default;

    /// @brief Saves time of task adding in queue.
    PANDA_PUBLIC_API void GetAndStoreTimeOfTaskAddingToQueue();

    /// @brief Saves start time of execution. Use this method before task execution start
    PANDA_PUBLIC_API void GetAndStoreTimeOfTaskExecutionStart();

    /// @brief Logs life time and execution time of task. Use this method after task execution end
    PANDA_PUBLIC_API void GetTimeOfTaskExecutionFinishAndStoreTimeStats(TaskProperties prop);

    PANDA_PUBLIC_API ~TaskLifeTimeAggregator() = default;

private:
    // Time points for logs
    uint64_t addingToQueueTime_ {0};
    uint64_t startExecutionTime_ {0};
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

    PANDA_PUBLIC_API void EventOnTaskAdding();

    PANDA_PUBLIC_API ~Task() = default;

private:
    Task(TaskProperties properties, RunnerCallback runner) : properties_(properties), runner_(std::move(runner)) {}

    PANDA_PUBLIC_API void EventOnStartExecution();
    PANDA_PUBLIC_API void EventOnEndExecution();

    TaskProperties properties_ {INVALID_TASK_PROPERTIES};
    RunnerCallback runner_ {nullptr};
    TaskLifeTimeAggregator lifeTimeStorage_;
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_H
