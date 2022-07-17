/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "runtime/scheduler/task.h"
#include "runtime/scheduler/worker_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace panda::scheduler {

// TODO(xuliang): task id
Task::Task(PandaVM *vm, ObjectHeader *obj)
    : ManagedThread(-1, Runtime::GetCurrent()->GetInternalAllocator(), vm, Thread::ThreadType::THREAD_TYPE_TASK)
{
    future = obj;
}

Task *Task::Create(PandaVM *vm, ObjectHeader *obj)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *task = allocator->New<Task>(vm, obj);
    task->Initialize();
    return task;
}

void Task::Initialize()
{
    trace::ScopedTrace scoped_trace2("ThreadManager::RegisterThread");
    // TODO(xuliang): RegisterThread
}

void Task::Destroy()
{
    ASSERT(this == Task::GetCurrent());
    // TODO(xuliang): should be done in UnregisterExitedThread.
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto wt = workerThread;
    allocator->Delete(this);
    Thread::SetCurrent(wt);
}

void Task::SwitchFromWorkerThread()
{
    workerThread = WorkerThread::GetCurrent();
    workerThread->SetTask(this);
    Thread::SetCurrent(this);
}

void Task::SuspendCurrent()
{
    auto task = Task::GetCurrent();
    auto wt = task->workerThread;
    ASSERT(wt != nullptr);
    Thread::SetCurrent(wt);
    wt->SetTask(nullptr);
}

void Task::EndCurrent()
{
    auto task = Task::GetCurrent();
    auto wt = task->workerThread;
    ASSERT(wt != nullptr);
    task->Destroy();
    Thread::SetCurrent(wt);
    wt->SetTask(nullptr);
}

}  // namespace panda::scheduler
