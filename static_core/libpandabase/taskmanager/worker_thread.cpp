/**
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

WorkerThread::WorkerThread(os::memory::Mutex *outside_lock, os::memory::ConditionVariable *outside_cond_var,
                           size_t tasks_count)
    : outside_lock_(outside_lock), outside_cond_var_(outside_cond_var)
{
    thread_ = new std::thread(&WorkerThread::WorkerLoop, this, tasks_count);
    os::thread::SetThreadName(thread_->native_handle(), "TaskSchedulerWorker");
}

void WorkerThread::AddTask(Task &&task)
{
    if (task.GetTaskProperties().GetTaskExecutionMode() == TaskExecutionMode::FOREGROUND) {
        foreground_queue_.push(std::move(task));
    } else {
        background_queue_.push(std::move(task));
    }
    size_++;
}

Task WorkerThread::PopTask()
{
    ASSERT(!(foreground_queue_.empty() && background_queue_.empty()));
    std::queue<Task> *queue = nullptr;
    if (!foreground_queue_.empty()) {
        queue = &foreground_queue_;
    } else {
        queue = &background_queue_;
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

void WorkerThread::WorkerLoop(size_t tasks_count)
{
    while (true) {
        auto finish_cond = TaskScheduler::GetTaskScheduler()->FillWithTasks(this, tasks_count);
        ExecuteTasks();
        if (finish_cond) {
            break;
        }
        // Notify TaskScheduler that all tasks are done
        os::memory::LockHolder lock_holder(*outside_lock_);
        outside_cond_var_->Signal();
    }
}

bool WorkerThread::IsTasksRunning()
{
    return is_tasks_running_;
}

void WorkerThread::ExecuteTasks()
{
    is_tasks_running_ = true;
    while (!IsEmpty()) {
        auto task = PopTask();
        task.RunTask();
    }
    is_tasks_running_ = false;
}

WorkerThread::~WorkerThread()
{
    delete thread_;
}

}  // namespace panda::taskmanager