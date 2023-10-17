/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <functional>
#include <ostream>

namespace panda::taskmanager {

enum class TaskType : uint16_t { UNKNOWN = 0, GC, JIT };

enum class VMType : uint16_t { UNKNOWN = 0, DYNAMIC_VM, STATIC_VM };

enum class TaskExecutionMode { FOREGROUND, BACKGROUND };

/**
 * @brief TaskProperties is class that consider all enums that are related to Task. It's used to parameterize task
 * creation.
 */
class TaskProperties {
public:
    constexpr TaskProperties(TaskType task_type, VMType vm_type, TaskExecutionMode execution_mode)
        : task_type_(task_type), vm_type_(vm_type), execution_mode_(execution_mode)
    {
    }

    constexpr TaskType GetTaskType() const
    {
        return task_type_;
    }

    constexpr VMType GetVMType() const
    {
        return vm_type_;
    }

    constexpr TaskExecutionMode GetTaskExecutionMode() const
    {
        return execution_mode_;
    }

private:
    TaskType task_type_;
    VMType vm_type_;
    TaskExecutionMode execution_mode_;
};

class Task {
public:
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

    PANDA_PUBLIC_API ~Task() = default;

private:
    Task(TaskProperties properties, RunnerCallback runner);

    TaskProperties properties_;
    RunnerCallback runner_;
};

PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskType type);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, VMType type);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskExecutionMode mode);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskProperties prop);

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_H