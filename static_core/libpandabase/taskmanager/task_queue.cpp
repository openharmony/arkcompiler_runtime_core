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

#include "libpandabase/taskmanager/task_queue.h"
#include "libpandabase/globals.h"

namespace panda::taskmanager {

TaskQueue::TaskQueue(TaskType task_type, VMType vm_type, size_t priority)
    : priority_(priority), task_type_(task_type), vm_type_(vm_type)
{
    ASSERT(priority_ != 0);
}

Task TaskQueue::PopTaskFromQueue(std::queue<Task> &queue)
{
    auto task = std::move(queue.front());
    queue.pop();
    return task;
}

void TaskQueue::PushTaskToInternalQueues(Task &&task)
{
    if (task.GetTaskProperties().GetTaskExecutionMode() == TaskExecutionMode::FOREGROUND) {
        foreground_task_queue_.push(std::move(task));
    } else {
        background_task_queue_.push(std::move(task));
    }
}

size_t TaskQueue::AddTask(Task &&task)
{
    ASSERT(task.GetTaskProperties().GetTaskType() == task_type_);
    ASSERT(task.GetTaskProperties().GetVMType() == vm_type_);
    size_t size = 0;
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        PushTaskToInternalQueues(std::move(task));
        task_queue_cond_var_.Signal();
        size = SumSizeOfInternalQueues();
    }
    {
        os::memory::LockHolder lock_holder(subscriber_lock_);
        // Notify subscriber about new task
        if (outside_lock_ != nullptr) {
            os::memory::LockHolder outside_lock_holder(*outside_lock_);
            outside_cond_var_->Signal();
        }
    }
    return size;
}

bool TaskQueue::HasTaskWithExecutionMode(TaskExecutionMode mode) const
{
    os::memory::LockHolder lock_holder(task_queue_lock_);
    if (mode == TaskExecutionMode::FOREGROUND) {
        return !foreground_task_queue_.empty();
    }
    return !background_task_queue_.empty();
}

Task TaskQueue::PopTaskFromInternalQueues()
{
    if (!foreground_task_queue_.empty()) {
        return PopTaskFromQueue(foreground_task_queue_);
    }
    return PopTaskFromQueue(background_task_queue_);
}

std::optional<Task> TaskQueue::PopTask()
{
    os::memory::LockHolder lock_holder(task_queue_lock_);
    while (AreInternalQueuesEmpty()) {
        if (finish_) {
            return std::nullopt;
        }
        task_queue_cond_var_.Wait(&task_queue_lock_);
    }
    auto task = PopTaskFromInternalQueues();
    finish_var_.Signal();
    return std::make_optional(std::move(task));
}

std::optional<Task> TaskQueue::PopTask(TaskExecutionMode mode)
{
    os::memory::LockHolder lock_holder(task_queue_lock_);
    std::queue<Task> *queue = &foreground_task_queue_;
    if (mode != TaskExecutionMode::FOREGROUND) {
        queue = &background_task_queue_;
    }
    while (queue->empty()) {
        if (finish_) {
            return std::nullopt;
        }
        task_queue_cond_var_.Wait(&task_queue_lock_);
    }
    auto task = PopTaskFromQueue(*queue);
    finish_var_.Signal();
    return std::make_optional(std::move(task));
}

bool TaskQueue::AreInternalQueuesEmpty() const
{
    return foreground_task_queue_.empty() && background_task_queue_.empty();
}

bool TaskQueue::IsEmpty() const
{
    os::memory::LockHolder lock_holder(task_queue_lock_);
    return AreInternalQueuesEmpty();
}

size_t TaskQueue::SumSizeOfInternalQueues() const
{
    return foreground_task_queue_.size() + background_task_queue_.size();
}

size_t TaskQueue::Size() const
{
    os::memory::LockHolder lock_holder(task_queue_lock_);
    return SumSizeOfInternalQueues();
}

size_t TaskQueue::GetPriority() const
{
    // Atomic with acquire order reason: data race with priority_ with dependencies on reads after the
    // load which should become visible
    return priority_.load(std::memory_order_acquire);
}

void TaskQueue::SetPriority(size_t priority)
{
    // Atomic with release order reason: data race with priority_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    priority_.store(priority, std::memory_order_release);
}

TaskType TaskQueue::GetTaskType() const
{
    return task_type_;
}

VMType TaskQueue::GetVMType() const
{
    return vm_type_;
}

void TaskQueue::SubscribeCondVarToAddTask(os::memory::Mutex *lock, os::memory::ConditionVariable *cond_var)
{
    os::memory::LockHolder lock_holder(subscriber_lock_);
    outside_lock_ = lock;
    outside_cond_var_ = cond_var;
}

void TaskQueue::UnsubscribeCondVarFromAddTask()
{
    os::memory::LockHolder lock_holder(subscriber_lock_);
    outside_lock_ = nullptr;
    outside_cond_var_ = nullptr;
}

void TaskQueue::WaitForQueueEmptyAndFinish()
{
    os::memory::LockHolder lock_holder(task_queue_lock_);
    while (!AreInternalQueuesEmpty()) {
        finish_var_.Wait(&task_queue_lock_);
    }
    finish_ = true;
    task_queue_cond_var_.SignalAll();
}

TaskQueue::~TaskQueue()
{
    WaitForQueueEmptyAndFinish();
}

TaskQueueTraits::TaskQueueTraits(TaskType tt, VMType vt)
{
    static_assert(sizeof(TaskType) == sizeof(TaskQueueTraits) / 2);
    static_assert(sizeof(VMType) == sizeof(TaskQueueTraits) / 2);
    // Multiply by BITS_IN_BYTE to convert bytes to bits
    val_ = static_cast<uint32_t>(tt) | static_cast<uint32_t>(vt) << (BITS_PER_BYTE * sizeof(TaskQueueTraits) / 2);
}

TaskType TaskQueueTraits::GetTaskType() const
{
    return static_cast<TaskType>(val_);  // cuts first 16 bits with VMType info
}

VMType TaskQueueTraits::GetVMType() const
{
    // Multiply by BITS_IN_BYTE to convert bytes to bits
    return static_cast<VMType>(val_ >> (BITS_PER_BYTE * sizeof(TaskQueueTraits) / 2));
}

bool operator<(const TaskQueueTraits &lv, const TaskQueueTraits &rv)
{
    return lv.val_ < rv.val_;
}
}  // namespace panda::taskmanager