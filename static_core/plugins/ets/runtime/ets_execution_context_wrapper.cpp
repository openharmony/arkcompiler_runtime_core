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

#include "plugins/ets/runtime/ets_execution_context_wrapper.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/job_manager.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {

EtsExecutionContextWrapper::EtsExecutionContextWrapper(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                                                       Job *job)
    : JobExecutionContext(id, allocator, vm, ThreadType::THREAD_TYPE_WORKER_THREAD, ark::panda_file::SourceLang::ETS,
                          job),
      executionCtx_(this)
{
    ASSERT(vm != nullptr);
}

void EtsExecutionContextWrapper::Initialize()
{
    executionCtx_.Initialize();
    GetManager()->RegisterExecutionContext(this);
    GetJob()->SetStatus(Job::Status::RUNNABLE);
}

void EtsExecutionContextWrapper::UpdateCachedObjects()
{
    // update the interop context pointer
    auto *worker = GetWorker();
    auto *ptr = worker->GetLocalStorage().Get<JobWorkerThread::DataIdx::INTEROP_CTX_PTR, void *>();
    executionCtx_.GetLocalStorage().Set<EtsExecutionContext::DataIdx::INTEROP_CTX_PTR>(ptr);

    ASSERT_MANAGED_CODE();
    // update the string cache pointer
    auto *cacheRef =
        worker->GetLocalStorage().Get<JobWorkerThread::DataIdx::FLATTENED_STRING_CACHE, mem::Reference *>();
    auto *cache = GetVM()->GetGlobalObjectStorage()->Get(cacheRef);
    SetFlattenedStringCache(cache);
}

void EtsExecutionContextWrapper::CacheBuiltinClasses()
{
    executionCtx_.CacheBuiltinClasses();
}

}  // namespace ark::ets
