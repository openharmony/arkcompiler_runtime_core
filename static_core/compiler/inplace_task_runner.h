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

#ifndef PANDA_COMPILER_INPLACE_TASK_RUNNER_H
#define PANDA_COMPILER_INPLACE_TASK_RUNNER_H

#include <memory>

#include "libpandabase/task_runner.h"
#include "libpandabase/mem/arena_allocator.h"

namespace panda {

class Method;
class PandaVM;
}  // namespace panda

namespace panda::compiler {

class Pipeline;
class Graph;

class InPlaceCompilerContext {
public:
    void SetMethod(Method *method)
    {
        method_ = method;
    }

    void SetOsr(bool is_osr)
    {
        is_osr_ = is_osr;
    }

    void SetVM(PandaVM *panda_vm)
    {
        panda_vm_ = panda_vm;
    }

    void SetAllocator(ArenaAllocator *allocator)
    {
        allocator_ = allocator;
    }

    void SetLocalAllocator(ArenaAllocator *local_allocator)
    {
        local_allocator_ = local_allocator;
    }

    void SetMethodName(std::string method_name)
    {
        method_name_ = std::move(method_name);
    }

    void SetGraph(Graph *graph)
    {
        graph_ = graph;
    }

    void SetPipeline(Pipeline *pipeline)
    {
        pipeline_ = pipeline;
    }

    void SetCompilationStatus(bool compilation_status)
    {
        compilation_status_ = compilation_status;
    }

    Method *GetMethod() const
    {
        return method_;
    }

    bool IsOsr() const
    {
        return is_osr_;
    }

    PandaVM *GetVM() const
    {
        return panda_vm_;
    }

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return local_allocator_;
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
        return pipeline_;
    }

    bool GetCompilationStatus() const
    {
        return compilation_status_;
    }

private:
    Method *method_ {nullptr};
    bool is_osr_ {false};
    PandaVM *panda_vm_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    ArenaAllocator *local_allocator_ {nullptr};
    std::string method_name_;
    Graph *graph_ {nullptr};
    Pipeline *pipeline_ {nullptr};
    // Used only in JIT Compilation
    bool compilation_status_ {false};
};

class InPlaceCompilerTaskRunner : public panda::TaskRunner<InPlaceCompilerTaskRunner, InPlaceCompilerContext> {
public:
    InPlaceCompilerContext &GetContext() override
    {
        return task_ctx_;
    }

    /**
     * @brief Starts task in-place
     * @param task_runner - Current TaskRunner containing context and callbacks
     * @param task_func - task which will be executed with @param task_runner
     */
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    static void StartTask(InPlaceCompilerTaskRunner task_runner, TaskRunner::TaskFunc task_func)
    {
        task_func(std::move(task_runner));
    }

private:
    InPlaceCompilerContext task_ctx_;
};

}  // namespace panda::compiler

#endif  // PANDA_COMPILER_INPLACE_TASK_RUNNER_H
