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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_LAUNCH_H
#define PANDA_RUNTIME_EXECUTION_JOB_LAUNCH_H

#include "runtime/execution/job_priority.h"
#include "runtime/execution/job_worker_group.h"

#include <cstdint>

namespace ark {

class JobEvent;

/// @brief defines result of launch
enum class LaunchResult : uint8_t {
    /// coroutine was created and scheduled successfully
    OK,
    /**
     * job was not launched because of resource limit was exceeded
     * NOTE: in case of this error job was not launched and should be removed externally
     */
    RESOURCE_LIMIT_EXCEED,
    /// job was not launched because no suitable worker was found
    NO_SUITABLE_WORKER,
    /**
     * coroutine was not created because it's launch method is not supported, for example take a look at
     * ThreadedCoroutineManager::LaunchImmediately
     */
    NOT_SUPPORTED
};

/// @brief set of launch parameters
struct LaunchParams {
    using WorkerGroup = JobWorkerThreadGroup::Id;

    explicit LaunchParams(JobPriority prio) : priority(prio), groupId(JobWorkerThreadGroup::AnyId()) {}
    explicit LaunchParams(JobPriority prio, const WorkerGroup &wGroup) : priority(prio), groupId(wGroup) {}
    explicit LaunchParams(JobPriority prio, const WorkerGroup &wGroup, JobEvent *evt)
        : priority(prio), groupId(wGroup), startEvent(evt)
    {
    }
    explicit LaunchParams(bool launchImm) : launchImmediately(launchImm) {}

    /// priority of launched job
    JobPriority priority = JobPriority::DEFAULT_PRIORITY;  // NOLINT(misc-non-private-member-variables-in-classes)
    /// set of workers that job can be scheduled on
    WorkerGroup groupId = JobWorkerThreadGroup::InvalidId();  // NOLINT(misc-non-private-member-variables-in-classes)
    /// launched job's dependency event (currently only TimerEvents are supported)
    JobEvent *startEvent = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    /// means job execution in-place on the same worker
    bool launchImmediately = false;  // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_JOB_LAUNCH_H
