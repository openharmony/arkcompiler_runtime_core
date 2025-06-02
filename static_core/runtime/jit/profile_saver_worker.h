/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_PROFILE_SAVER_WORKER_H
#define PANDA_RUNTIME_PROFILE_SAVER_WORKER_H

#include <chrono>
#include "runtime/include/mem/allocator.h"
#include "libpandabase/taskmanager/task_queue_interface.h"
#include "libpandabase/taskmanager/task_scheduler.h"

namespace ark {
/// @brief Profile Saver worker
class ProfileSaverWorker {
public:
    explicit ProfileSaverWorker(mem::InternalAllocatorPtr internalAllocator);
    NO_COPY_SEMANTIC(ProfileSaverWorker);
    NO_MOVE_SEMANTIC(ProfileSaverWorker);

    ~ProfileSaverWorker()
    {
        taskmanager::TaskScheduler::GetTaskScheduler()
            ->UnregisterAndDestroyTaskQueue<decltype(internalAllocator_->Adapter())>(profileSaverTaskQueue_);
    }

    void InitializeWorker();

    void FinalizeWorker()
    {
        JoinWorker();
    }

    void JoinWorker();

    bool IsWorkerJoined()
    {
        return profileSaverWorkerJoined_;
    }

    void EndTask();

    bool TryAddTask();

    void AddTask();

    void PreZygoteFork()
    {
        FinalizeWorker();
    }

    void PostZygoteFork()
    {
        InitializeWorker();
    }

    void WaitForFinishSaverTask();

private:
    static constexpr taskmanager::TaskProperties PROFILE_SAVER_WORKER_TASK_PROPERTIES = {
        taskmanager::TaskType::PROFILE_SAVER, taskmanager::VMType::STATIC_VM,
        taskmanager::TaskExecutionMode::BACKGROUND};

    taskmanager::TaskQueueInterface *profileSaverTaskQueue_ {nullptr};
    os::memory::Mutex taskQueueLock_;
    std::atomic<bool> profileSaverWorkerJoined_ {true};
    bool hasProfileSaverTaskInQueue_ GUARDED_BY(taskQueueLock_) {false};
    std::chrono::steady_clock::time_point lastSaveTime_ GUARDED_BY(taskQueueLock_);
    mem::InternalAllocatorPtr internalAllocator_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_PROFILE_SAVER_WORKER_H
