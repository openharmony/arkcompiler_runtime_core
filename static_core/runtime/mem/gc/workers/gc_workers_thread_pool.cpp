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

#include <utility>

#include "libpandabase/os/cpu_affinity.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_workers_thread_pool.h"

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

bool GCWorkersProcessor::Process(GCWorkersTask &&task)
{
    gc_threads_pools_->RunGCWorkersTask(&task, worker_data_);
    return true;
}

GCWorkersThreadPool::GCWorkersThreadPool(GC *gc, size_t threads_count)
    : GCWorkersTaskPool(gc), internal_allocator_(gc->GetInternalAllocator()), threads_count_(threads_count)
{
    ASSERT(gc->GetPandaVm() != nullptr);
    queue_ = internal_allocator_->New<GCWorkersQueueSimple>(internal_allocator_, QUEUE_SIZE_MAX_SIZE);
    worker_iface_ = internal_allocator_->New<GCWorkersCreationInterface>(gc->GetPandaVm());
    thread_pool_ = internal_allocator_->New<ThreadPool<GCWorkersTask, GCWorkersProcessor, GCWorkersThreadPool *>>(
        internal_allocator_, queue_, this, threads_count, "GC_WORKER",
        static_cast<WorkerCreationInterface *>(worker_iface_));
}

bool GCWorkersThreadPool::TryAddTask(GCWorkersTask &&task)
{
    return thread_pool_->TryPutTask(std::forward<GCWorkersTask &&>(task));
}

static void SetAffinity(GCWorkersProcessor *proc, size_t gc_threads_count)
{
    // For first GC, GC-workers can be not started
    if (UNLIKELY(!proc->IsStarted())) {
        return;
    }
    const auto &best_and_middle = os::CpuAffinityManager::GetBestAndMiddleCpuSet();
    if (gc_threads_count < best_and_middle.Count()) {
        os::CpuAffinityManager::SetAffinityForThread(proc->GetTid(), best_and_middle);
    }
}

static void UnsetAffinity(GCWorkersProcessor *proc, [[maybe_unused]] size_t data)
{
    os::CpuAffinityManager::SetAffinityForThread(proc->GetTid(), os::CpuPower::WEAK);
}

void GCWorkersThreadPool::SetAffinityForGCWorkers()
{
    // Total GC threads count = GC Thread + GC workers
    thread_pool_->EnumerateProcs(SetAffinity, threads_count_ + 1U);
}

void GCWorkersThreadPool::UnsetAffinityForGCWorkers()
{
    thread_pool_->EnumerateProcs(UnsetAffinity, 0U);
}

GCWorkersThreadPool::~GCWorkersThreadPool()
{
    internal_allocator_->Delete(thread_pool_);
    internal_allocator_->Delete(worker_iface_);
    queue_->Finalize();
    internal_allocator_->Delete(queue_);
}

void GCWorkersThreadPool::RunInCurrentThread()
{
    thread_pool_->Help();
}

}  // namespace panda::mem
