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
#include "libpandabase/utils/logger.h"
#include "libpandabase/taskmanager/task_statistics/fine_grained_task_statistics_impl.h"
#include "libpandabase/taskmanager/task_statistics/simple_task_statistics_impl.h"

namespace panda::taskmanager {

TaskScheduler *TaskScheduler::instance_ = nullptr;

TaskScheduler::TaskScheduler(size_t workersCount, TaskStatisticsImplType taskStatisticsType)
    : workersCount_(workersCount), gen_(std::random_device()())
{
    switch (taskStatisticsType) {
        case TaskStatisticsImplType::FINE_GRAINED:
            taskStatistics_ = new FineGrainedTaskStatisticsImpl();
            break;
        case TaskStatisticsImplType::SIMPLE:
            taskStatistics_ = new SimpleTaskStatisticsImpl();
            break;
        default:
            UNREACHABLE();
    }
}

/* static */
TaskScheduler *TaskScheduler::Create(size_t threadsCount, TaskStatisticsImplType taskStatisticsType)
{
    ASSERT(instance_ == nullptr);
    ASSERT(threadsCount > 0);
    instance_ = new TaskScheduler(threadsCount, taskStatisticsType);
    return instance_;
}

/* static */
TaskScheduler *TaskScheduler::GetTaskScheduler()
{
    return instance_;
}

/* static */
void TaskScheduler::Destroy()
{
    ASSERT(instance_ != nullptr);
    delete instance_;
    instance_ = nullptr;
}

TaskQueueId TaskScheduler::RegisterQueue(internal::SchedulableTaskQueueInterface *queue)
{
    os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
    ASSERT(!start_);
    TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
    if (taskQueues_.find(id) != taskQueues_.end()) {
        return INVALID_TASKQUEUE_ID;
    }
    taskQueues_[id] = queue;
    queue->SetNewTasksCallback([this](TaskProperties properties, size_t count, bool wasEmpty) {
        this->IncrementCounterOfAddedTasks(properties, count, wasEmpty);
    });
    return id;
}

void TaskScheduler::Initialize()
{
    ASSERT(!start_);
    start_ = true;
    LOG(DEBUG, RUNTIME) << "TaskScheduler: creates " << workersCount_ << " threads";
    for (size_t i = 0; i < workersCount_; i++) {
        workers_.push_back(new WorkerThread(
            [this](const TaskPropertiesCounterMap &counterMap) { this->IncrementCounterOfExecutedTasks(counterMap); }));
    }
}

bool TaskScheduler::FillWithTasks(WorkerThread *worker, size_t tasksCount)
{
    ASSERT(start_);
    os::memory::LockHolder workerLockHolder(workersLock_);
    LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks";
    {
        os::memory::LockHolder taskSchedulerLockHolder(taskSchedulerStateLock_);
        while (AreQueuesEmpty()) {
            /// We exit in situation when finish_ = true .TM Finalize, Worker wake up and go finish WorkerLoop.
            if (finish_) {
                LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks: return queue without issues";
                return true;
            }
            queuesWaitCondVar_.Wait(&taskSchedulerStateLock_);
        }
    }
    os::memory::LockHolder popLockHolder(popFromTaskQueuesLock_);
    SelectNextTasks(tasksCount);
    PutTasksInWorker(worker);
    LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks: return queue with tasks";
    return false;
}

void TaskScheduler::SelectNextTasks(size_t tasksCount)
{
    LOG(DEBUG, RUNTIME) << "TaskScheduler: SelectNextTasks()";
    if (AreQueuesEmpty()) {
        return;
    }
    UpdateKineticPriorities();
    // Now kinetic_priorities_
    size_t kineticMax = 0;
    std::tie(kineticMax, std::ignore) = *kineticPriorities_.rbegin();  // Get key of the last element in map
    std::uniform_int_distribution<size_t> distribution(0U, kineticMax - 1U);

    for (size_t i = 0; i < tasksCount; i++) {
        size_t choice = distribution(gen_);  // Get random number in range [0, kinetic_max)
        internal::SchedulableTaskQueueInterface *queue = nullptr;
        std::tie(std::ignore, queue) = *kineticPriorities_.upper_bound(choice);  // Get queue of chosen element

        TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
        selectedQueues_[id]++;
    }
}

void TaskScheduler::UpdateKineticPriorities()
{
    ASSERT(!taskQueues_.empty());  // no TaskQueues
    size_t kineticSum = 0;
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    for (const auto &idQueuePair : taskQueues_) {
        std::tie(std::ignore, queue) = idQueuePair;
        if (queue->IsEmpty()) {
            continue;
        }
        kineticSum += queue->GetPriority();
        kineticPriorities_[kineticSum] = queue;
    }
}

size_t TaskScheduler::PutTasksInWorker(WorkerThread *worker)
{
    size_t taskCount = 0;
    for (auto [id, size] : selectedQueues_) {
        if (size == 0) {
            continue;
        }
        auto addTaskFunc = [worker](Task &&task) { worker->AddTask(std::move(task)); };
        size_t queueTaskCount = taskQueues_[id]->PopTasksToWorker(addTaskFunc, size);
        taskCount += queueTaskCount;
        LOG(DEBUG, RUNTIME) << "PutTasksInWorker: worker have gotten " << queueTaskCount << " tasks";
    }
    selectedQueues_.clear();
    return taskCount;
}

bool TaskScheduler::AreQueuesEmpty() const
{
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    for (const auto &traitsQueuePair : taskQueues_) {
        std::tie(std::ignore, queue) = traitsQueuePair;
        if (!queue->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool TaskScheduler::AreNoMoreTasks() const
{
    return taskStatistics_->GetCountOfTaskInSystem() == 0;
}

std::optional<Task> TaskScheduler::GetTaskFromQueue(TaskProperties properties)
{
    LOG(DEBUG, RUNTIME) << "TaskScheduler: GetTaskFromQueue()";
    os::memory::LockHolder popLockHolder(popFromTaskQueuesLock_);
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    {
        os::memory::LockHolder taskManagerLockHolder(taskSchedulerStateLock_);
        auto taskQueuesIterator = taskQueues_.find({properties.GetTaskType(), properties.GetVMType()});
        if (taskQueuesIterator == taskQueues_.end()) {
            if (finish_) {
                return std::nullopt;
            }
            LOG(FATAL, COMMON) << "Attempt to take a task from a non-existent queue";
        }
        std::tie(std::ignore, queue) = *taskQueuesIterator;
    }
    if (!queue->HasTaskWithExecutionMode(properties.GetTaskExecutionMode())) {
        return std::nullopt;
    }
    // Now we can pop the task from the chosen queue
    auto task = queue->PopTask();
    // Only after popping we can notify task statistics that task was POPPED from queue to get it outside. This sequence
    // ensures that the number of tasks in the system (see TaskStatistics) will be correct.
    taskStatistics_->IncrementCount(TaskStatus::POPPED, properties, 1);
    if (taskStatistics_->GetCountOfTasksInSystemWithTaskProperties(properties) == 0) {
        os::memory::LockHolder taskManagerLockHolder(taskSchedulerStateLock_);
        finishTasksCondVar_.SignalAll();
    }
    return task;
}

void TaskScheduler::WaitForFinishAllTasksWithProperties(TaskProperties properties)
{
    os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
    while (taskStatistics_->GetCountOfTasksInSystemWithTaskProperties(properties) != 0) {
        finishTasksCondVar_.Wait(&taskSchedulerStateLock_);
    }
    LOG(DEBUG, RUNTIME) << "After waiting tasks with properties: " << properties
                        << " {added: " << taskStatistics_->GetCount(TaskStatus::ADDED, properties)
                        << ", executed: " << taskStatistics_->GetCount(TaskStatus::EXECUTED, properties)
                        << ", popped: " << taskStatistics_->GetCount(TaskStatus::POPPED, properties) << "}";
    taskStatistics_->ResetCountersWithTaskProperties(properties);
}

void TaskScheduler::Finalize()
{
    ASSERT(start_);
    {
        // Wait all tasks will be done
        os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
        while (!AreNoMoreTasks()) {
            finishTasksCondVar_.Wait(&taskSchedulerStateLock_);
        }
        finish_ = true;
        queuesWaitCondVar_.Signal();
    }
    for (auto worker : workers_) {
        worker->Join();
        delete worker;
    }
    taskStatistics_->ResetAllCounters();
    LOG(DEBUG, RUNTIME) << "TaskScheduler: Finalized";
}

void TaskScheduler::IncrementCounterOfAddedTasks(TaskProperties properties, size_t ivalue, bool wasEmpty)
{
    taskStatistics_->IncrementCount(TaskStatus::ADDED, properties, ivalue);
    if (wasEmpty) {
        os::memory::LockHolder outsideLockHolder(taskSchedulerStateLock_);
        queuesWaitCondVar_.Signal();
    }
}

void TaskScheduler::IncrementCounterOfExecutedTasks(const TaskPropertiesCounterMap &counterMap)
{
    for (const auto &[properties, count] : counterMap) {
        taskStatistics_->IncrementCount(TaskStatus::EXECUTED, properties, count);
        if (taskStatistics_->GetCountOfTasksInSystemWithTaskProperties(properties) == 0) {
            os::memory::LockHolder outsideLockHolder(taskSchedulerStateLock_);
            finishTasksCondVar_.SignalAll();
        }
    }
}

TaskScheduler::~TaskScheduler()
{
    // We can delete TaskScheduler if it wasn't started or it was finished
    ASSERT(start_ == finish_);
    // Check if all task queue was deleted
    ASSERT(taskQueues_.empty());
    delete taskStatistics_;
    LOG(DEBUG, RUNTIME) << "TaskScheduler: ~TaskScheduler: All threads finished jobs";
}

}  // namespace panda::taskmanager
