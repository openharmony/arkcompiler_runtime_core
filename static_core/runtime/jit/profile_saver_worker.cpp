/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/jit/profile_saver_worker.h"
#include "compiler/aot/aot_manager.h"
#include "include/runtime.h"
#include "runtime/jit/profiling_saver.h"
#include "libarkbase/taskmanager/task_manager.h"
#include "libarkbase/utils/logger.h"

namespace ark {

ProfileSaverWorker::ProfileSaverWorker(mem::InternalAllocatorPtr internalAllocator)
    : internalAllocator_(internalAllocator)
{
    os::memory::LockHolder lock(taskQueueLock_);
    profileSaverTaskQueue_ = taskmanager::TaskManager::CreateTaskQueue<decltype(internalAllocator_->Adapter())>(
        taskmanager::MIN_QUEUE_PRIORITY);
    lastSaveTime_ = std::chrono::steady_clock::now();
    hasProfileSaverTaskInQueue_ = false;
    profileSaverWorkerJoined_ = false;
    ASSERT(profileSaverTaskQueue_ != nullptr);
}

void ProfileSaverWorker::InitializeWorker()
{
    os::memory::LockHolder lock(taskQueueLock_);
    lastSaveTime_ = std::chrono::steady_clock::now();
    profileSaverWorkerJoined_ = false;
}

void ProfileSaverWorker::JoinWorker()
{
    {
        os::memory::LockHolder lock(taskQueueLock_);
        profileSaverWorkerJoined_ = true;
    }
    profileSaverTaskQueue_->WaitBackgroundTasks();
}

void ProfileSaverWorker::WaitForFinishSaverTask()
{
    profileSaverTaskQueue_->WaitBackgroundTasks();
}

void ProfileSaverWorker::EndTask()
{
    os::memory::LockHolder lock(taskQueueLock_);
    hasProfileSaverTaskInQueue_ = false;
}

bool ProfileSaverWorker::TryAddTask()
{
    if (profileSaverWorkerJoined_) {
        return false;
    }

    {
        os::memory::LockHolder lock(taskQueueLock_);
        auto timestamp = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - lastSaveTime_).count();
        if (duration < Runtime::GetCurrent()->GetOptions().GetProfilesaverSleepingTimeMs()) {
            return false;
        }
        lastSaveTime_ = timestamp;
        if (hasProfileSaverTaskInQueue_) {
            LOG(INFO, RUNTIME) << "[profile_saver] already has task in queue.";
            return false;
        }
        hasProfileSaverTaskInQueue_ = true;
    }
    LOG(INFO, RUNTIME) << "[profile_saver] profile saver task created.";
    AddTask();
    return true;
}

void ProfileSaverWorker::AddTask()
{
    profileSaverTaskQueue_->AddBackgroundTask([this]() mutable {
        auto instance = Runtime::GetCurrent();
        auto *aotManager = instance->GetClassLinker()->GetAotManager();
        if (!aotManager->HasProfiledMethods()) {
            LOG(INFO, RUNTIME) << "[profile_saver] No profiled method, skip saver task.";
            this->EndTask();
            return;
        }

        LOG(INFO, RUNTIME) << "[profile_saver] increamental saving profile data...";
        auto &profiledMethods = aotManager->GetProfiledMethods();
        auto profiledMethodsFinal = aotManager->GetProfiledMethodsFinal();

        auto pathMapSnapshot = aotManager->GetProfiledPandaFilesMapSnapshot();
        ProfilingSaver profileSaver;

        // All panda files (boot + app) for inline cache pfIdx resolution
        auto allPandaFiles = aotManager->GetProfiledPandaFiles();

        if (!pathMapSnapshot.empty()) {
            // Multi-path mode: each app ABC saves to its own profile, no boot context needed
            auto classCtxStr = aotManager->GetAppClassContext();
            // Group ABC files by their output profile path
            PandaUnorderedMap<PandaString, PandaUnorderedSet<std::string>> pathToAbcs;
            for (const auto &[abcPath, pathInfo] : pathMapSnapshot) {
                pathToAbcs[pathInfo.curProfilePath].insert(std::string(abcPath));
            }
            // Save each group to its corresponding profile path
            for (auto &[profilePath, abcSet] : pathToAbcs) {
                profileSaver.SaveProfile(profilePath, classCtxStr, profiledMethods, profiledMethodsFinal, allPandaFiles,
                                         abcSet);
            }
        } else {
            // Fallback: save all (including boot) to the default profile output path
            auto classCtxStr = aotManager->GetBootClassContext() + ":" + aotManager->GetAppClassContext();
            auto savingPath = PandaString(instance->GetOptions().GetProfileOutput());
            profileSaver.SaveProfile(savingPath, classCtxStr, profiledMethods, profiledMethodsFinal, allPandaFiles,
                                     allPandaFiles);
        }

        this->EndTask();
    });
}
}  // namespace ark
