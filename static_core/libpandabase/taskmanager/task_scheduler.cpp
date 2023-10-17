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
#include "libpandabase/utils/logger.h"

namespace panda::taskmanager {

TaskScheduler *TaskScheduler::instance_ = nullptr;

TaskScheduler::TaskScheduler(size_t workers_count) : workers_count_(workers_count) {}

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

TaskQueueId TaskScheduler::RegisterQueue(TaskQueue *queue)
{
    os::memory::LockHolder lock_holder(task_manager_lock_);
    ASSERT(!start_);
    TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
    if (task_queues_.find(id) != task_queues_.end()) {
        return INVALID_TASKQUEUE_ID;
    }
    task_queues_[id] = queue;
    queue->SubscribeCallbackToAddTask([this](size_t count) { this->IncrementNewTaskCounter(count); });
    return id;
}

void TaskScheduler::Initialize()
{
    ASSERT(!start_);
    start_ = true;
    LOG(DEBUG, RUNTIME) << "TaskScheduler: creates " << workers_count_ << " threads";
    for (size_t i = 0; i < workers_count_; i++) {
        workers_.push_back(new WorkerThread([this](size_t count) { this->IncrementFinishedTaskCounter(count); }));
    }
}

size_t TaskScheduler::AddTask(Task &&task)
{
    LOG(DEBUG, RUNTIME) << "TaskScheduler: AddTask({ type:" << task.GetTaskProperties().GetTaskType() << "})";
    TaskQueueId id(task.GetTaskProperties().GetTaskType(), task.GetTaskProperties().GetVMType());
    ASSERT(task_queues_.find(id) != task_queues_.end());
    auto size = task_queues_[id]->AddTask(std::move(task));
    return size;
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

    // NOLINTNEXTLINE(cert-msc50-cpp)
    size_t choice = std::rand() % kinetic_max;  // Get random number in range [0, kinetic_max)
    TaskQueue *queue = nullptr;
    std::tie(std::ignore, queue) = *kinetic_priorities.upper_bound(choice);  // Get queue of chosen element

    return queue->PopTask().value();
}

std::map<size_t, TaskQueue *> TaskScheduler::GetKineticPriorities() const
{
    ASSERT(!task_queues_.empty());  // no TaskQueues
    size_t kinetic_sum = 0;
    std::map<size_t, TaskQueue *> kinetic_priorities;
    TaskQueue *queue = nullptr;
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
    TaskQueue *queue = nullptr;
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
    return new_task_counter_ == finished_task_counter_;
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
    TaskQueue *queue = nullptr;
    std::tie(std::ignore, queue) = *task_queues_iterator;
    if (!queue->HasTaskWithExecutionMode(mode)) {
        return std::nullopt;
    }
    finished_task_counter_++;
    return queue->PopTask();
}

std::optional<Task> TaskScheduler::GetTaskFromQueue(TaskProperties properties)
{
    TaskQueueId queue_id(properties.GetTaskType(), properties.GetVMType());
    return GetTaskFromQueue(queue_id, properties.GetTaskExecutionMode());
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
    TaskQueue *queue = nullptr;
    for (auto &traits_queue_pair : task_queues_) {
        std::tie(std::ignore, queue) = traits_queue_pair;
        queue->UnsubscribeCallback();
    }
    task_queues_.clear();
    LOG(DEBUG, RUNTIME) << "TaskScheduler: Finalized";
}

void TaskScheduler::IncrementNewTaskCounter(size_t ivalue)
{
    os::memory::LockHolder outside_lock_holder(task_manager_lock_);
    queues_wait_cond_var_.Signal();
    new_task_counter_ += ivalue;
}

void TaskScheduler::IncrementFinishedTaskCounter(size_t ivalue)
{
    os::memory::LockHolder outside_lock_holder(task_manager_lock_);
    finish_tasks_cond_var_.Signal();
    finished_task_counter_ += ivalue;
}

TaskScheduler::~TaskScheduler()
{
    // We can delete TaskScheduler if it wasn't started or it was finished
    ASSERT(start_ == finish_);
    LOG(DEBUG, RUNTIME) << "TaskScheduler: ~TaskScheduler: All threads finished jobs";
}

}  // namespace panda::taskmanager