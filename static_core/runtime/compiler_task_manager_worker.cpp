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

#include "runtime/compiler.h"
#include "runtime/compiler_task_manager_worker.h"

namespace panda {

CompilerTaskManagerWorker::CompilerTaskManagerWorker(mem::InternalAllocatorPtr internal_allocator, Compiler *compiler)
    : CompilerWorker(internal_allocator, compiler)
{
    compiler_task_manager_queue_ = internal_allocator_->New<taskmanager::TaskQueue>(
        taskmanager::TaskType::JIT, taskmanager::VMType::STATIC_VM, taskmanager::TaskQueue::DEFAULT_PRIORITY);
    ASSERT(compiler_task_manager_queue_ != nullptr);
}

void CompilerTaskManagerWorker::JoinWorker()
{
    //  TODO(molotkov): implement method in TaskManager to wait for tasks to finish (#13962)
    os::memory::LockHolder lock(task_queue_lock_);
    compiler_worker_joined_ = true;
    while (!compiler_task_deque_.empty()) {
        compiler_tasks_processed_.Wait(&task_queue_lock_);
    }
}

void CompilerTaskManagerWorker::AddTask(CompilerTask &&task)
{
    bool start_compile = false;
    {
        os::memory::LockHolder lock(task_queue_lock_);
        if (compiler_worker_joined_) {
            return;
        }
        start_compile = compiler_task_deque_.empty();
        if (start_compile) {
            CompilerTask empty_task;
            compiler_task_deque_.emplace_back(std::move(empty_task));
        } else {
            compiler_task_deque_.emplace_back(std::move(task));
        }
    }
    // This means that this is first task in queue, so we can compile it in-place.
    if (start_compile) {
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        AddTaskInTaskManager(std::move(task));
    }
}

void CompilerTaskManagerWorker::AddTaskInTaskManager(CompilerTask &&ctx)
{
    auto *ctx_ptr = internal_allocator_->New<CompilerTask>(std::move(ctx));
    auto task_runner = [this, ctx_ptr] {
        if (ctx_ptr->GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
            compiler_->StartCompileMethod(std::move(*ctx_ptr));
        }
        internal_allocator_->Delete(ctx_ptr);
        {
            os::memory::LockHolder lock(task_queue_lock_);
            ASSERT(!compiler_task_deque_.empty());
            // compilation of current task is complete
            compiler_task_deque_.pop_front();
            CompileNextMethod();
        }
    };
    compiler_task_manager_queue_->AddTask(taskmanager::Task::Create(JIT_TASK_PROPERTIES, std::move(task_runner)));
}

void CompilerTaskManagerWorker::CompileNextMethod()
{
    if (!compiler_task_deque_.empty()) {
        // now queue has empty task which will be popped at the end of compilation
        auto next_task = std::move(compiler_task_deque_.front());
        AddTaskInTaskManager(std::move(next_task));
        return;
    }
    if (compiler_worker_joined_) {
        compiler_tasks_processed_.Signal();
    }
}

}  // namespace panda
