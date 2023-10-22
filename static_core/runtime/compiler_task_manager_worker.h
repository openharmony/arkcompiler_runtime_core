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
#ifndef RUTNIME_COMPILER_TASK_MANAGER_WORKER_H_
#define RUTNIME_COMPILER_TASK_MANAGER_WORKER_H_

#include "runtime/compiler_worker.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "libpandabase/taskmanager/task.h"
#include "libpandabase/taskmanager/task_queue.h"
#include "libpandabase/taskmanager/task_scheduler.h"

namespace panda {

/// @brief Compiler worker task pool based on common TaskManager (TaskQueue)
class CompilerTaskManagerWorker : public CompilerWorker {
public:
    /* Compiler task queue (TaskManager) specific variables */
    static constexpr taskmanager::TaskProperties JIT_TASK_PROPERTIES {
        taskmanager::TaskType::JIT, taskmanager::VMType::STATIC_VM, taskmanager::TaskExecutionMode::BACKGROUND};

    CompilerTaskManagerWorker(mem::InternalAllocatorPtr internal_allocator, Compiler *compiler);

    NO_COPY_SEMANTIC(CompilerTaskManagerWorker);
    NO_MOVE_SEMANTIC(CompilerTaskManagerWorker);

    void InitializeWorker() override
    {
        [[maybe_unused]] taskmanager::TaskQueueId id =
            Runtime::GetTaskScheduler()->RegisterQueue(compiler_task_manager_queue_);
        ASSERT(!(id == taskmanager::INVALID_TASKQUEUE_ID));
        compiler_worker_joined_ = false;
    }

    void FinalizeWorker() override
    {
        JoinWorker();
    }

    void JoinWorker() override;

    bool IsWorkerJoined() override
    {
        return compiler_worker_joined_;
    }

    void AddTask(CompilerTask &&task) override;

    ~CompilerTaskManagerWorker() override
    {
        internal_allocator_->Delete(compiler_task_manager_queue_);
    }

private:
    void AddTaskInTaskManager(CompilerTask &&ctx);
    void CompileNextMethod() REQUIRES(task_queue_lock_);

    taskmanager::TaskQueue *compiler_task_manager_queue_ {nullptr};
    os::memory::Mutex task_queue_lock_;
    // This queue is used for methods need to be compiled inside TaskScheduler without compilation_lock_.
    PandaDeque<CompilerTask> compiler_task_deque_ GUARDED_BY(task_queue_lock_);
    os::memory::ConditionVariable compiler_tasks_processed_ GUARDED_BY(task_queue_lock_);
    std::atomic<bool> compiler_worker_joined_ {true};
};

}  // namespace panda

#endif  // RUTNIME_COMPILER_TASK_MANAGER_WORKER_H_
