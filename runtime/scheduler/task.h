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

#ifndef PANDA_RUNTIME_SCHEDULER_TASK_H_
#define PANDA_RUNTIME_SCHEDULER_TASK_H_

#include "runtime/include/managed_thread.h"
#include "runtime/scheduler/worker_thread.h"

namespace panda::scheduler {

// Task is a user-level light-weight thread.
class Task : public panda::ManagedThread {
public:
    explicit Task(PandaVM *vm, ObjectHeader *obj = nullptr);
    // We need to override this to call appropriate destructor in thread manager
    ~Task() override = default;

    static Task *GetCurrent()
    {
        auto task = Thread::GetCurrent();
        ASSERT(task->GetThreadType() == Thread::ThreadType::THREAD_TYPE_TASK);
        return static_cast<Task *>(task);
    }

    static Task *Create(PandaVM *vm, ObjectHeader *obj = nullptr);
    void Initialize();
    void Destroy();

    void FreeInternalMemory() override
    {
        ManagedThread::FreeInternalMemory();
    }

    void VisitGCRoots(const ObjectVisitor &cb) override
    {
        ManagedThread::VisitGCRoots(cb);
        if (future != nullptr) {
            cb(future);
        }
    }

    void UpdateGCRoots() override
    {
        ManagedThread::UpdateGCRoots();
        if ((future != nullptr) && (future->IsForwarded())) {
            future = ::panda::mem::GetForwardAddress(future);
        }
    }

    void SwitchFromWorkerThread();
    static void SuspendCurrent();
    static void EndCurrent();

private:
    WorkerThread *workerThread = nullptr;
    ObjectHeader *future = nullptr;

    NO_COPY_SEMANTIC(Task);
    NO_MOVE_SEMANTIC(Task);
};

}  // namespace panda::scheduler

#endif  // PANDA_RUNTIME_SCHEDULER_TASK_H_
