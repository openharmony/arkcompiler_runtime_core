/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "libpandabase/taskmanager/task_scheduler.h"
#include "libpandabase/taskmanager/task_queue_set.h"
#include "libpandabase/taskmanager/task.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/os/thread.h"

namespace ark::taskmanager::internal {

WorkerThread::WorkerThread(TaskScheduler *scheduler, TaskTimeStatsBase *taskTimeStats, const std::string &name)
    : scheduler_(scheduler), taskTimeStats_(taskTimeStats), name_(name)
{
    thread_ = new std::thread(&WorkerThread::WorkerLoop, this);
    [[maybe_unused]] auto setNameCore = os::thread::SetThreadName(thread_->native_handle(), name.c_str());
    ASSERT_PRINT(setNameCore == 0, setNameCore);
}

void WorkerThread::AddForegroundTask(TaskPtr task)
{
    foregroundTaskQueue_.Push(std::move(task));
}

void WorkerThread::AddBackgroundTask(TaskPtr task)
{
    backgroundTaskQueue_.Push(std::move(task));
}

void WorkerThread::Join()
{
    thread_->join();
}

bool WorkerThread::IsEmpty() const
{
    return foregroundTaskQueue_.IsEmpty() && backgroundTaskQueue_.IsEmpty();
}

size_t WorkerThread::Size() const
{
    return foregroundTaskQueue_.Size() + backgroundTaskQueue_.Size();
}

TaskPtr WorkerThread::PopTask()
{
    TaskPtr task;
    if (foregroundTaskQueue_.TryPop(&task)) {
        return task;
    }
    backgroundTaskQueue_.TryPop(&task);
    return task;
}

TaskPtr WorkerThread::PopForegroundTask()
{
    TaskPtr task;
    foregroundTaskQueue_.TryPop(&task);
    return task;
}

TaskPtr WorkerThread::PopBackgroundTask()
{
    TaskPtr task;
    backgroundTaskQueue_.TryPop(&task);
    return task;
}

void WorkerThread::WorkerLoop()
{
    if (taskTimeStats_ != nullptr) {
        taskTimeStats_->RegisterWorkerThread();
    }
    auto finishCond = false;
    while (!finishCond) {
        finishCond = scheduler_->FillWithTasks(this);
        ExecuteTasksFromLocalQueue();
    }
}

size_t WorkerThread::ExecuteTasksFromLocalQueue()
{
    size_t executed = 0;
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        auto task = PopTask();
        if (task == nullptr) {
            break;
        }
        task->RunTask();
        executed++;
    }
    return executed;
}

void WorkerThread::Start()
{
    os::memory::LockHolder<os::memory::Mutex> lockHolder(startWaitLock_);
    start_ = true;
    startWaitCondVar_.SignalAll();
}

void WorkerThread::WaitForStart()
{
    os::memory::LockHolder<os::memory::Mutex> lockHolder(startWaitLock_);
    while (!start_) {
        startWaitCondVar_.Wait(&startWaitLock_);
    }
}

std::string WorkerThread::GetWorkerName() const
{
    return name_;
}

WorkerThread::~WorkerThread()
{
    delete thread_;
}

}  // namespace ark::taskmanager::internal
