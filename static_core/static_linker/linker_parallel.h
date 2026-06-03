/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_LINKER_PARALLEL_H
#define PANDA_LINKER_PARALLEL_H

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"

namespace ark::static_linker {

constexpr int STRTOL_BASE = 10;
constexpr unsigned K_FALLBACK = 4U;
constexpr unsigned K_HARD_CAP = 16U;
constexpr size_t DEFAULT_PARALLEL_MIN_WORK_PER_THREAD = 4096U;

inline unsigned ParseParallelismFromEnv()
{
    const char *e = std::getenv("PANDA_LINKER_THREADS");
    if (e == nullptr) {
        return 0U;
    }
    char *endp = nullptr;
    const int64_t v = std::strtoll(e, &endp, STRTOL_BASE);
    if (endp == e || v < 0) {
        return 0U;
    }
    if (v == 0) {
        return 1U;
    }
    return static_cast<unsigned>(std::min<int64_t>(v, K_HARD_CAP));
}

inline unsigned GetParallelism()
{
    static const unsigned CACHED = []() -> unsigned {
        const unsigned fromEnv = ParseParallelismFromEnv();
        if (fromEnv != 0U) {
            return fromEnv;
        }
        unsigned hc = std::thread::hardware_concurrency();
        if (hc == 0U) {
            hc = K_FALLBACK;
        }
        return std::min(hc, K_HARD_CAP);
    }();
    return CACHED;
}

inline bool IsParallelEnabled()
{
    return GetParallelism() > 1U;
}

class LinkerThreadPool {
public:
    NO_COPY_SEMANTIC(LinkerThreadPool);
    NO_MOVE_SEMANTIC(LinkerThreadPool);

    static LinkerThreadPool &Instance()
    {
        static LinkerThreadPool inst;
        return inst;
    }

    template <typename Fn>
    void RunAll(size_t count, Fn &&fn)
    {
        if (count == 0U) {
            return;
        }
        if (workers_.empty() || count == 1U) {
            for (size_t i = 0; i < count; ++i) {
                fn(i);
            }
            return;
        }

        std::atomic<size_t> remaining {count};
        os::memory::Mutex doneMtx;
        os::memory::ConditionVariable doneCv;

        auto wrap = [&fn, &remaining, &doneMtx, &doneCv](size_t idx) {
            fn(idx);
            // Atomic with acq_rel order reason: publish task completion and synchronize with waiter on last task
            if (remaining.fetch_sub(1U, std::memory_order_acq_rel) == 1U) {
                os::memory::LockHolder lh(doneMtx);
                doneCv.Signal();
            }
        };

        {
            os::memory::LockHolder lh(taskMtx_);
            for (size_t i = 0; i < count; ++i) {
                tasks_.emplace([wrap, i]() { wrap(i); });
            }
        }
        taskCv_.SignalAll();

        doneMtx.Lock();
        // Atomic with acquire order reason: synchronize with fetch_sub release when all tasks complete
        while (remaining.load(std::memory_order_acquire) != 0U) {
            doneCv.Wait(&doneMtx);
        }
        doneMtx.Unlock();
    }

    size_t Size() const noexcept
    {
        return workers_.size();
    }

private:
    LinkerThreadPool()
    {
        const unsigned par = GetParallelism();
        if (par <= 1U) {
            return;
        }
        workers_.reserve(par);
        for (unsigned i = 0; i < par; ++i) {
            workers_.emplace_back([this]() { WorkerLoop(); });
        }
    }

    ~LinkerThreadPool()
    {
        {
            os::memory::LockHolder lh(taskMtx_);
            stop_ = true;
        }
        taskCv_.SignalAll();
        for (auto &t : workers_) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    void WorkerLoop()
    {
        // CC-OFFNXT(G.CTL.03): worker thread runs until stop is requested and task queue is empty
        while (true) {
            std::function<void()> task;
            taskMtx_.Lock();
            while (!stop_ && tasks_.empty()) {
                taskCv_.Wait(&taskMtx_);
            }
            if (stop_ && tasks_.empty()) {
                taskMtx_.Unlock();
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
            taskMtx_.Unlock();
            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    os::memory::Mutex taskMtx_;
    os::memory::ConditionVariable taskCv_;
    bool stop_ {false};
};

inline size_t GetParallelChunkCount(size_t total, size_t minWorkPerThread = DEFAULT_PARALLEL_MIN_WORK_PER_THREAD)
{
    if (total == 0U) {
        return 0U;
    }
    auto numThreads = static_cast<size_t>(GetParallelism());
    if (numThreads > total) {
        numThreads = total;
    }
    if (numThreads <= 1U || total < minWorkPerThread * 2U || !IsParallelEnabled()) {
        return 1U;
    }
    const size_t chunkSize = (total + numThreads - 1U) / numThreads;
    return (total + chunkSize - 1U) / chunkSize;
}

template <typename Fn>
void ParallelForChunks(size_t begin, size_t end, Fn &&fn,
                       size_t minWorkPerThread = DEFAULT_PARALLEL_MIN_WORK_PER_THREAD)
{
    if (end <= begin) {
        return;
    }
    const size_t total = end - begin;
    if (GetParallelChunkCount(total, minWorkPerThread) <= 1U) {
        fn(size_t {0}, begin, end);
        return;
    }

    auto numThreads = static_cast<size_t>(GetParallelism());
    if (numThreads > total) {
        numThreads = total;
    }
    const size_t chunkSize = (total + numThreads - 1U) / numThreads;
    const size_t actualChunks = (total + chunkSize - 1U) / chunkSize;

    auto &pool = LinkerThreadPool::Instance();
    if (pool.Size() == 0U) {
        fn(size_t {0}, begin, end);
        return;
    }

    pool.RunAll(actualChunks, [&fn, begin, end, chunkSize](size_t t) {
        const size_t b = begin + t * chunkSize;
        const size_t e = std::min(end, b + chunkSize);
        if (b < e) {
            fn(t, b, e);
        }
    });
}

template <typename Fn>
void ParallelForDynamic(size_t begin, size_t end, Fn &&fn, size_t minWork = 8U)
{
    if (end <= begin) {
        return;
    }
    const size_t total = end - begin;
    if (total < minWork || !IsParallelEnabled()) {
        for (size_t i = begin; i < end; ++i) {
            fn(i);
        }
        return;
    }
    auto &pool = LinkerThreadPool::Instance();
    if (pool.Size() == 0U) {
        for (size_t i = begin; i < end; ++i) {
            fn(i);
        }
        return;
    }
    pool.RunAll(total, [&fn, begin](size_t i) { fn(begin + i); });
}

}  // namespace ark::static_linker

#endif  // PANDA_LINKER_PARALLEL_H
