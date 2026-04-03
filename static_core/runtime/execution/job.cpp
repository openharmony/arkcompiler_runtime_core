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

#include "runtime/execution/job.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/method.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"

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
    InvokeEntrypointImpl(false);
}

void Job::InvokeEntrypointImpl(bool resumedFrame)
{
    ASSERT(HasEntrypoint());
    ASSERT(GetExecutionContext() == JobExecutionContext::GetCurrent());

    if (HasManagedEntrypoint()) {
        auto executionCtx = GetExecutionContext();
        ASSERT(executionCtx != nullptr);
        auto invokeMethod = [this, executionCtx, resumedFrame]() {
            auto &args = GetManagedEntrypointArguments();
            auto callFlags = resumedFrame ? CallFlags {CallFlags::IS_RESUMED} : CallFlags {};
            Value result = GetManagedEntrypoint()->Invoke(executionCtx, args.data(), callFlags);
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

}  // namespace ark
