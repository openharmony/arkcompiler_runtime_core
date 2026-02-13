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
#ifndef PANDA_RUNTIME_EXECUTION_STACKLESS_SUSPENDABLE_JOB_H
#define PANDA_RUNTIME_EXECUTION_STACKLESS_SUSPENDABLE_JOB_H

#include "mem/refstorage/reference.h"
#include "runtime/execution/job.h"

namespace ark {

/**
 * @brief A Job that can be suspended and resumed asynchronously.
 *
 * SuspendableJob extends Job to support asynchronous operations that require
 * the job to be suspended while waiting for external events (I/O, timers,
 * other jobs, etc.) and later resumed.
 *
 * The async context holds the state needed to resume execution after the
 * blocking operation completes. This is used for:
 * - Async/await patterns
 * - Promise-based concurrency
 * - Event loop integration
 *
 * @see Job
 * @see JobExecutionContext
 */
class SuspendableJob : public Job {
public:
    mem::Reference *GetSuspensionContext()
    {
        return suspensionCtx_;
    }

    void SetSuspensionContext(mem::Reference *suspensionCtx)
    {
        suspensionCtx_ = suspensionCtx;
    }

private:
    mem::Reference *suspensionCtx_ = nullptr;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_STACKLESS_SUSPENDABLE_JOB_H
