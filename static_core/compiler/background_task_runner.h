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

#ifndef PANDA_COMPILER_BACKGROUND_TASK_RUNNER_H
#define PANDA_COMPILER_BACKGROUND_TASK_RUNNER_H

#include <memory>

#include "libpandabase/task_runner.h"
#include "libpandabase/taskmanager/task.h"
#include "libpandabase/taskmanager/task_queue.h"
#include "libpandabase/mem/arena_allocator.h"
#include "compiler/optimizer/pipeline.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/thread.h"

namespace panda {

class CompilerTask;
class Thread;
class Method;
class PandaVM;

}  // namespace panda

namespace panda::compiler {

class Graph;

class BackgroundCompilerContext {
public:
    using CompilerTask = std::unique_ptr<panda::CompilerTask, std::function<void(panda::CompilerTask *)>>;
    using CompilerThread = std::unique_ptr<panda::Thread, std::function<void(panda::Thread *)>>;

    void SetCompilerTask(CompilerTask compiler_task)
    {
        compiler_task_ = std::move(compiler_task);
    }

    void SetCompilerThread(CompilerThread compiler_thread)
    {
        compiler_thread_ = std::move(compiler_thread);
    }

    void SetAllocator(std::unique_ptr<ArenaAllocator> allocator)
    {
        allocator_ = std::move(allocator);
    }

    void SetLocalAllocator(std::unique_ptr<ArenaAllocator> local_allocator)
    {
        local_allocator_ = std::move(local_allocator);
    }

    void SetMethodName(std::string method_name)
    {
        method_name_ = std::move(method_name);
    }

    void SetGraph(Graph *graph)
    {
        graph_ = graph;
    }

    void SetPipeline(std::unique_ptr<Pipeline> pipeline)
    {
        pipeline_ = std::move(pipeline);
    }

    void SetCompilationStatus(bool compilation_status)
    {
        compilation_status_ = compilation_status;
    }

    Method *GetMethod() const
    {
        return compiler_task_->GetMethod();
    }

    bool IsOsr() const
    {
        return compiler_task_->IsOsr();
    }

    PandaVM *GetVM() const
    {
        return compiler_task_->GetVM();
    }

    ArenaAllocator *GetAllocator() const
    {
        return allocator_.get();
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return local_allocator_.get();
    }

    const std::string &GetMethodName() const
    {
        return method_name_;
    }

    Graph *GetGraph() const
    {
        return graph_;
    }

    Pipeline *GetPipeline() const
    {
        return pipeline_.get();
    }

    bool GetCompilationStatus() const
    {
        return compilation_status_;
    }

private:
    CompilerTask compiler_task_;
    CompilerThread compiler_thread_;
    std::unique_ptr<ArenaAllocator> allocator_;
    std::unique_ptr<ArenaAllocator> local_allocator_;
    std::string method_name_;
    Graph *graph_ {nullptr};
    std::unique_ptr<Pipeline> pipeline_;
    // Used only in JIT Compilation
    bool compilation_status_ {false};
};

namespace copy_hooks {
class FailOnCopy {
public:
    FailOnCopy() = default;
    FailOnCopy(const FailOnCopy &other)
    {
        UNUSED_VAR(other);
        UNREACHABLE();
    }
    FailOnCopy &operator=(const FailOnCopy &other)
    {
        UNUSED_VAR(other);
        UNREACHABLE();
    }
    DEFAULT_MOVE_SEMANTIC(FailOnCopy);
    ~FailOnCopy() = default;
};

template <typename T>
class FakeCopyable : public FailOnCopy {
public:
    explicit FakeCopyable(T &&t) : target_(std::forward<T>(t)) {}
    FakeCopyable(const FakeCopyable &other)
        : FailOnCopy(other),                                  // this will fail
          target_(std::move(const_cast<T &>(other.target_)))  // never reached
    {
    }
    FakeCopyable &operator=(const FakeCopyable &other) = default;
    DEFAULT_MOVE_SEMANTIC(FakeCopyable);
    ~FakeCopyable() = default;

    template <typename... Args>
    auto operator()(Args &&...args)
    {
        return target_(std::forward<Args>(args)...);
    }

private:
    T target_;
};

template <typename T>
FakeCopyable<T> MakeFakeCopyable(T &&t)
{
    return FakeCopyable<T>(std::forward<T>(t));
}

}  // namespace copy_hooks

class BackgroundCompilerTaskRunner : public panda::TaskRunner<BackgroundCompilerTaskRunner, BackgroundCompilerContext> {
public:
    static constexpr taskmanager::TaskProperties TASK_PROPERTIES = {
        taskmanager::TaskType::JIT, taskmanager::VMType::STATIC_VM, taskmanager::TaskExecutionMode::BACKGROUND};

    BackgroundCompilerTaskRunner(taskmanager::TaskQueueInterface *compiler_queue, Thread *compiler_thread,
                                 RuntimeInterface *runtime_iface)
        : compiler_queue_(compiler_queue), compiler_thread_(compiler_thread), runtime_iface_(runtime_iface)
    {
    }

    BackgroundCompilerContext &GetContext() override
    {
        return task_ctx_;
    }

    /**
     * @brief Adds a task to the TaskManager queue. This task will run in the background
     * @param task_runner - Current TaskRunner containing context and callbacks
     * @param task_func - task which will be executed with @param task_runner
     */
    static void StartTask(BackgroundCompilerTaskRunner task_runner, TaskRunner::TaskFunc task_func)
    {
        auto *compiler_queue = task_runner.compiler_queue_;
        auto callback = [next_task = std::move(task_func), next_runner = std::move(task_runner)]() mutable {
            auto *runtime_iface = next_runner.runtime_iface_;
            runtime_iface->SetCurrentThread(next_runner.compiler_thread_);
            next_task(std::move(next_runner));
            runtime_iface->SetCurrentThread(nullptr);
        };
        // We use MakeFakeCopyable becuase std::function can't wrap a lambda that has captured non-copyable object
        auto task = taskmanager::Task::Create(TASK_PROPERTIES, copy_hooks::MakeFakeCopyable(std::move(callback)));
        compiler_queue->AddTask(std::move(task));
    }

private:
    taskmanager::TaskQueueInterface *compiler_queue_ {nullptr};
    Thread *compiler_thread_ {nullptr};
    RuntimeInterface *runtime_iface_;
    BackgroundCompilerContext task_ctx_;
};

}  // namespace panda::compiler

#endif  // PANDA_COMPILER_BACKGROUND_TASK_RUNNER_H
