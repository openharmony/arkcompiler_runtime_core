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

#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_execution_context.h"

namespace ark {

void JobManager::DestroyJob(Job *job)
{
    if (job == nullptr) {
        return;
    }
    FreeJobId(job->GetId());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(job);
}

void JobManager::InitializeWorkerIdAllocator()
{
    os::memory::LockHolder lh(workerIdLock_);
    for (auto id = MAIN_WORKER_ID; id <= MAX_WORKER_ID; ++id) {
        freeWorkerIds_.push_back(id);
    }
}

JobWorkerThread::Id JobManager::AllocateWorkerId()
{
    os::memory::LockHolder lh(workerIdLock_);
    ASSERT(!freeWorkerIds_.empty());
    auto workerId = freeWorkerIds_.front();
    freeWorkerIds_.pop_front();
    return workerId;
}

void JobManager::ReleaseWorkerId(JobWorkerThread::Id workerId)
{
    os::memory::LockHolder lh(workerIdLock_);
    freeWorkerIds_.push_back(workerId);
    ASSERT(freeWorkerIds_.size() <= AffinityMask::MAX_WORKERS_COUNT);
}

static JobWorkerThreadGroup::Id TryApplyHint(const JobWorkerThreadGroup::Id &group,
                                             const PandaVector<JobWorkerThread::Id> &hint)
{
    JobWorkerThreadGroup::Id hintGroup = JobWorkerThreadGroup::Empty();
    for (auto h : hint) {
        ASSERT(h != JobWorkerThread::INVALID_ID);
        hintGroup = JobWorkerThreadGroup::ExtendGroup(hintGroup, h, false);
    }
    return ((group & hintGroup) != JobWorkerThreadGroup::Empty()) ? (group & hintGroup) : group;
}

JobWorkerThreadGroup::Id JobManager::GenerateWorkerGroupId(JobWorkerThreadDomain domain,
                                                           const PandaVector<JobWorkerThread::Id> &hint)
{
    switch (domain) {
        case JobWorkerThreadDomain::GENERAL:
            return TryApplyHint(generalWorkerGroup_, hint);
        case JobWorkerThreadDomain::EXACT_ID:
            return TryApplyHint(JobWorkerThreadGroup::AnyId(), hint);
        case JobWorkerThreadDomain::MAIN:
            // Ignore hint
            return JobWorkerThreadGroup::GenerateExactWorkerId(MAIN_WORKER_ID);
        case JobWorkerThreadDomain::EXCLUSIVE:
            return TryApplyHint(eaWorkerGroup_, hint);
    }
    UNREACHABLE();
    return JobWorkerThreadGroup::InvalidId();
}

uint32_t JobManager::AllocateJobId()
{
    // Taken by copy-paste from MTThreadManager. Need to generalize if possible.
    // NOTE(konstanting, #IAD5MH): try to generalize internal ID allocation
    os::memory::LockHolder lock(idsLock_);
    for (size_t i = 0; i < jobIds_.size(); i++) {
        lastJobId_ = (lastJobId_ + 1) % jobIds_.size();
        if (!jobIds_[lastJobId_]) {
            jobIds_.set(lastJobId_);
            return lastJobId_ + 1;  // 0 is reserved as uninitialized value.
        }
    }
    LOG(FATAL, COROUTINES) << "Out of Job ids";
    UNREACHABLE();
}

void JobManager::FreeJobId(uint32_t id)
{
    // Taken by copy-paste from MTThreadManager. Need to generalize if possible.
    // NOTE(konstanting, #IAD5MH): try to generalize internal ID allocation
    id--;  // 0 is reserved as uninitialized value.
    os::memory::LockHolder lock(idsLock_);
    ASSERT(jobIds_[id]);
    jobIds_.reset(id);
}

void JobManager::OnWorkerStartup(JobWorkerThread *worker)
{
    if (worker->IsMainWorker()) {
        return;
    }
    if (!worker->InExclusiveMode()) {
        generalWorkerGroup_ = JobWorkerThreadGroup::ExtendGroup(generalWorkerGroup_, worker->GetId());
        return;
    }
    eaWorkerGroup_ = JobWorkerThreadGroup::ExtendGroup(eaWorkerGroup_, worker->GetId());
}

void JobManager::OnWorkerShutdown(JobWorkerThread *worker)
{
    auto workerId = worker->GetId();
    if (workerId != MAIN_WORKER_ID) {
        ASSERT(JobWorkerThreadGroup::HasWorker(generalWorkerGroup_, workerId) ||
               JobWorkerThreadGroup::HasWorker(eaWorkerGroup_, workerId));
        if (JobWorkerThreadGroup::HasWorker(generalWorkerGroup_, workerId)) {
            generalWorkerGroup_ = JobWorkerThreadGroup::ShrinkGroup(generalWorkerGroup_, workerId);
        } else {
            eaWorkerGroup_ = JobWorkerThreadGroup::ShrinkGroup(eaWorkerGroup_, workerId);
        }
    }
}

}  // namespace ark
