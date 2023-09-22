/*
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
#include "compiler/background_task_runner.h"
#include "compiler/compiler_task_runner.h"

namespace panda {

CompilerTaskManagerWorker::CompilerTaskManagerWorker(mem::InternalAllocatorPtr internal_allocator, Compiler *compiler)
    : CompilerWorker(internal_allocator, compiler)
{
    auto *tm = taskmanager::TaskScheduler::GetTaskScheduler();
    compiler_task_manager_queue_ = tm->CreateAndRegisterTaskQueue<decltype(internal_allocator_->Adapter())>(
        taskmanager::TaskType::JIT, taskmanager::VMType::STATIC_VM, taskmanager::TaskQueueInterface::DEFAULT_PRIORITY);
    ASSERT(compiler_task_manager_queue_ != nullptr);
}

void CompilerTaskManagerWorker::JoinWorker()
{
    {
        os::memory::LockHolder lock(task_queue_lock_);
        compiler_worker_joined_ = true;
    }
    taskmanager::TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(JIT_TASK_PROPERTIES);
}

void CompilerTaskManagerWorker::AddTask(CompilerTask &&task)
{
    {
        os::memory::LockHolder lock(task_queue_lock_);
        if (compiler_worker_joined_) {
            return;
        }
        if (compiler_task_deque_.empty()) {
            CompilerTask empty_task;
            compiler_task_deque_.emplace_back(std::move(empty_task));
        } else {
            compiler_task_deque_.emplace_back(std::move(task));
            return;
        }
    }
    // This means that this is first task in queue, so we can compile it in-place.
    BackgroundCompileMethod(std::move(task));
}

void CompilerTaskManagerWorker::BackgroundCompileMethod(CompilerTask &&ctx)
{
    auto thread_deleter = [this](Thread *thread) { internal_allocator_->Delete(thread); };
    compiler::BackgroundCompilerContext::CompilerThread compiler_thread(
        internal_allocator_->New<Thread>(ctx.GetVM(), Thread::ThreadType::THREAD_TYPE_COMPILER),
        std::move(thread_deleter));

    auto task_deleter = [this](CompilerTask *task) { internal_allocator_->Delete(task); };
    compiler::BackgroundCompilerContext::CompilerTask compiler_task(
        internal_allocator_->New<CompilerTask>(std::move(ctx)), std::move(task_deleter));

    compiler::BackgroundCompilerTaskRunner task_runner(compiler_task_manager_queue_, compiler_thread.get(),
                                                       compiler_->GetRuntimeInterface());
    auto &compiler_ctx = task_runner.GetContext();
    compiler_ctx.SetCompilerThread(std::move(compiler_thread));
    compiler_ctx.SetCompilerTask(std::move(compiler_task));

    // Callback to compile next method from compiler_task_deque_
    task_runner.AddFinalize([this]([[maybe_unused]] compiler::BackgroundCompilerContext &task_context) {
        CompilerTask next_task;
        {
            os::memory::LockHolder lock(task_queue_lock_);
            ASSERT(!compiler_task_deque_.empty());
            // compilation of current task is complete
            compiler_task_deque_.pop_front();
            if (compiler_task_deque_.empty()) {
                return;
            }
            // now queue has empty task which will be popped at the end of compilation
            next_task = std::move(compiler_task_deque_.front());
        }
        BackgroundCompileMethod(std::move(next_task));
    });

    auto background_task = [this](compiler::BackgroundCompilerTaskRunner runner) {
        if (runner.GetContext().GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
            compiler_->StartCompileMethod<compiler::BACKGROUND_MODE>(std::move(runner));
            return;
        }
        compiler::BackgroundCompilerTaskRunner::EndTask(std::move(runner), false);
    };
    compiler::BackgroundCompilerTaskRunner::StartTask(std::move(task_runner), std::move(background_task));
}

}  // namespace panda
