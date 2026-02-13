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

#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/execution/job.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_events.h"

#include <utility>
#include <variant>

namespace ark {

/* static */
void Job::SetCurrent(Job *job)
{
    JobExecutionContext::CastFromMutator(ManagedThread::GetCurrent())->SetJob(job);
}

Job *Job::GetCurrent()
{
    return JobExecutionContext::CastFromMutator(ManagedThread::GetCurrent())->GetJob();
}

Job::ManagedEntrypointInfo::ManagedEntrypointInfo(ManagedEntrypointInfo &&epInfo)
    : completionEvent(std::exchange(epInfo.completionEvent, nullptr)),
      entrypoint(epInfo.entrypoint),
      arguments(std::move(epInfo.arguments))
{
}

Job::ManagedEntrypointInfo &Job::ManagedEntrypointInfo::operator=(ManagedEntrypointInfo &&epInfo)
{
    if (this == &epInfo) {
        return *this;
    }
    completionEvent = std::exchange(epInfo.completionEvent, completionEvent);
    entrypoint = epInfo.entrypoint;
    arguments = std::move(epInfo.arguments);
    return *this;
}

Job::ManagedEntrypointInfo::~ManagedEntrypointInfo()
{
    // delete the event as it is owned by the ManagedEntrypointData instance
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(completionEvent);
}

void Job::InvokeEntrypoint()
{
    ASSERT(HasEntrypoint());
    ASSERT(GetExecutionContext() == JobExecutionContext::GetCurrent());

    if (HasManagedEntrypoint()) {
        auto executionCtx = GetExecutionContext();
        ASSERT(executionCtx != nullptr);
        auto invokeMethod = [this, executionCtx]() {
            auto &args = GetManagedEntrypointArguments();
            Value result = GetManagedEntrypoint()->Invoke(executionCtx, args.data());
            if (GetStatus() != Job::Status::BLOCKED) {
                executionCtx->OnJobCompletion(result);
            }
        };
        if (executionCtx->IsInNativeCode()) {
            ScopedManagedCodeThread s(executionCtx);
            invokeMethod();
        } else {
            invokeMethod();
        }
    } else if (HasNativeEntrypoint()) {
        GetNativeEntrypoint()(GetNativeEntrypointParam());
    }
}

/* static */
Job *Job::Create(PandaString name, Id id, EntrypointInfo &&epInfo, JobPriority priority, Type type, bool abortFlag)
{
    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *job = allocator->New<Job>(std::move(name), id, std::move(epInfo), priority, type, abortFlag);
    return job;
}

/* static */
void Job::Destroy(Job *job)
{
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(job);
}

}  // namespace ark
