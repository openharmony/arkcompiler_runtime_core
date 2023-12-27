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

#include "libpandabase/taskmanager/task_scheduler.h"
#include "libpandabase/os/thread.h"

namespace panda::taskmanager {

WorkerThread::WorkerThread(FinishedTasksCallback callback, size_t tasksCount)
    : finishedTasksCallback_(std::move(callback))
{
    thread_ = new std::thread(&WorkerThread::WorkerLoop, this, tasksCount);
    os::thread::SetThreadName(thread_->native_handle(), "TaskSchedulerWorker");
}

void WorkerThread::AddTask(Task &&task)
{
    if (task.GetTaskProperties().GetTaskExecutionMode() == TaskExecutionMode::FOREGROUND) {
        foregroundQueue_.push(std::move(task));
    } else {
        backgroundQueue_.push(std::move(task));
    }
    size_++;
}

Task WorkerThread::PopTask()
{
    ASSERT(!IsEmpty());
    std::queue<Task> *queue = nullptr;
    if (!foregroundQueue_.empty()) {
        queue = &foregroundQueue_;
    } else {
        queue = &backgroundQueue_;
    }
    auto task = std::move(queue->front());
    queue->pop();
    size_--;
    return task;
}

void WorkerThread::Join()
{
    thread_->join();
}

bool WorkerThread::IsEmpty() const
{
    return size_ == 0;
}

void WorkerThread::WorkerLoop(size_t tasksCount)
{
    while (true) {
        auto finishCond = TaskScheduler::GetTaskScheduler()->FillWithTasks(this, tasksCount);
        ExecuteTasks();
        if (finishCond) {
            break;
        }
        finishedTasksCallback_(finishedTasksCounterMap_);
        finishedTasksCounterMap_.clear();
    }
}

void WorkerThread::ExecuteTasks()
{
    while (!IsEmpty()) {
        auto task = PopTask();
        task.RunTask();
        finishedTasksCounterMap_[task.GetTaskProperties()]++;
    }
}

WorkerThread::~WorkerThread()
{
    delete thread_;
}

}  // namespace panda::taskmanager
