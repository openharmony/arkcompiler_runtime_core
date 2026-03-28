/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_EXECUTION_JOB_WORKER_THREAD_INL_H
#define PANDA_RUNTIME_EXECUTION_JOB_WORKER_THREAD_INL_H

#include "runtime/execution/job_worker_thread.h"

namespace ark {

inline Runtime *JobWorkerThread::GetRuntime() const
{
    return runtime_;
}

inline JobWorkerThread::Id JobWorkerThread::GetId() const
{
    return id_;
}

inline PandaString JobWorkerThread::GetName() const
{
    return name_;
}

inline void JobWorkerThread::SetName(PandaString name)
{
    name_ = std::move(name);
}

inline bool JobWorkerThread::InExclusiveMode() const
{
    return inExclusiveMode_;
}

inline bool JobWorkerThread::IsMainWorker() const
{
    return isMainWorker_;
}

inline JobWorkerThread::LocalStorage &JobWorkerThread::GetLocalStorage()
{
    return localStorage_;
}

inline void JobWorkerThread::SetCallbackPoster(PandaUniquePtr<CallbackPoster> poster)
{
    ASSERT(!extSchedulingPoster_);
    extSchedulingPoster_ = std::move(poster);
}

inline bool JobWorkerThread::IsExternalSchedulingEnabled()
{
    os::memory::LockHolder l(posterLock_);
    return extSchedulingPoster_ != nullptr;
}

inline void JobWorkerThread::PostSchedulingTask(int64_t delayMs)
{
    os::memory::LockHolder l(posterLock_);
    if (extSchedulingPoster_ != nullptr) {
        extSchedulingPoster_->Post(delayMs);
    }
}

inline void JobWorkerThread::DestroyCallbackPoster()
{
    os::memory::LockHolder l(posterLock_);
    extSchedulingPoster_.reset(nullptr);
}

inline JobExecutionContext *JobWorkerThread::GetSchedulerExecutionCtx() const
{
    return schedulerExecutionCtx_;
}

inline void JobWorkerThread::SetSchedulerExecutionCtx(JobExecutionContext *executionCtx)
{
    schedulerExecutionCtx_ = executionCtx;
}

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_JOB_WORKER_THREAD_INL_H
