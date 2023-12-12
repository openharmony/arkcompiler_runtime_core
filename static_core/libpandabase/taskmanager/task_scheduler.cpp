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

TaskScheduler::TaskScheduler(size_t workers_count, TaskStatisticsImplType task_statistics_type)
    : workers_count_(workers_count), gen_(std::random_device()())
{
    switch (task_statistics_type) {
        case TaskStatisticsImplType::FINE_GRAINED:
            task_statistics_ = new FineGrainedTaskStatisticsImpl();
            break;
        case TaskStatisticsImplType::SIMPLE:
            task_statistics_ = new SimpleTaskStatisticsImpl();
            break;
        default:
            UNREACHABLE();
    }
}

/* static */
TaskScheduler *TaskScheduler::Create(size_t threads_count, TaskStatisticsImplType task_statistics_type)
{
    ASSERT(instance_ == nullptr);
    ASSERT(threads_count > 0);
    instance_ = new TaskScheduler(threads_count, task_statistics_type);
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
    os::memory::LockHolder lock_holder(task_scheduler_state_lock_);
    ASSERT(!start_);
    TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
    if (task_queues_.find(id) != task_queues_.end()) {
        return INVALID_TASKQUEUE_ID;
    }
    task_queues_[id] = queue;
    queue->SetNewTasksCallback([this](TaskProperties properties, size_t count, bool was_empty) {
        this->IncrementCounterOfAddedTasks(properties, count, was_empty);
    });
    return id;
}

void TaskScheduler::Initialize()
{
    ASSERT(!start_);
    start_ = true;
    LOG(DEBUG, RUNTIME) << "TaskScheduler: creates " << workers_count_ << " threads";
    for (size_t i = 0; i < workers_count_; i++) {
        workers_.push_back(new WorkerThread([this](const TaskPropertiesCounterMap &counter_map) {
            this->IncrementCounterOfExecutedTasks(counter_map);
        }));
    }
}

bool TaskScheduler::FillWithTasks(WorkerThread *worker, size_t tasks_count)
{
    ASSERT(start_);
    os::memory::LockHolder worker_lock_holder(workers_lock_);
    LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks";
    {
        os::memory::LockHolder task_scheduler_lock_holder(task_scheduler_state_lock_);
        while (AreQueuesEmpty()) {
            /// We exit in situation when finish_ = true .TM Finalize, Worker wake up and go finish WorkerLoop.
            if (finish_) {
                LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks: return queue without issues";
                return true;
            }
            queues_wait_cond_var_.Wait(&task_scheduler_state_lock_);
        }
    }
    os::memory::LockHolder pop_lock_holder(pop_from_task_queues_lock_);
    SelectNextTasks(tasks_count);
    PutTasksInWorker(worker);
    LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks: return queue with tasks";
    return false;
}

void TaskScheduler::SelectNextTasks(size_t tasks_count)
{
    LOG(DEBUG, RUNTIME) << "TaskScheduler: SelectNextTasks()";
    if (AreQueuesEmpty()) {
        return;
    }
    UpdateKineticPriorities();
    // Now kinetic_priorities_
    size_t kinetic_max = 0;
    std::tie(kinetic_max, std::ignore) = *kinetic_priorities_.rbegin();  // Get key of the last element in map
    std::uniform_int_distribution<size_t> distribution(0U, kinetic_max - 1U);

    for (size_t i = 0; i < tasks_count; i++) {
        size_t choice = distribution(gen_);  // Get random number in range [0, kinetic_max)
        internal::SchedulableTaskQueueInterface *queue = nullptr;
        std::tie(std::ignore, queue) = *kinetic_priorities_.upper_bound(choice);  // Get queue of chosen element

        TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
        selected_queues_[id]++;
    }
}

void TaskScheduler::UpdateKineticPriorities()
{
    ASSERT(!task_queues_.empty());  // no TaskQueues
    size_t kinetic_sum = 0;
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    for (const auto &id_queue_pair : task_queues_) {
        std::tie(std::ignore, queue) = id_queue_pair;
        if (queue->IsEmpty()) {
            continue;
        }
        kinetic_sum += queue->GetPriority();
        kinetic_priorities_[kinetic_sum] = queue;
    }
}

size_t TaskScheduler::PutTasksInWorker(WorkerThread *worker)
{
    size_t task_count = 0;
    for (auto [id, size] : selected_queues_) {
        if (size == 0) {
            continue;
        }
        auto add_task_func = [worker](Task &&task) { worker->AddTask(std::move(task)); };
        size_t queue_task_count = task_queues_[id]->PopTasksToWorker(add_task_func, size);
        task_count += queue_task_count;
        LOG(DEBUG, RUNTIME) << "PutTasksInWorker: worker have gotten " << queue_task_count << " tasks";
    }
    selected_queues_.clear();
    return task_count;
}

