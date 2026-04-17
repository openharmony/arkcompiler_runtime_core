/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "common_components/taskpool/task_queue.h"

namespace common_vm {
void TaskQueue::PostTask(std::unique_ptr<Task> task)
{
    ark::os::memory::LockHolder guard(mtx_);
    DCHECK(!terminate_);
    tasks_.push_back(std::move(task));
    cv_.SignalAll();
}

void TaskQueue::PostDelayedTask(std::unique_ptr<Task> task, uint64_t delayMilliseconds)
{
    ark::os::memory::LockHolder guard(mtx_);
    DCHECK(!terminate_);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMilliseconds);
    delayedTasks_.insert({deadline, std::move(task)});
    cv_.SignalAll();
}

std::unique_ptr<Task> TaskQueue::PopTask()
{
    ark::os::memory::LockHolder lock(mtx_);
    while (true) {
        MoveExpiredTask();
        if (!tasks_.empty()) {
            std::unique_ptr<Task> task = std::move(tasks_.front());
            tasks_.pop_front();
            return task;
        }
        if (terminate_) {
            cv_.SignalAll();
            return nullptr;
        }
        WaitForTask();
    }
}

void TaskQueue::TerminateTask(int32_t id, TaskType type)
{
    ark::os::memory::LockHolder guard(mtx_);
    for (auto &task : tasks_) {
        if (id != ALL_TASK_ID && id != task->GetId()) {
            continue;
        }
        if (type != TaskType::ALL && type != task->GetTaskType()) {
            continue;
        }
        task->Terminated();
    }
    for (auto &taskItem : delayedTasks_) {
        if (id != ALL_TASK_ID && id != (taskItem.second)->GetId()) {
            continue;
        }
        if (type != TaskType::ALL && type != (taskItem.second)->GetTaskType()) {
            continue;
        }
        (taskItem.second)->Terminated();
    }
}

void TaskQueue::Terminate()
{
    ark::os::memory::LockHolder guard(mtx_);
    terminate_ = true;
    cv_.SignalAll();
}

void TaskQueue::ForEachTask(const std::function<void(Task *)> &f)
{
    ark::os::memory::LockHolder guard(mtx_);
    for (auto &task : tasks_) {
        if (task.get() != nullptr) {
            f(task.get());
        }
    }
}

void TaskQueue::MoveExpiredTask()
{
    while (!delayedTasks_.empty()) {
        auto it = delayedTasks_.begin();
        auto currentTime = std::chrono::steady_clock::now();
        if ((std::chrono::duration_cast<std::chrono::duration<double>>(it->first - currentTime)).count() > 0) {
            return;
        }
        tasks_.push_back(std::move(it->second));
        delayedTasks_.erase(it);
    }
}

void TaskQueue::WaitForTask()
{
    if (!delayedTasks_.empty()) {
        auto it = delayedTasks_.begin();
        auto currentTime = std::chrono::steady_clock::now();
        if ((std::chrono::duration_cast<std::chrono::duration<double>>(it->first - currentTime)).count() < 0) {
            return;
        }
        auto waitingTime = std::chrono::duration_cast<std::chrono::milliseconds>(it->first - currentTime);
        cv_.TimedWait(&mtx_, waitingTime.count());
    } else {
        cv_.Wait(&mtx_);
    }
}
}  // namespace common_vm
