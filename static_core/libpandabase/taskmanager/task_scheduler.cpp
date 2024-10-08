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
#include "libpandabase/utils/logger.h"

namespace ark::taskmanager {

TaskScheduler *TaskScheduler::instance_ = nullptr;

TaskScheduler::TaskScheduler(size_t workersCount, TaskTimeStatsType taskTimeStatsType)
    : workersCount_(workersCount), taskTimeStatsType_(taskTimeStatsType), selector_(taskQueues_)
{
    switch (taskTimeStatsType) {
        case TaskTimeStatsType::LIGHT_STATISTICS:
            taskTimeStats_ = new internal::LightTaskTimeTimeStats(workersCount);
            break;
        case TaskTimeStatsType::NO_STATISTICS:
            break;
        default:
            UNREACHABLE();
    }
}

/* static */
TaskScheduler *TaskScheduler::Create(size_t threadsCount, TaskTimeStatsType taskTimeStatsType)
{
    ASSERT(instance_ == nullptr);
    ASSERT(threadsCount > 0);
    instance_ = new TaskScheduler(threadsCount, taskTimeStatsType);
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
    LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: Register task queue with {" << queue->GetTaskType() << ", "
                             << queue->GetVMType() << "}";
    TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
    if (UNLIKELY(taskQueues_.find(id) != taskQueues_.end())) {
        return INVALID_TASKQUEUE_ID;
    }
    taskQueues_[id] = queue;
    queue->SetCallbacks(
        [this](TaskProperties properties, size_t count) { this->IncrementCounterOfAddedTasks(properties, count); },
        [this]() { this->SignalWorkers(); });

    // init countOfTasksIsSystem_ for possible task from registered queue
    countOfTasksInSystem_[{queue->GetTaskType(), queue->GetVMType(), TaskExecutionMode::FOREGROUND}] = 0U;
    countOfTasksInSystem_[{queue->GetTaskType(), queue->GetVMType(), TaskExecutionMode::BACKGROUND}] = 0U;

    return id;
}

void TaskScheduler::Initialize()
{
    ASSERT(!start_);
    selector_.Init();
    start_ = true;
    LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: creates " << workersCount_ << " threads";
    // Starts all workers
    for (size_t i = 0; i < workersCount_; i++) {
        workers_.push_back(new WorkerThread("TSWorker_" + std::to_string(i + 1UL)));
        LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: created thread with name " << workers_.back()->GetWorkerName();
    }
    // Set names of workers and get them info about other ones
    for (auto *worker : workers_) {
        worker->RegisterAllWorkersInLocalQueue(workers_);
    }
    // Start worker loop executing
    for (auto *worker : workers_) {
        worker->Start();
    }
    // Atomic with release order reason: other thread should see last value
    disableHelpers_.store(false, std::memory_order_release);
}

void TaskScheduler::StealTaskFromOtherWorker(WorkerThread *taskReceiver)
{
    // WorkerThread tries to find Worker with the most tasks
    auto chosenWorker =
        *std::max_element(workers_.begin(), workers_.end(),
                          [](const WorkerThread *lv, const WorkerThread *rv) { return lv->Size() < rv->Size(); });
    if (chosenWorker->Size() == 0) {
        return;
    }
    // If worker was successfully found, steals task from its local queue
    chosenWorker->GiveTasksToAnotherWorker(
        [taskReceiver](Task &&task) { taskReceiver->SetStolenTask(std::move(task)); }, 1UL,
        chosenWorker->GetLocalWorkerQueuePopId(taskReceiver));
}

bool TaskScheduler::FillWithTasks(WorkerThread *worker)
{
    ASSERT(start_);
    std::queue<Task> readyTasks;
    {
        os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
        // We increment counter of waiters to signal them in future
        if (AreQueuesEmpty() && !waitList_.HaveReadyValue()) {
            if (!WaitUntilNewTasks()) {
                return true;
            }
        }
        if (!waitList_.HaveReadyValue()) {
            // Use selector to choose next queue to pop task
            auto selectedQueue = selector_.SelectQueue();
            ASSERT(selectedQueue != INVALID_TASKQUEUE_ID);
            // Getting task from selected queue
            PutTasksInWorker(worker, selectedQueue);
            return false;
        }
        // Wait list have task that should be added in queues. So worker firstly should add this task in local queue
        // under lock-holder. Next in should add them in TaskQueues
        PutWaitTaskInLocalQueue(readyTasks);
    }
    // Worker puts tasks to TaskQueues without lock-holder to avoid unnecessary waiting of other workers
    PutTaskInTaskQueues(readyTasks);
    return false;
}

bool TaskScheduler::WaitUntilNewTasks()
{
    // Atomic with acq_rel order reason: sync for counter
    waitWorkersCount_.fetch_add(1, std::memory_order_acq_rel);
    while (AreQueuesEmpty() && !waitList_.HaveReadyValue() && !finish_) {
        queuesWaitCondVar_.TimedWait(&taskSchedulerStateLock_, TASK_WAIT_TIMEOUT);
    }
    // Atomic with acq_rel order reason: sync for counter
    waitWorkersCount_.fetch_sub(1, std::memory_order_acq_rel);
    return !finish_;
}

size_t TaskScheduler::PutTasksInWorker(WorkerThread *worker, TaskQueueId selectedQueue)
{
    auto addTaskFunc = [worker](Task &&task) { worker->AddTask(std::move(task)); };
    auto queue = taskQueues_[selectedQueue];

    // Now we calc how many task we want to get from queue. If there are few tasks, then we want them to be evenly
    // distributed among the workers.
    size_t size = queue->Size();
    size_t countToGet = size / workers_.size();
    countToGet = (countToGet >= WorkerThread::WORKER_QUEUE_SIZE) ? WorkerThread::WORKER_QUEUE_SIZE
                 : (size % workers_.size() == 0)                 ? countToGet
                                                                 : countToGet + 1;
    // Firstly we use method to delete retired ptrs
    worker->TryDeleteRetiredPtrs();
    // Execute popping task form queue
    size_t queueTaskCount = queue->PopTasksToWorker(addTaskFunc, countToGet);
    LOG(DEBUG, TASK_MANAGER) << worker->GetWorkerName() << ": get tasks " << queueTaskCount << "; ";
    return queueTaskCount;
}

bool TaskScheduler::AreQueuesEmpty() const
{
    for ([[maybe_unused]] const auto &[id, queue] : taskQueues_) {
        ASSERT(queue != nullptr);
        if (!queue->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool TaskScheduler::AreWorkersEmpty() const
{
    for (auto *worker : workers_) {
        if (!worker->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool TaskScheduler::AreNoMoreTasks() const
{
    return GetCountOfTasksInSystem() == 0;
}

size_t TaskScheduler::HelpWorkersWithTasks(TaskProperties properties)
{
    // Atomic with acquire order reason: getting correct value
    if (disableHelpers_.load(std::memory_order_acquire)) {
        return 0;
    }
    size_t executedTasksCount = 0;
    auto *queue = GetQueue({properties.GetTaskType(), properties.GetVMType()});
    if (queue->HasTaskWithExecutionMode(properties.GetTaskExecutionMode())) {
        executedTasksCount = GetAndExecuteSetOfTasksFromQueue(properties);
    } else if (LIKELY(!workers_.empty())) {
        executedTasksCount = StealAndExecuteOneTaskFromWorkers(properties);
    }
    if (UNLIKELY(executedTasksCount == 0)) {
        LOG(DEBUG, TASK_MANAGER) << "Helper: got no tasks;";
        return 0;
    }
    LOG(DEBUG, TASK_MANAGER) << "Helper: executed tasks: " << executedTasksCount << ";";
    DecrementCountOfTasksInSystem(properties, executedTasksCount);

    // Atomic with acquire order reason: get correct value
    auto waitToFinish = waitToFinish_.load(std::memory_order_acquire);
    if (waitToFinish > 0 && GetCountOfTasksInSystemWithTaskProperties(properties) == 0) {
        os::memory::LockHolder taskManagerLockHolder(taskSchedulerStateLock_);
        finishTasksCondVar_.SignalAll();
    }
    return executedTasksCount;
}

size_t TaskScheduler::GetAndExecuteSetOfTasksFromQueue(TaskProperties properties)
{
    auto *queue = GetQueue({properties.GetTaskType(), properties.GetVMType()});
    if (queue->IsEmpty()) {
        return 0;
    }

    std::queue<Task> taskQueue;
    size_t realCount = 0;
    {
        os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
        size_t size = queue->CountOfTasksWithExecutionMode(properties.GetTaskExecutionMode());
        size_t countToGet = size / (workers_.size() + 1);
        countToGet = (countToGet >= WorkerThread::WORKER_QUEUE_SIZE) ? WorkerThread::WORKER_QUEUE_SIZE
                     : (size % (workers_.size() + 1) == 0)           ? countToGet
                                                                     : countToGet + 1;
        realCount = queue->PopTasksToHelperThread([&taskQueue](Task &&task) { taskQueue.push(std::move(task)); },
                                                  countToGet, properties.GetTaskExecutionMode());
    }
    while (!taskQueue.empty()) {
        taskQueue.front().RunTask();
        taskQueue.pop();
    }
    return realCount;
}

size_t TaskScheduler::StealAndExecuteOneTaskFromWorkers(TaskProperties properties)
{
    ASSERT(!workers_.empty());
    std::queue<Task> taskQueue;
    auto addTaskToQueue = [&taskQueue](Task &&task) { taskQueue.push(std::move(task)); };
    while (true) {
        auto chosenWorker = *std::max_element(
            workers_.begin(), workers_.end(), [&properties](const WorkerThread *lv, const WorkerThread *rv) {
                return lv->CountOfTasksWithProperties(properties) < rv->CountOfTasksWithProperties(properties);
            });
        if UNLIKELY (chosenWorker->CountOfTasksWithProperties(properties) == 0) {
            return 0;
        }

        auto stolen = chosenWorker->GiveTasksToAnotherWorker(
            addTaskToQueue, 1UL, chosenWorker->GetLocalWorkerQueueSchedulerPopId(), properties);
        if (stolen == 0) {  // check if stealing was successful
            // if we did not stole we should retry
            continue;
        }
        while (!taskQueue.empty()) {
            taskQueue.front().RunTask();
            taskQueue.pop();
        }
        return stolen;
    }
}

void TaskScheduler::WaitForFinishAllTasksWithProperties(TaskProperties properties)
{
    os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
    // Atomic with acq_rel order reason: other thread should see correct value
    waitToFinish_.fetch_add(1, std::memory_order_acq_rel);
    while (GetCountOfTasksInSystemWithTaskProperties(properties) != 0) {
        finishTasksCondVar_.Wait(&taskSchedulerStateLock_);
    }
    // Atomic with acq_rel order reason: other thread should see correct value
    waitToFinish_.fetch_sub(1, std::memory_order_acq_rel);
}

void TaskScheduler::Finalize()
{
    ASSERT(start_);
    {
        // Wait all tasks will be done
        os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
        // Atomic with acq_rel order reason: other thread should
        // see correct value
        waitToFinish_.fetch_add(1, std::memory_order_acq_rel);
        while (!AreNoMoreTasks()) {
            finishTasksCondVar_.Wait(&taskSchedulerStateLock_);
        }
        finish_ = true;
        // Atomic with release order reason: other thread should see last value
        disableHelpers_.store(true, std::memory_order_release);
        // Atomic with acq_rel order reason: other thread should
        // see correct value
        waitToFinish_.fetch_sub(1, std::memory_order_acq_rel);
        queuesWaitCondVar_.SignalAll();
    }
    for (auto *worker : workers_) {
        worker->Join();
    }
    for (auto *worker : workers_) {
        delete worker;
    }

    if (IsTaskLifetimeStatisticsUsed()) {
        for (const auto &line : taskTimeStats_->GetTaskStatistics()) {
            LOG(INFO, TASK_MANAGER) << line;
        }
    }
    LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: Finalized";
}

void TaskScheduler::IncrementCounterOfAddedTasks(TaskProperties properties, size_t ivalue)
{
    IncrementCountOfTasksInSystem(properties, ivalue);
}

size_t TaskScheduler::IncrementCounterOfExecutedTasks(const TaskPropertiesCounterMap &counterMap)
{
    size_t countOfTasks = 0;
    for (const auto &[properties, count] : counterMap) {
        countOfTasks += count;
        DecrementCountOfTasksInSystem(properties, count);
        // Atomic with acquire order reason: get correct value
        auto waitToFinish = waitToFinish_.load(std::memory_order_acquire);
        if (waitToFinish > 0 && GetCountOfTasksInSystemWithTaskProperties(properties) == 0) {
            os::memory::LockHolder outsideLockHolder(taskSchedulerStateLock_);
            finishTasksCondVar_.SignalAll();
        }
    }
    return countOfTasks;
}

void TaskScheduler::SignalWorkers()
{
    // Atomic with acquire order reason: get correct value
    if (waitWorkersCount_.load(std::memory_order_acquire) > 0) {
        os::memory::LockHolder outsideLockHolder(taskSchedulerStateLock_);
        queuesWaitCondVar_.Signal();
    }
}

internal::SchedulableTaskQueueInterface *TaskScheduler::GetQueue(TaskQueueId id) const
{
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    auto taskQueuesIterator = taskQueues_.find(id);
    if (taskQueuesIterator == taskQueues_.end()) {
        LOG(FATAL, COMMON) << "Attempt to take a task from a non-existent queue";
    }
    std::tie(std::ignore, queue) = *taskQueuesIterator;
    return queue;
}

WaiterId TaskScheduler::AddTaskToWaitListWithTimeout(Task &&task, uint64_t time)
{
    os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
    this->IncrementCounterOfAddedTasks(task.GetTaskProperties(), 1U);
    return waitList_.AddValueToWait(std::move(task), time);
}

WaiterId TaskScheduler::AddTaskToWaitList(Task &&task)
{
    // Use adding with max time as possible, wait list will understand that it should set max possible time
    return AddTaskToWaitListWithTimeout(std::move(task), std::numeric_limits<uint64_t>().max());
}

void TaskScheduler::PutWaitTaskInLocalQueue(LocalTaskQueue &queue)
{
    for (auto task = waitList_.GetReadyValue(); task.has_value(); task = waitList_.GetReadyValue()) {
        queue.push(std::move(task.value()));
    }
}

void TaskScheduler::PutTaskInTaskQueues(LocalTaskQueue &queue)
{
    while (!queue.empty()) {
        Task task = std::move(queue.front());
        queue.pop();
        auto prop = task.GetTaskProperties();
        auto *taskQueue = GetQueue({prop.GetTaskType(), prop.GetVMType()});
        taskQueue->AddTaskWithoutNewTaskCallbackExecution(std::move(task));
    }
}

void TaskScheduler::SignalWaitList(WaiterId waiterId)
{
    std::optional<Task> task;
    {
        os::memory::LockHolder lockHolder(taskSchedulerStateLock_);
        task = waitList_.GetValueById(waiterId);
    }
    if (!task.has_value()) {
        return;
    }
    auto prop = task->GetTaskProperties();
    auto *queue = GetQueue({prop.GetTaskType(), prop.GetVMType()});
    queue->AddTaskWithoutNewTaskCallbackExecution(std::move(task.value()));
}

TaskScheduler::~TaskScheduler()
{
    // We can delete TaskScheduler if it wasn't started or it was finished
    ASSERT(start_ == finish_);
    // Check if all task queue was deleted
    ASSERT(taskQueues_.empty());
    delete taskTimeStats_;
}

void TaskScheduler::IncrementCountOfTasksInSystem(TaskProperties prop, size_t count)
{
    // Atomic with acq_rel order reason: fast add count to countOfTasksInSystem_[prop]
    countOfTasksInSystem_[prop].fetch_add(count, std::memory_order_acq_rel);
}

void TaskScheduler::DecrementCountOfTasksInSystem(TaskProperties prop, size_t count)
{
    // Atomic with acq_rel order reason: fast sub count to countOfTasksInSystem_[prop]
    countOfTasksInSystem_[prop].fetch_sub(count, std::memory_order_acq_rel);
}

size_t TaskScheduler::GetCountOfTasksInSystemWithTaskProperties(TaskProperties prop) const
{
    // Atomic with acquire order reason: need to sync with all prev fetch_adds and fetch_subs
    return countOfTasksInSystem_.at(prop).load(std::memory_order_acquire);
}

size_t TaskScheduler::GetCountOfTasksInSystem() const
{
    size_t sumCount = 0;
    for ([[maybe_unused]] const auto &[prop, counter] : countOfTasksInSystem_) {
        // Atomic with acquire order reason: need to sync with all prev fetch_adds and fetch_subs
        sumCount += counter.load(std::memory_order_acquire);
    }
    return sumCount;
}

bool TaskScheduler::IsTaskLifetimeStatisticsUsed() const
{
    return taskTimeStatsType_ != TaskTimeStatsType::NO_STATISTICS;
}

TaskTimeStatsBase *TaskScheduler::GetTaskTimeStats() const
{
    return taskTimeStats_;
}

}  // namespace ark::taskmanager
