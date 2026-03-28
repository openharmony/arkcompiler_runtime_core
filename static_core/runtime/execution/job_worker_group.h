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

#ifndef PANDA_RUNTIME_EXECUTION_JOB_WORKER_GROUP_H
#define PANDA_RUNTIME_EXECUTION_JOB_WORKER_GROUP_H

#include "runtime/execution/affinity_mask.h"
#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/job_worker_domain.h"

namespace ark {

class JobManager;

/**
 * @brief A group of job workers
 *
 * A job worker group is the set of workers with a unique ID that can be targeted at the job launch time.
 *
 * JobManager's Launch* APIs accept the JobWorkerThreadGroup unique ID and then schedule the new job to
 * on of the workers from the target worker group.
 */
class JobWorkerThreadGroup {
public:
    using Id = AffinityMask::Storage;

    JobWorkerThreadGroup() = delete;

    static Id InvalidId()
    {
        return Id().reset();
    }

    static Id AnyId()
    {
        return Id().set();
    }

    static Id Empty()
    {
        return Id().reset();
    }

    static Id ExtendGroup(const Id &group, JobWorkerThread::Id newWorkerId,
                          [[maybe_unused]] bool checkDuplicates = true)
    {
        ASSERT(newWorkerId != JobWorkerThread::INVALID_ID);
        ASSERT(!checkDuplicates || ((group & FromWorkerId(newWorkerId)) == Empty()));
        return group | FromWorkerId(newWorkerId);
    }

    static Id ShrinkGroup(const Id &group, JobWorkerThread::Id workerId)
    {
        ASSERT(workerId != JobWorkerThread::INVALID_ID);
        ASSERT((group & FromWorkerId(workerId)) != Empty());
        return group & ~FromWorkerId(workerId);
    }

    static JobWorkerThreadGroup::Id GenerateExactWorkerId(JobWorkerThread::Id hint)
    {
        ASSERT(hint != JobWorkerThread::INVALID_ID);
        return FromWorkerId(hint);
    }

    static bool HasOnlyWorker(const JobWorkerThreadGroup::Id &groupId, JobWorkerThread::Id workerId)
    {
        return groupId == FromWorkerId(workerId);
    }

    static bool HasWorker(const JobWorkerThreadGroup::Id &groupId, JobWorkerThread::Id workerId)
    {
        return (groupId & FromWorkerId(workerId)) != Empty();
    }

    // Converts 128 bitset into two uint64_t
    static std::tuple<uint64_t, uint64_t> ToTuple(const Id &groupId)
    {
        uint64_t lower = 0;
        uint64_t upper = 0;
        for (size_t i = 0; i < UL64; ++i) {
            if (groupId[i]) {
                lower |= static_cast<uint64_t>(1) << i;
            }
            if (groupId[i + UL64]) {
                upper |= static_cast<uint64_t>(1) << i;
            }
        }
        return {lower, upper};
    }

    // Converts two uint64_t into 128 bitset
    static Id FromTuple(const std::tuple<uint64_t, uint64_t> &t)
    {
        const auto &[lower, upper] = t;
        JobWorkerThreadGroup::Id groupId;
        for (size_t i = 0; i < UL64; ++i) {
            groupId[i] = (((lower >> i) & 1ULL) != 0U);
            groupId[i + UL64] = (((upper >> i) & 1ULL) != 0U);
        }
        return groupId;
    }

    static Id FromDomain(JobManager *jobMan, JobWorkerThreadDomain domain,
                         const PandaVector<JobWorkerThread::Id> &hint);

    static Id FromDomain(JobManager *jobMan, JobWorkerThreadDomain domain)
    {
        return FromDomain(jobMan, domain, {});
    }

private:
    static Id FromWorkerId(JobWorkerThread::Id newId)
    {
        auto group = Empty();
        group |= 1U;
        return group << (size_t)newId;
    }

    static constexpr uint64_t UL64 = 64U;
};

static_assert(std::is_same_v<JobWorkerThreadGroup::Id, AffinityMask::JobWorkerThreadGroupId>);

}  // namespace ark

#endif /* PANDA_RUNTIME_EXECUTION_JOB_WORKER_GROUP_H */
