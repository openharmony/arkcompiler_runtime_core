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

#ifndef PANDA_VERIFIER_JOB_QUEUE_JOB_QUEUE_H
#define PANDA_VERIFIER_JOB_QUEUE_JOB_QUEUE_H

#include "libpandabase/macros.h"

#include "runtime/include/method.h"
#include "runtime/thread_pool.h"

#include "verification/absint/panda_types.h"
#include "verification/jobs/cache.h"
#include "verification/jobs/job.h"
#include "verification/util/optional_ref.h"

#include <atomic>

namespace panda::verifier {

class Task final : public TaskInterface {
public:
    DEFAULT_COPY_SEMANTIC(Task);
    DEFAULT_MOVE_SEMANTIC(Task);

    Task() : opt_method_ {} {}

    explicit Task(Method &method) : opt_method_ {method} {}

    bool IsEmpty() const
    {
        return !opt_method_.HasRef();
    }

    Method &GetMethod()
    {
        return opt_method_.Get();
    }

private:
    OptionalRef<Method> opt_method_;
};

class TaskQueue final : public TaskQueueInterface<Task> {
public:
    explicit TaskQueue(mem::InternalAllocatorPtr allocator)
        : TaskQueueInterface<Task>(std::numeric_limits<size_t>::max()), queue_(allocator->Adapter())
    {
    }

    ~TaskQueue() = default;
    NO_COPY_SEMANTIC(TaskQueue);
    NO_MOVE_SEMANTIC(TaskQueue);

    Task GetTask() override
    {
        if (queue_.empty()) {
            LOG(DEBUG, VERIFIER) << "No jobs in the verifier queue";
            return {};
        }

        Task task {std::move(queue_.front())};
        queue_.pop_front();
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(Task task, [[maybe_unused]] size_t priority = 0) override
    {
        queue_.push_back(std::move(task));
    }

    void Finalize() override
    {
        queue_.clear();
    }

protected:
    size_t GetQueueSize() override
    {
        return queue_.size();
    }

private:
    PandaDeque<Task> queue_;
};

class Processor final : public ProcessorInterface<Task, std::monostate> {
public:
    explicit Processor(std::monostate dummy)
        // Atomic with acq_rel order reason: could be relaxed, but we need to allow reinitialization for tests
        : types_ {next_thread_num_.fetch_add(1, std::memory_order_acq_rel)}
    {
        // GCC 8 doesn't accept [[maybe_unused]] in this particular case
        (void)dummy;
    }
    ~Processor() = default;
    NO_COPY_SEMANTIC(Processor);
    NO_MOVE_SEMANTIC(Processor);

    bool Init() override;
    bool Process(Task task) override;
    bool Destroy() override;

private:
    PandaTypes types_;
    inline static std::atomic<ThreadNum> next_thread_num_ {0};
    friend class ThreadPool;
};

class ThreadPool {
public:
    NO_COPY_SEMANTIC(ThreadPool);
    NO_MOVE_SEMANTIC(ThreadPool);

    static void Initialize(mem::InternalAllocatorPtr allocator, size_t num_threads);
    static void Destroy();

    static OptionalRef<LibCache> GetCache()
    {
        Data *data = GetData();
        if (data == nullptr) {
            return {};
        }
        return data->cache;
    }

    static bool Enqueue(Method *method);

    // TODO(romanov) Current API implies waking everyone waiting for a method when _any_ method is verified.
    // This means a lot of unnecessary wakeups. It would be good to reduce their number, but need to decide how exactly.
    // E.g. pass Method& here and to SignalMethodVerified? Include a condition variable to wait on in a Job?
    template <typename Handler, typename FailureHandler>
    static void WaitForVerification(Handler &&continue_waiting, FailureHandler &&failure_handler)
    {
        Data *data = GetData();
        if (data == nullptr) {
            return;
        }
        panda::os::memory::LockHolder lck {data->lock};
        while (continue_waiting()) {
            if (GetData() == nullptr) {
                failure_handler();
                return;
            }
            constexpr uint64_t quantum = 500;
            data->cond_var.TimedWait(&data->lock, quantum);
        }
    }
    static void SignalMethodVerified();

private:
    struct Data {
        LibCache cache;
        TaskQueue queue;
        panda::ThreadPool<Task, Processor, std::monostate> thread_pool;
        panda::os::memory::Mutex lock;
        panda::os::memory::ConditionVariable cond_var GUARDED_BY(lock);

        NO_COPY_SEMANTIC(Data);
        NO_MOVE_SEMANTIC(Data);
        Data(mem::InternalAllocatorPtr allocator, size_t num_threads)
            : cache {}, queue {allocator}, thread_pool {allocator, &queue, std::monostate {}, num_threads, "verifier"}
        {
        }
    };

    static Data *GetData(bool allow_shutting_down = false)
    {
        // Atomic with acquire order reason: data race with shutdown_ with dependecies on reads after the load which
        // should become visible
        if (!allow_shutting_down && shutdown_.load(std::memory_order_acquire)) {
            return nullptr;
        }
        // Atomic with seq_cst order reason: data race with data_ with requirement for sequentially consistent order
        // where threads observe all modifications in the same order
        return data_.load(std::memory_order_seq_cst);
    }

    inline static mem::InternalAllocatorPtr allocator_ {nullptr};
    inline static std::atomic<Data *> data_ {nullptr};
    inline static std::atomic<bool> shutdown_ {false};

    friend class Processor;
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_JOB_QUEUE_JOB_QUEUE_H
