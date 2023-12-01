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
#include "runtime/include/thread.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_worker.h"

namespace panda::mem {
GCWorker::GCWorker(GC *gc) : gc_(gc)
{
    auto internal_allocator = gc_->GetInternalAllocator();
    gc_task_queue_ = internal_allocator->New<GCQueueWithTime>(gc_);
    ASSERT(gc_task_queue_ != nullptr);
    auto *vm = gc_->GetPandaVm();
    ASSERT(vm != nullptr);
    gc_thread_ = internal_allocator->New<Thread>(vm, Thread::ThreadType::THREAD_TYPE_GC);
    ASSERT(gc_thread_ != nullptr);
    if (gc_->GetSettings()->UseTaskManagerForGC()) {
        gc_runner_ = [this]() { this->GCTaskRunner(); };
    }
}

GCWorker::~GCWorker()
{
    auto internal_allocator = gc_->GetInternalAllocator();
    internal_allocator->Delete(gc_thread_);
    internal_allocator->Delete(gc_task_queue_);
}

/* static */
void GCWorker::GCThreadLoop(GCWorker *gc_worker)
{
    // We need to set VM to current_thread, since GC can call ObjectAccessor::GetBarrierSet() methods
    ScopedCurrentThread gc_current_thread_scope(gc_worker->gc_thread_);

    while (true) {
        // Get gc task from local gc tasks queue
        auto task = gc_worker->GetTask();
        if (!gc_worker->gc_->IsGCRunning()) {
            LOG(DEBUG, GC) << "Stopping GC thread";
            break;
        }
        gc_worker->RunGC(std::move(task));
    }
}

void GCWorker::CreateAndStartWorker()
{
    // If GC runs in place or Task manager is used for GC, so no need create separate internal GC worker
    if (gc_->GetSettings()->RunGCInPlace()) {
        return;
    }
    if (gc_->GetSettings()->UseTaskManagerForGC()) {
        need_to_finish_ = false;
        return;
    }
    ASSERT(gc_->GetSettings()->UseThreadPoolForGC());
    ASSERT(gc_internal_thread_ == nullptr);
    auto allocator = gc_->GetInternalAllocator();
    gc_internal_thread_ = allocator->New<std::thread>(GCWorker::GCThreadLoop, this);
    ASSERT(gc_internal_thread_ != nullptr);
    auto set_gc_thread_name_result = os::thread::SetThreadName(gc_internal_thread_->native_handle(), "GCThread");
    LOG_IF(set_gc_thread_name_result != 0, ERROR, RUNTIME) << "Failed to set a name for the gc thread";
}

void GCWorker::FinalizeAndDestroyWorker()
{
    // Signal that no need to delay task running
    gc_task_queue_->Signal();
    if (gc_->GetSettings()->UseTaskManagerForGC()) {
        need_to_finish_ = true;
        taskmanager::TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(GC_WORKER_TASK_PROPERTIES);
        return;
    }
    ASSERT(gc_->GetSettings()->UseThreadPoolForGC());
    // Internal GC thread was not created, so just return
    if (gc_internal_thread_ == nullptr) {
        return;
    }
    gc_internal_thread_->join();
    gc_->GetInternalAllocator()->Delete(gc_internal_thread_);
    gc_internal_thread_ = nullptr;
}

void GCWorker::CreateAndAddTaskToTaskManager()
{
    ASSERT_PRINT(gc_runner_ != nullptr, "Need to create task only for TaskManager case");
    auto gc_taskmanager_task = taskmanager::Task::Create(GC_WORKER_TASK_PROPERTIES, gc_runner_);
    gc_->GetWorkersTaskQueue()->AddTask(std::move(gc_taskmanager_task));
}

void GCWorker::GCTaskRunner()
{
    // only one task can get gc task from queue and run it
    if (!gc_task_run_mutex_.TryLock()) {
        // If any task is executed in TaskManager then current task should do nothing and just return
        // According task for TaskManager will be created after RunGC if needed
        return;
    }
    // Task manager does not know anything about panda threads, so set gc thread as current thread during task running
    ScopedCurrentThread gc_current_thread_scope(gc_thread_);
    auto gc_task = GetTask();
    // If GC was not started then task should not be run, so delay the task execution
    if (!gc_->IsGCRunning()) {
        if (!need_to_finish_) {
            // Added task can run on another worker and try to lock gc_task_run_mutex_, but in the current worker we
            // already held the mutex, so TryLock fails and task running cancels
            // So unlock the mutex before adding task
            gc_task_run_mutex_.Unlock();
            AddTask(std::move(gc_task));
        } else {
            gc_task_run_mutex_.Unlock();
        }
        return;
    }
    RunGC(std::move(gc_task));
    gc_task_run_mutex_.Unlock();
    // If gc tasks queue has a task, so need to create Task for TaskManager to process it
    if (!gc_task_queue_->IsEmpty()) {
        CreateAndAddTaskToTaskManager();
    }
}

bool GCWorker::AddTask(PandaUniquePtr<GCTask> task)
{
    bool was_added = gc_task_queue_->AddTask(std::move(task));
    // If Task Manager is used then create a new task for task manager and put it
    if (was_added && gc_->GetSettings()->UseTaskManagerForGC()) {
        CreateAndAddTaskToTaskManager();
    }
    return was_added;
}

PandaUniquePtr<GCTask> GCWorker::GetTask()
{
    auto full_gc_bombing_freq = gc_->GetSettings()->FullGCBombingFrequency();
    // 0 means full gc bombing is not used, so just return task from local queue
    if (full_gc_bombing_freq == 0U) {
        return gc_task_queue_->GetTask(gc_->GetSettings()->UseThreadPoolForGC());
    }
    // Need to bombs full GC in according with full gc bombing frequency
    if (collect_number_mod_ == full_gc_bombing_freq) {
        collect_number_mod_ = 1;
        return MakePandaUnique<GCTask>(GCTaskCause::OOM_CAUSE, time::GetCurrentTimeInNanos());
    }
    ++collect_number_mod_;
    return gc_task_queue_->GetTask(gc_->GetSettings()->UseThreadPoolForGC());
}

void GCWorker::RunGC(PandaUniquePtr<GCTask> task)
{
    if (task == nullptr || task->reason == GCTaskCause::INVALID_CAUSE) {
        return;
    }
    if (gc_->IsPostponeEnabled() && task->reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE) {
        // If GC was postponed then return task back to local gc tasks queue
        this->AddTask(std::move(task));
        // In task manager case worker does not wait new task, otherwise we capture this worker and decrese
        // possibilities for Task manager usages
        if (gc_->GetSettings()->UseThreadPoolForGC()) {
            gc_task_queue_->WaitForGCTask();
        }
        return;
    }
    LOG(DEBUG, GC) << "Running GC task, reason " << task->reason;
    task->Run(*gc_);
}

}  // namespace panda::mem
