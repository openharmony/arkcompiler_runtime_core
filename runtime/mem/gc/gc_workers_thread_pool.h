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
#include "runtime/thread_pool_queue.h"
#include "runtime/mem/gc/gc.h"

namespace panda::mem {
class GCWorkersThreadPool;
class Region;

inline static const char *GCWorkersTaskTypesToString(GCWorkersTaskTypes type)
{
    switch (type) {
        case GCWorkersTaskTypes::TASK_EMPTY:
            return "Empty task";
        case GCWorkersTaskTypes::TASK_MARKING:
            return "Marking task";
        case GCWorkersTaskTypes::TASK_REMARK:
            return "Remark task";
        case GCWorkersTaskTypes::TASK_REGION_COMPACTING:
            return "Region compacting task";
        case GCWorkersTaskTypes::TASK_ROOT_COLLECTION:
            return "Root collection task";
        case GCWorkersTaskTypes::TASK_MOVE_YOUNG_ALIVE_OBJECTS:
            return "Move alive young objects task";
        case GCWorkersTaskTypes::TASK_INIT_REFS_FROM_REMSETS:
            return "Initialize references from remsets task";
        default:
            return "Unknown task";
    }
}

class RefInfo {
    static constexpr uint32_t VOLATILE_MASK = 1U;

public:
    RefInfo() = default;

    RefInfo(ObjectHeader *object, uint32_t ref_offset, bool is_volatile)
        : object_(object), ref_offset_(ref_offset | static_cast<uint32_t>(is_volatile))
    {
    }

    ~RefInfo() = default;

    ObjectHeader *GetObject() const
    {
        return object_;
    }

    uint32_t GetReferenceOffset() const
    {
        return ref_offset_ & ~VOLATILE_MASK;
    }

    bool IsVolatile() const
    {
        return (ref_offset_ & VOLATILE_MASK) != 0;
    }

    DEFAULT_COPY_SEMANTIC(RefInfo);
    DEFAULT_MOVE_SEMANTIC(RefInfo);

private:
    ObjectHeader *object_;
    uint32_t ref_offset_;
};

class GCWorkersTask : public TaskInterface {
public:
    using StackType = GCMarkingStackType;
    using RegionDataType = std::pair<PandaVector<ObjectHeader *> *, Region *>;
    using RefVector = PandaVector<RefInfo>;

    GCWorkersTask()

    {
        task_type_ = GCWorkersTaskTypes::TASK_EMPTY;
    }

    explicit GCWorkersTask(GCWorkersTaskTypes type, StackType *marking_stack) : task_type_(type)
    {
        ASSERT(type == GCWorkersTaskTypes::TASK_MARKING || type == GCWorkersTaskTypes::TASK_REMARK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        storage_.marking_stack = marking_stack;
    }

    explicit GCWorkersTask(GCWorkersTaskTypes type, RegionDataType *region_data) : task_type_(type)
    {
        ASSERT(type == GCWorkersTaskTypes::TASK_REGION_COMPACTING);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        storage_.region_data = region_data;
    }

    explicit GCWorkersTask(GCWorkersTaskTypes type, CardTable::CardPtr card) : task_type_(type)
    {
        ASSERT(type == GCWorkersTaskTypes::TASK_INIT_REFS_FROM_REMSETS);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        storage_.card = card;
    }

    ~GCWorkersTask() = default;
    DEFAULT_COPY_SEMANTIC(GCWorkersTask);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    DEFAULT_MOVE_SEMANTIC(GCWorkersTask);

    bool IsEmpty() const
    {
        return task_type_ == GCWorkersTaskTypes::TASK_EMPTY;
    }

    StackType *GetMarkingStack() const
    {
        ASSERT(task_type_ == GCWorkersTaskTypes::TASK_MARKING || task_type_ == GCWorkersTaskTypes::TASK_REMARK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return storage_.marking_stack;
    }

    RegionDataType *GetRegionData() const
    {
        ASSERT(task_type_ == GCWorkersTaskTypes::TASK_REGION_COMPACTING);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return storage_.region_data;
    }

    CardTable::CardPtr GetCard() const
    {
        ASSERT(task_type_ == GCWorkersTaskTypes::TASK_INIT_REFS_FROM_REMSETS);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return storage_.card;
    }

    GCWorkersTaskTypes GetType() const
    {
        return task_type_;
    }

private:
    union StorageType {
        StackType *marking_stack;
        RegionDataType *region_data;
        CardTable::CardPtr card;
    };
    GCWorkersTaskTypes task_type_;
    StorageType storage_ {nullptr};
};

class GCWorkersProcessor : public ProcessorInterface<GCWorkersTask, GCWorkersThreadPool *> {
public:
    explicit GCWorkersProcessor(GCWorkersThreadPool *gc_threads_pools)
        : gc_threads_pools_(gc_threads_pools), worker_data_(nullptr)
    {
    }

    ~GCWorkersProcessor() override = default;
    NO_COPY_SEMANTIC(GCWorkersProcessor);
    NO_MOVE_SEMANTIC(GCWorkersProcessor);

    bool Process(GCWorkersTask task) override;
    bool Init() override;
    bool Destroy() override;

private:
    GCWorkersThreadPool *gc_threads_pools_;
    void *worker_data_;
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
    void AddTask(GCWorkersTask ctx, [[maybe_unused]] size_t priority = 0) override
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

class GCWorkersThreadPool {
public:
    NO_COPY_SEMANTIC(GCWorkersThreadPool);
    NO_MOVE_SEMANTIC(GCWorkersThreadPool);
    GCWorkersThreadPool(mem::InternalAllocatorPtr internal_allocator, GC *gc, size_t threads_count = 0);
    ~GCWorkersThreadPool();

    template <class T>
    bool AddTask(GCWorkersTaskTypes type, T *storage_ptr)
    {
        IncreaseSendedTasks();
        GCWorkersTask task {type, storage_ptr};
        if (thread_pool_->TryPutTask(task)) {
            LOG(DEBUG, GC) << "Added a new " << GCWorkersTaskTypesToString(type) << " to queue";
            return true;
        }
        DecreaseSendedTasks();
        return false;
    }

    void WaitUntilTasksEnd();

    GC *GetGC()
    {
        return gc_;
    }

    void IncreaseSolvedTasks()
    {
        solved_tasks_++;
        if (solved_tasks_ == sended_tasks_) {
            os::memory::LockHolder lock(cond_var_lock_);
            cond_var_.Signal();
        }
    }

    void IncreaseSendedTasks()
    {
        sended_tasks_++;
    }

private:
    static constexpr uint64_t ALL_TASK_FINISH_WAIT_TIMEOUT = 100U;

    void ResetTasks()
    {
        solved_tasks_ = 0;
        sended_tasks_ = 0;
    }

    size_t GetSolvedTasks()
    {
        return solved_tasks_;
    }

    void DecreaseSendedTasks()
    {
        sended_tasks_--;
    }

    size_t GetSendedTasks()
    {
        return sended_tasks_;
    }

    GC *gc_;
    ThreadPool<GCWorkersTask, GCWorkersProcessor, GCWorkersThreadPool *> *thread_pool_;
    GCWorkersQueueSimple *queue_;
    GCWorkersCreationInterface *worker_iface_;
    mem::InternalAllocatorPtr internal_allocator_;
    std::atomic_size_t solved_tasks_ {0};
    std::atomic_size_t sended_tasks_ {0};
    os::memory::Mutex cond_var_lock_;
    os::memory::ConditionVariable cond_var_;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H
