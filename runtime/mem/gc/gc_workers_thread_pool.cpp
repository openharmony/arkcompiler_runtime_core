/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/gc_workers_thread_pool.h"

#include <utility>
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/region_space.h"

namespace panda::mem {

bool GCWorkersProcessor::Init()
{
    return gc_threads_pools_->GetGC()->InitWorker(&worker_data_);
}

bool GCWorkersProcessor::Destroy()
{
    gc_threads_pools_->GetGC()->DestroyWorker(worker_data_);
    return true;
}

bool GCWorkersProcessor::Process(GCWorkersTask task)
{
    gc_threads_pools_->GetGC()->WorkerTaskProcessing(&task, worker_data_);
    gc_threads_pools_->IncreaseSolvedTasks();
    return true;
}

GCWorkersThreadPool::GCWorkersThreadPool(mem::InternalAllocatorPtr internal_allocator, GC *gc, size_t threads_count)
    : gc_(gc), internal_allocator_(internal_allocator)
{
    ASSERT(gc->GetPandaVm() != nullptr);
    queue_ = internal_allocator_->New<GCWorkersQueueSimple>(internal_allocator_, QUEUE_SIZE_MAX_SIZE);
    worker_iface_ = internal_allocator_->New<GCWorkersCreationInterface>(gc->GetPandaVm());
    thread_pool_ = internal_allocator_->New<ThreadPool<GCWorkersTask, GCWorkersProcessor, GCWorkersThreadPool *>>(
        internal_allocator_, queue_, this, threads_count, "GC_WORKER",
        static_cast<WorkerCreationInterface *>(worker_iface_));
}

GCWorkersThreadPool::~GCWorkersThreadPool()
{
    internal_allocator_->Delete(thread_pool_);
    internal_allocator_->Delete(worker_iface_);
    queue_->Finalize();
    internal_allocator_->Delete(queue_);
}

void GCWorkersThreadPool::WaitUntilTasksEnd()
{
    thread_pool_->Help();
    for (;;) {
        os::memory::LockHolder lock(cond_var_lock_);
        if (GetSolvedTasks() == GetSendedTasks()) {
            break;
        }
        cond_var_.TimedWait(&cond_var_lock_, ALL_TASK_FINISH_WAIT_TIMEOUT);
    }
    ResetTasks();
}

}  // namespace panda::mem
