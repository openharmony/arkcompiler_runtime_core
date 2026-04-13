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
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_events.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {

static EtsObject *GetReturnValueAsObject(EtsExecutionContext *executionCtx, panda_file::Type returnType,
                                         Value returnValue);

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

void EtsExecutionContextWrapper::OnJobCompletion(Value result)
{
    auto *completionEvt = GetJob()->GetCompletionEvent();
    if (completionEvt == nullptr) {
        return;
    }
    auto *retValueRef = completionEvt->ReleaseReturnValueObject();
    if (retValueRef == nullptr) {
        JobExecutionContext::OnJobCompletion(result);
        return;
    }

    EtsObject *retObject = nullptr;
    if (!HasPendingException()) {
        panda_file::Type returnType = GetJob()->GetManagedEntrypoint()->GetReturnType();
        retObject = GetReturnValueAsObject(GetExecutionCtx(), returnType, result);
    }

    auto *storage = executionCtx_.GetPandaVM()->GetGlobalObjectStorage();
    auto *completedJob = EtsJob::FromCoreType(storage->Get(retValueRef));
    storage->Remove(retValueRef);
    ASSERT(completedJob != nullptr);
    ASSERT(completedJob->GetClass() == PlatformTypes(GetExecutionCtx())->coreJob);

    // An exception may occur while boxin primitive return value in GetReturnValueAsObject
    if (HasPendingException()) {
        auto *exc = GetException();
        if (!GetJob()->HasAbortFlag()) {
            ClearException();
        }
        EtsJob::EtsJobFail(completedJob, EtsObject::FromCoreType(exc));
        return;
    }
    EtsJob::EtsJobFinish(completedJob, retObject);
}

void EtsExecutionContextWrapper::ListUnhandledEventsOnProgramExit()
{
    executionCtx_.ProcessUnhandledRejectedPromises(true);
}

static EtsObject *GetReturnValueAsObject(EtsExecutionContext *executionCtx, panda_file::Type returnType,
                                         Value returnValue)
{
    switch (returnType.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return nullptr;  // a representation of ets "undefined"
        case panda_file::Type::TypeId::U1:
            return EtsBoxPrimitive<EtsBoolean>::Create(executionCtx, returnValue.GetAs<EtsBoolean>());
        case panda_file::Type::TypeId::I8:
            return EtsBoxPrimitive<EtsByte>::Create(executionCtx, returnValue.GetAs<EtsByte>());
        case panda_file::Type::TypeId::I16:
            return EtsBoxPrimitive<EtsShort>::Create(executionCtx, returnValue.GetAs<EtsShort>());
        case panda_file::Type::TypeId::U16:
            return EtsBoxPrimitive<EtsChar>::Create(executionCtx, returnValue.GetAs<EtsChar>());
        case panda_file::Type::TypeId::I32:
            return EtsBoxPrimitive<EtsInt>::Create(executionCtx, returnValue.GetAs<EtsInt>());
        case panda_file::Type::TypeId::F32:
            return EtsBoxPrimitive<EtsFloat>::Create(executionCtx, returnValue.GetAs<EtsFloat>());
        case panda_file::Type::TypeId::F64:
            return EtsBoxPrimitive<EtsDouble>::Create(executionCtx, returnValue.GetAs<EtsDouble>());
        case panda_file::Type::TypeId::I64:
            return EtsBoxPrimitive<EtsLong>::Create(executionCtx, returnValue.GetAs<EtsLong>());
        case panda_file::Type::TypeId::REFERENCE:
            return EtsObject::FromCoreType(returnValue.GetAs<ObjectHeader *>());
        default:
            LOG(FATAL, EXECUTION) << "Unsupported return type: " << returnType;
            break;
    }
    return nullptr;
}

}  // namespace ark::ets
