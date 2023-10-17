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

#ifndef PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H
#define PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H

#include "runtime/include/thread.h"
#include "runtime/thread_pool.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace panda::mem {
// Forward declaration for GCWorkersProcessor
class GCWorkersThreadPool;

class GCWorkersProcessor : public ProcessorInterface<GCWorkersTask, GCWorkersThreadPool *> {
public:
    explicit GCWorkersProcessor(GCWorkersThreadPool *gc_threads_pools) : gc_threads_pools_(gc_threads_pools) {}

    ~GCWorkersProcessor() override = default;
    NO_COPY_SEMANTIC(GCWorkersProcessor);
    NO_MOVE_SEMANTIC(GCWorkersProcessor);

    bool Process(GCWorkersTask &&task) override;
    bool Init() override;
    bool Destroy() override;

private:
    GCWorkersThreadPool *gc_threads_pools_;
    void *worker_data_ {nullptr};
};

class GCWorkersQueueSimple : public TaskQueueInterface<GCWorkersTask> {
public:
    explicit GCWorkersQueueSimple(mem::InternalAllocatorPtr allocator, size_t queue_limit)
        : TaskQueueInterface<GCWorkersTask>(queue_limit), queue_(allocator->Adapter())
    {
    }

    ~GCWorkersQueueSimple() override = default;
    NO_COPY_SEMANTIC(GCWorkersQueueSimple);
    NO_MOVE_SEMANTIC(GCWorkersQueueSimple);

    GCWorkersTask GetTask() override
    {
        if (queue_.empty()) {
            LOG(DEBUG, GC) << "Empty " << queue_name_ << ", return nothing";
            return GCWorkersTask();
        }
        auto task = queue_.front();
        queue_.pop_front();
        LOG(DEBUG, GC) << "Extract a task from a " << queue_name_ << ": " << GetTaskDescription(task);
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(GCWorkersTask &&ctx, [[maybe_unused]] size_t priority = 0) override
    {
        LOG(DEBUG, GC) << "Add task to a " << queue_name_ << ": " << GetTaskDescription(ctx);
        queue_.push_front(ctx);
    }

    void Finalize() override
    {
        // Nothing to deallocate
        LOG(DEBUG, GC) << "Clear a " << queue_name_;
        queue_.clear();
    }

protected:
    PandaString GetTaskDescription(const GCWorkersTask &ctx)
    {
        PandaOStringStream stream;
        stream << GCWorkersTaskTypesToString(ctx.GetType());
        return stream.str();
    }

    size_t GetQueueSize() override
    {
        return queue_.size();
    }

private:
    PandaList<GCWorkersTask> queue_;
    const char *queue_name_ = "simple gc workers task queue";
};

class GCWorkersCreationInterface : public WorkerCreationInterface {
public:
    explicit GCWorkersCreationInterface(PandaVM *vm) : gc_thread_(vm, Thread::ThreadType::THREAD_TYPE_GC)
    {
        ASSERT(vm != nullptr);
    }

    ~GCWorkersCreationInterface() override = default;
    NO_COPY_SEMANTIC(GCWorkersCreationInterface);
    NO_MOVE_SEMANTIC(GCWorkersCreationInterface);

    void AttachWorker(bool helper_thread) override
    {
        if (!helper_thread) {
            Thread::SetCurrent(&gc_thread_);
        }
    }
    void DetachWorker(bool helper_thread) override
    {
        if (!helper_thread) {
            Thread::SetCurrent(nullptr);
        }
    }

private:
    Thread gc_thread_;
};

/// @brief GC workers task pool based on internal thread pool
class GCWorkersThreadPool final : public GCWorkersTaskPool {
public:
    NO_COPY_SEMANTIC(GCWorkersThreadPool);
    NO_MOVE_SEMANTIC(GCWorkersThreadPool);
    explicit GCWorkersThreadPool(GC *gc, size_t threads_count = 0);
    ~GCWorkersThreadPool() final;

    void SetAffinityForGCWorkers();

    void UnsetAffinityForGCWorkers();

private:
    /**
     * @brief Try to add new gc workers task to thread pool
     * @param task gc workers task
     * @return true if task successfully added to thread pool, false - otherwise
     */
    bool TryAddTask(GCWorkersTask &&task) final;

    /**
     * @brief Try to get gc task from gc workers task queue and
     * run it in the current thread to help gc workers from thread pool.
     * Do nothing if couldn't get task
     * @see GCWorkersQueueSimple
     */
    void RunInCurrentThread() final;

    /* GC thread pool specific variables */
    ThreadPool<GCWorkersTask, GCWorkersProcessor, GCWorkersThreadPool *> *thread_pool_;
    GCWorkersQueueSimple *queue_;
    GCWorkersCreationInterface *worker_iface_;
    mem::InternalAllocatorPtr internal_allocator_;
    const size_t threads_count_;

    friend class GCWorkersProcessor;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H
