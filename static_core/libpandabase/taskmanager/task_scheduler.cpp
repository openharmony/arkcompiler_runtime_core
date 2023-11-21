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

namespace panda::taskmanager {

TaskScheduler *TaskScheduler::instance_ = nullptr;

TaskScheduler::TaskScheduler(size_t workers_count) : workers_count_(workers_count), gen_(std::random_device()()) {}

/* static */
TaskScheduler *TaskScheduler::Create(size_t threads_count)
{
    ASSERT(instance_ == nullptr);
    ASSERT(threads_count > 0);
    instance_ = new TaskScheduler(threads_count);
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
    os::memory::LockHolder lock_holder(task_manager_lock_);
    ASSERT(!start_);
    TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
    if (task_queues_.find(id) != task_queues_.end()) {
        return INVALID_TASKQUEUE_ID;
    }
    task_queues_[id] = queue;
    queue->SetNewTasksCallback(
        [this](TaskProperties properties, size_t count) { this->IncrementNewTaskCounter(properties, count); });
    return id;
}

void TaskScheduler::Initialize()
{
    ASSERT(!start_);
    start_ = true;
    LOG(DEBUG, RUNTIME) << "TaskScheduler: creates " << workers_count_ << " threads";
    for (size_t i = 0; i < workers_count_; i++) {
        workers_.push_back(new WorkerThread(
            [this](const TaskPropertiesCounterMap &counter_map) { this->IncrementFinishedTaskCounter(counter_map); }));
    }
}

bool TaskScheduler::FillWithTasks(WorkerThread *worker, size_t tasks_count)
{
    ASSERT(start_);
    os::memory::LockHolder worker_lock_holder(workers_lock_);
    os::memory::LockHolder task_manager_lock_holder(task_manager_lock_);
    LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks";

    for (size_t i = 0; i < tasks_count; i++) {
        while (AreQueuesEmpty()) {
            /**
             * We exit in 2 situations:
             * - !is_worker_empty : if worker has tasks and queues are empty, worker will not wait.
             * - finish_ : when TM Finalize, Worker wake up and go finish WorkerLoop.
             */
            if (!worker->IsEmpty() || finish_) {
                LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks: return queue with" << i << "issues";
                return finish_;
            }
            queues_wait_cond_var_.Wait(&task_manager_lock_);
        }
        PutTaskInWorker(worker, GetNextTask());
    }
    LOG(DEBUG, RUNTIME) << "TaskScheduler: FillWithTasks: return full queue";
    return false;
}

Task TaskScheduler::GetNextTask()
{
    ASSERT(!AreQueuesEmpty());
    LOG(DEBUG, RUNTIME) << "TaskScheduler: GetNextTask()";

    auto kinetic_priorities = GetKineticPriorities();
    size_t kinetic_max = 0;
    std::tie(kinetic_max, std::ignore) = *kinetic_priorities.rbegin();  // Get key of the last element in map

    std::uniform_int_distribution<size_t> distribution(0U, kinetic_max - 1U);
    size_t choice = distribution(gen_);  // Get random number in range [0, kinetic_max)
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    std::tie(std::ignore, queue) = *kinetic_priorities.upper_bound(choice);  // Get queue of chosen element

    return queue->PopTask().value();
}

std::map<size_t, internal::SchedulableTaskQueueInterface *> TaskScheduler::GetKineticPriorities() const
{
    ASSERT(!task_queues_.empty());  // no TaskQueues
    size_t kinetic_sum = 0;
    std::map<size_t, internal::SchedulableTaskQueueInterface *> kinetic_priorities;
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    for (auto &traits_queue_pair : task_queues_) {
        std::tie(std::ignore, queue) = traits_queue_pair;
        if (queue->IsEmpty()) {
            continue;
        }
        kinetic_sum += queue->GetPriority();
        kinetic_priorities[kinetic_sum] = queue;
    }
    return kinetic_priorities;
}

void TaskScheduler::PutTaskInWorker(WorkerThread *worker, Task &&task)
{
    worker->AddTask(std::move(task));
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
    return new_tasks_count_ == finished_tasks_count_;
}

std::optional<Task> TaskScheduler::GetTaskFromQueue(TaskQueueId id, TaskExecutionMode mode)
{
    LOG(DEBUG, RUNTIME) << "TaskScheduler: GetTaskFromQueue()";
    os::memory::LockHolder lock_holder(task_manager_lock_);
    auto task_queues_iterator = task_queues_.find(id);
    if (task_queues_iterator == task_queues_.end()) {
        if (finish_) {
            return std::nullopt;
        }
        LOG(FATAL, COMMON) << "Attempt to take a task from a non-existent queue";
    }
    internal::SchedulableTaskQueueInterface *queue = nullptr;
    std::tie(std::ignore, queue) = *task_queues_iterator;
    if (!queue->HasTaskWithExecutionMode(mode)) {
        return std::nullopt;
    }
    finished_tasks_count_[{queue->GetTaskType(), queue->GetVMType(), mode}]++;
    return queue->PopTask();
}

std::optional<Task> TaskScheduler::GetTaskFromQueue(TaskProperties properties)
{
    TaskQueueId queue_id(properties.GetTaskType(), properties.GetVMType());
    return GetTaskFromQueue(queue_id, properties.GetTaskExecutionMode());
}

void TaskScheduler::WaitForFinishAllTasksWithProperties(TaskProperties properties)
{
    os::memory::LockHolder lock_holder(task_manager_lock_);
    while (finished_tasks_count_[properties] != new_tasks_count_[properties]) {
        finish_tasks_cond_var_.Wait(&task_manager_lock_);
    }
}

void TaskScheduler::Finalize()
{
    ASSERT(start_);
    {
        // Wait all tasks will be done
        os::memory::LockHolder lock_holder(task_manager_lock_);
        while (!AreNoMoreTasks()) {
            finish_tasks_cond_var_.Wait(&task_manager_lock_);
        }
        finish_ = true;
        queues_wait_cond_var_.Signal();
    }
    for (auto worker : workers_) {
        worker->Join();
        delete worker;
    }
    LOG(DEBUG, RUNTIME) << "TaskScheduler: Finalized";
}

void TaskScheduler::IncrementNewTaskCounter(TaskProperties properties, size_t ivalue)
{
    os::memory::LockHolder outside_lock_holder(task_manager_lock_);
    queues_wait_cond_var_.Signal();
    new_tasks_count_[properties] += ivalue;
}

void TaskScheduler::IncrementFinishedTaskCounter(const TaskPropertiesCounterMap &counter_map)
{
    os::memory::LockHolder outside_lock_holder(task_manager_lock_);
    finish_tasks_cond_var_.Signal();
    for (const auto &[properties, count] : counter_map) {
        finished_tasks_count_[properties] += count;
    }
}

TaskScheduler::~TaskScheduler()
{
    // We can delete TaskScheduler if it wasn't started or it was finished
    ASSERT(start_ == finish_);
    // Check if all task queue was deleted
    ASSERT(task_queues_.empty());
    LOG(DEBUG, RUNTIME) << "TaskScheduler: ~TaskScheduler: All threads finished jobs";
}

}  // namespace panda::taskmanager
