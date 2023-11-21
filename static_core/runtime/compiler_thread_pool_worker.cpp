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
#include "runtime/compiler_thread_pool_worker.h"
#include "runtime/compiler_queue_simple.h"
#include "runtime/compiler_queue_aged_counter_priority.h"

namespace panda {

CompilerThreadPoolWorker::CompilerThreadPoolWorker(mem::InternalAllocatorPtr internal_allocator, Compiler *compiler,
                                                   bool &no_async_jit, const RuntimeOptions &options)
    : CompilerWorker(internal_allocator, compiler)
{
    queue_ = CreateJITTaskQueue(no_async_jit ? "simple" : options.GetCompilerQueueType(),
                                options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan(),
                                options.GetCompilerDeathCounterValue(), options.GetCompilerEpochDuration());
    if (queue_ == nullptr) {
        // Because of problems (no memory) in allocator
        LOG(ERROR, COMPILER) << "Cannot create a compiler queue";
        no_async_jit = true;
    }
}

CompilerThreadPoolWorker::~CompilerThreadPoolWorker()
{
    if (queue_ != nullptr) {
        queue_->Finalize();
        internal_allocator_->Delete(queue_);
    }
}

CompilerQueueInterface *CompilerThreadPoolWorker::CreateJITTaskQueue(const std::string &queue_type, uint64_t max_length,
                                                                     uint64_t task_life, uint64_t death_counter,
                                                                     uint64_t epoch_duration)
{
    LOG(DEBUG, COMPILER) << "Creating " << queue_type << " task queue";
    if (queue_type == "simple") {
        return internal_allocator_->New<CompilerQueueSimple>(internal_allocator_);
    }
    if (queue_type == "counter-priority") {
        return internal_allocator_->New<CompilerPriorityCounterQueue>(internal_allocator_, max_length, task_life);
    }
    if (queue_type == "aged-counter-priority") {
        return internal_allocator_->New<CompilerPriorityAgedCounterQueue>(internal_allocator_, task_life, death_counter,
                                                                          epoch_duration);
    }
    LOG(FATAL, COMPILER) << "Unknown queue type";
    return nullptr;
}

CompilerProcessor::CompilerProcessor(Compiler *compiler)
{
    compiler_ = compiler;
}

bool CompilerProcessor::Process(CompilerTask &&task)
{
    if (task.GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
        compiler_->CompileMethodLocked(std::move(task));
    }
    return true;
}

}  // namespace panda
