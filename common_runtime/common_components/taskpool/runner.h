/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_COMPONENTS_TASKPOOL_RUNNER_H
#define COMMON_COMPONENTS_TASKPOOL_RUNNER_H

#include <array>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

#include "common_components/taskpool/task_queue.h"
#include "common_interfaces/base/common.h"

namespace common {
static constexpr uint32_t MIN_TASKPOOL_THREAD_NUM = 3;
static constexpr uint32_t MAX_TASKPOOL_THREAD_NUM = 5;
static constexpr uint32_t DEFAULT_TASKPOOL_THREAD_NUM = 0;
using native_handle_type = std::thread::native_handle_type;

enum class PriorityMode {
    STW,
    FOREGROUND,
    BACKGROUND
};

class Runner {
public:
    explicit Runner(uint32_t threadNum,
        const std::function<void(native_handle_type)> prologueHook,
        const std::function<void(native_handle_type)> epilogueHook);
    ~Runner() = default;

    NO_COPY_SEMANTIC_CC(Runner);
    NO_MOVE_SEMANTIC_CC(Runner);

    void PostTask(std::unique_ptr<Task> task)
    {
        taskQueue_.PostTask(std::move(task));
    }

    void PostDelayedTask(std::unique_ptr<Task> task, uint64_t delayMilliseconds)
    {
        taskQueue_.PostDelayedTask(std::move(task), delayMilliseconds);
    }

    void PUBLIC_API TerminateThread();
    void TerminateTask(int32_t id, TaskType type);
    void SetQosPriority(PriorityMode mode);
    void RecordThreadId();

    uint32_t GetTotalThreadNum() const
    {
        return totalThreadNum_;
    }

    bool IsInThreadPool(std::thread::id id)
    {
        std::lock_guard<std::mutex> guard(mtxPool_);
        for (auto &thread : threadPool_) {
            if (thread->get_id() == id) {
                return true;
            }
        }
        return false;
    }

    void PrologueHook(native_handle_type thread)
    {
        if (prologueHook_ != nullptr) {
            prologueHook_(thread);
        }
    }
    void EpilogueHook(native_handle_type thread)
    {
        if (epilogueHook_ != nullptr) {
            epilogueHook_(thread);
        }
    }
    void ForEachTask(const std::function<void(Task*)> &f);

private:
    void Run(uint32_t threadId);
    void SetRunTask(uint32_t threadId, Task *task);

    std::vector<std::unique_ptr<std::thread>> threadPool_ {};
    TaskQueue taskQueue_ {};
    std::array<Task*, MAX_TASKPOOL_THREAD_NUM + 1> runningTask_;
    uint32_t totalThreadNum_ {0};
    std::vector<uint32_t> gcThreadId_ {};
    std::mutex mtx_;
    std::mutex mtxPool_;

    std::function<void(native_handle_type)> prologueHook_;
    std::function<void(native_handle_type)> epilogueHook_;
};
}  // namespace common
#endif  // COMMON_COMPONENTS_TASKPOOL_RUNNER_H
