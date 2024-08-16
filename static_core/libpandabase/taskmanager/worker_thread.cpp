/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "libpandabase/taskmanager/task.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/os/thread.h"

namespace ark::taskmanager {

WorkerThread::WorkerThread(const std::string &name) : scheduler_(TaskScheduler::GetTaskScheduler()), name_(name)
{
    perWorkerPopId_[this] = localQueue_.RegisterConsumer();
    schedulerPopId_ = localQueue_.RegisterConsumer();
    thread_ = new std::thread(&WorkerThread::WorkerLoop, this);
    [[maybe_unused]] auto setNameCore = os::thread::SetThreadName(thread_->native_handle(), name.c_str());
    ASSERT(setNameCore == 0);
}

void WorkerThread::AddTask(Task &&task)
{
    localQueue_.Push(std::move(task));
}

void WorkerThread::Join()
{
    thread_->join();
}

bool WorkerThread::IsEmpty() const
{
    return localQueue_.IsEmpty();
}

size_t WorkerThread::Size() const
{
    return localQueue_.Size();
}

size_t WorkerThread::CountOfTasksWithProperties(TaskProperties properties) const
{
    return localQueue_.CountOfTasksWithProperties(properties);
}

void WorkerThread::WorkerLoop()
{
    auto *scheduler = TaskScheduler::GetTaskScheduler();
    ASSERT(scheduler != nullptr);
    if (scheduler->IsTaskLifetimeStatisticsUsed()) {
        TaskScheduler::GetTaskScheduler()->GetTaskTimeStats()->RegisterWorkerThread();
    }
    WaitForStart();
    auto finishCond = false;
    while (!finishCond) {
        ASSERT(finishedTasksCounterMap_.empty());
        // Worker will steal tasks only if all queues are empty and it's possible to find worker for stealing
        if (UNLIKELY(scheduler_->AreQueuesEmpty() && !scheduler_->AreWorkersEmpty())) {
            scheduler_->StealTaskFromOtherWorker(this);
            if (stolenTask_.IsInvalid()) {
                continue;
            }
            ExecuteStolenTask();
        } else {  // Else it will try get/wait tasks.
            finishCond = scheduler_->FillWithTasks(this);
        }
        ExecuteTasksFromLocalQueue();
        [[maybe_unused]] size_t countOfTasks = scheduler_->IncrementCounterOfExecutedTasks(finishedTasksCounterMap_);
        LOG(DEBUG, TASK_MANAGER) << GetWorkerName() << ": executed tasks: " << countOfTasks;
        countOfExecutedTask_ += countOfTasks;
        finishedTasksCounterMap_.clear();
    }
    LOG(DEBUG, TASK_MANAGER) << GetWorkerName() << " have executed tasks at all: " << countOfExecutedTask_;
}

size_t WorkerThread::ExecuteTasksFromLocalQueue()
{
    // Start popping task from local queue and executing them
    size_t executeTasksCount = 0UL;
    for (; !localQueue_.IsEmpty(); executeTasksCount++) {
        auto task = localQueue_.Pop(perWorkerPopId_[this]);
        // If pop task returned nullopt need to finish execution
        if (UNLIKELY(!task.has_value())) {
            break;
        }
        task->RunTask();
        finishedTasksCounterMap_[task->GetTaskProperties()]++;
    }
    return executeTasksCount;
}

void WorkerThread::ExecuteStolenTask()
{
    ExecuteTask(&stolenTask_);
}

void WorkerThread::ExecuteTask(Task *task)
{
    ASSERT(task != nullptr);
    auto prop = task->GetTaskProperties();
    task->RunTask();
    task->MakeInvalid();
    finishedTasksCounterMap_[prop]++;
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

void WorkerThread::RegisterAllWorkersInLocalQueue(const std::vector<WorkerThread *> &workers)
{
    for (auto *worker : workers) {
        if (worker == this) {
            continue;
        }
        perWorkerPopId_[worker] = localQueue_.RegisterConsumer();
    }
}

std::string WorkerThread::GetWorkerName() const
{
    return name_;
}

size_t WorkerThread::GetLocalWorkerQueuePopId(WorkerThread *worker) const
{
    return perWorkerPopId_.at(worker);
}

size_t WorkerThread::GetLocalWorkerQueueSchedulerPopId() const
{
    return schedulerPopId_;
}

void WorkerThread::SetStolenTask(Task &&stolenTask)
{
    ASSERT(stolenTask_.IsInvalid());
    stolenTask_ = std::move(stolenTask);
}

void WorkerThread::TryDeleteRetiredPtrs()
{
    localQueue_.TryDeleteRetiredPtrs();
}

WorkerThread::~WorkerThread()
{
    delete thread_;
}

}  // namespace ark::taskmanager
