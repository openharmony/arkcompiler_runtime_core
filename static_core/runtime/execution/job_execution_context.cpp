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

#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"

namespace ark {

/* static */
JobExecutionContext *JobExecutionContext::Create(Runtime *runtime, PandaVM *vm, Job *job)
{
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    auto *executionCtx = allocator->New<JobExecutionContext>(os::thread::GetCurrentThreadId(), allocator, vm,
                                                             ManagedThread::ThreadType::THREAD_TYPE_WORKER_THREAD,
                                                             ark::panda_file::SourceLang::PANDA_ASSEMBLY, job);
    ASSERT(executionCtx != nullptr);
    return executionCtx;
}

JobExecutionContext::JobExecutionContext(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                                         ManagedThread::ThreadType threadType, ark::panda_file::SourceLang threadLang,
                                         Job *job)
    : ManagedThread(id, allocator, vm, threadType, threadLang),
      currentJob_(job),
      manager_(static_cast<JobManager *>(vm->GetThreadManager()))
{
    ASSERT(currentJob_ != nullptr);
}

JobExecutionContext::~JobExecutionContext()
{
    GetManager()->DestroyJob(GetJob());
}

void JobExecutionContext::ExecuteJob(Job *job)
{
    ASSERT(GetCurrent() == this);
    auto *parentJ = GetJob();
    job->SetExecutionContext(this);
    SetJob(job);

    parentJ->SetStatus(Job::Status::YIELDED);
    job->SetStatus(Job::Status::RUNNING);
    job->InvokeEntrypoint();
    parentJ->SetStatus(Job::Status::RUNNING);

    SetJob(parentJ);
    job->SetExecutionContext(nullptr);
}

void JobExecutionContext::OnJobCompletion([[maybe_unused]] Value result)
{
    if (GetJob()->GetCompletionEvent() != nullptr) {
        GetJob()->GetCompletionEvent()->Happen();
    }
}

}  // namespace ark