bool TaskScheduler::AreQueuesEmpty() const
{
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    for (const auto &traits_queue_pair : task_queues_) {
        std::tie(std::ignore, queue) = traits_queue_pair;
        if (!queue->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool TaskScheduler::AreNoMoreTasks() const
{
    return task_statistics_->GetCountOfTaskInSystem() == 0;
}

std::optional<Task> TaskScheduler::GetTaskFromQueue(TaskProperties properties)
{
    LOG(DEBUG, RUNTIME) << "TaskScheduler: GetTaskFromQueue()";
    os::memory::LockHolder pop_lock_holder(pop_from_task_queues_lock_);
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    {
        os::memory::LockHolder task_manager_lock_holder(task_scheduler_state_lock_);
        auto task_queues_iterator = task_queues_.find({properties.GetTaskType(), properties.GetVMType()});
        if (task_queues_iterator == task_queues_.end()) {
            if (finish_) {
                return std::nullopt;
            }
            LOG(FATAL, COMMON) << "Attempt to take a task from a non-existent queue";
        }
        std::tie(std::ignore, queue) = *task_queues_iterator;
    }
    if (!queue->HasTaskWithExecutionMode(properties.GetTaskExecutionMode())) {
        return std::nullopt;
    }
    // Now we can pop the task from the chosen queue
    auto task = queue->PopTask();
    // Only after popping we can notify task statistics that task was POPPED from queue to get it outside. This sequence
    // ensures that the number of tasks in the system (see TaskStatistics) will be correct.
    task_statistics_->IncrementCount(TaskStatus::POPPED, properties, 1);
    if (task_statistics_->GetCountOfTasksInSystemWithTaskProperties(properties) == 0) {
        os::memory::LockHolder task_manager_lock_holder(task_scheduler_state_lock_);
        finish_tasks_cond_var_.SignalAll();
    }
    return task;
}

void TaskScheduler::WaitForFinishAllTasksWithProperties(TaskProperties properties)
{
    os::memory::LockHolder lock_holder(task_scheduler_state_lock_);
    while (task_statistics_->GetCountOfTasksInSystemWithTaskProperties(properties) != 0) {
        finish_tasks_cond_var_.Wait(&task_scheduler_state_lock_);
    }
    LOG(DEBUG, RUNTIME) << "After waiting tasks with properties: " << properties
                        << " {added: " << task_statistics_->GetCount(TaskStatus::ADDED, properties)
                        << ", executed: " << task_statistics_->GetCount(TaskStatus::EXECUTED, properties)
                        << ", popped: " << task_statistics_->GetCount(TaskStatus::POPPED, properties) << "}";
    task_statistics_->ResetCountersWithTaskProperties(properties);
}

void TaskScheduler::Finalize()
{
    ASSERT(start_);
    {
        // Wait all tasks will be done
        os::memory::LockHolder lock_holder(task_scheduler_state_lock_);
        while (!AreNoMoreTasks()) {
            finish_tasks_cond_var_.Wait(&task_scheduler_state_lock_);
        }
        finish_ = true;
        queues_wait_cond_var_.Signal();
    }
    for (auto worker : workers_) {
        worker->Join();
        delete worker;
    }
    task_statistics_->ResetAllCounters();
    LOG(DEBUG, RUNTIME) << "TaskScheduler: Finalized";
}

void TaskScheduler::IncrementCounterOfAddedTasks(TaskProperties properties, size_t ivalue, bool was_empty)
{
    task_statistics_->IncrementCount(TaskStatus::ADDED, properties, ivalue);
    if (was_empty) {
        os::memory::LockHolder outside_lock_holder(task_scheduler_state_lock_);
        queues_wait_cond_var_.Signal();
    }
}

void TaskScheduler::IncrementCounterOfExecutedTasks(const TaskPropertiesCounterMap &counter_map)
{
    for (const auto &[properties, count] : counter_map) {
        task_statistics_->IncrementCount(TaskStatus::EXECUTED, properties, count);
        if (task_statistics_->GetCountOfTasksInSystemWithTaskProperties(properties) == 0) {
            os::memory::LockHolder outside_lock_holder(task_scheduler_state_lock_);
            finish_tasks_cond_var_.SignalAll();
        }
    }
}

TaskScheduler::~TaskScheduler()
{
    // We can delete TaskScheduler if it wasn't started or it was finished
    ASSERT(start_ == finish_);
    // Check if all task queue was deleted
    ASSERT(task_queues_.empty());
    delete task_statistics_;
    LOG(DEBUG, RUNTIME) << "TaskScheduler: ~TaskScheduler: All threads finished jobs";
}

}  // namespace panda::taskmanager
