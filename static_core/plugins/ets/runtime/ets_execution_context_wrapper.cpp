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
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/intrinsics/helpers/intrinsic_promise_impl.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/execution/job-inl.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_events.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_worker_thread.h"
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

EtsObject *EtsExecutionContextWrapper::BoxReturnValue(Value returnValue)
{
    panda_file::Type returnType = GetJob()->GetManagedEntrypoint()->GetReturnType();
    auto *ctx = GetExecutionCtx();
    switch (returnType.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return nullptr;  // a representation of ets "undefined"
        case panda_file::Type::TypeId::U1:
            return EtsBoxPrimitive<EtsBoolean>::Create(ctx, returnValue.GetAs<EtsBoolean>());
        case panda_file::Type::TypeId::I8:
            return EtsBoxPrimitive<EtsByte>::Create(ctx, returnValue.GetAs<EtsByte>());
        case panda_file::Type::TypeId::I16:
            return EtsBoxPrimitive<EtsShort>::Create(ctx, returnValue.GetAs<EtsShort>());
        case panda_file::Type::TypeId::U16:
            return EtsBoxPrimitive<EtsChar>::Create(ctx, returnValue.GetAs<EtsChar>());
        case panda_file::Type::TypeId::I32:
            return EtsBoxPrimitive<EtsInt>::Create(ctx, returnValue.GetAs<EtsInt>());
        case panda_file::Type::TypeId::F32:
            return EtsBoxPrimitive<EtsFloat>::Create(ctx, returnValue.GetAs<EtsFloat>());
        case panda_file::Type::TypeId::F64:
            return EtsBoxPrimitive<EtsDouble>::Create(ctx, returnValue.GetAs<EtsDouble>());
        case panda_file::Type::TypeId::I64:
            return EtsBoxPrimitive<EtsLong>::Create(ctx, returnValue.GetAs<EtsLong>());
        case panda_file::Type::TypeId::REFERENCE:
            return EtsObject::FromCoreType(returnValue.GetAs<ObjectHeader *>());
        default:
            LOG(FATAL, COROUTINES) << "Unsupported return type: " << returnType;
            return nullptr;
    }
}

EtsObject *EtsExecutionContextWrapper::GetCompletionObject(mem::Reference *retValueRef)
{
    auto *storage = executionCtx_.GetPandaVM()->GetGlobalObjectStorage();
    auto *completionObj = EtsObject::FromCoreType(storage->Get(retValueRef));
    storage->Remove(retValueRef);
    return completionObj;
}

EtsObject *EtsExecutionContextWrapper::TakePendingException(Job *job)
{
    auto *exc = GetException();
    if (!job->HasAbortFlag()) {
        ClearException();
    }
    return EtsObject::FromCoreType(exc);
}

void EtsExecutionContextWrapper::CompletePromise(EtsPromise *completedPromise, EtsObject *retObject, Job *job)
{
    if (HasPendingException()) {
        auto *exc = TakePendingException(job);
        intrinsics::helpers::EtsPromiseRejectImpl(GetExecutionCtx(), completedPromise, exc);
        return;
    }

    intrinsics::helpers::EtsPromiseResolveImpl(GetExecutionCtx(), completedPromise, retObject);
}

void EtsExecutionContextWrapper::OnJobCompletion(Value returnValue)
{
    auto *job = GetJob();
    ASSERT(job != nullptr);
    if (!job->HasManagedEntrypoint()) {
        return;
    }

    auto *completionEvt = job->GetCompletionEvent();
    if (completionEvt == nullptr) {
        return;
    }
    auto *retValueRef = completionEvt->ReleaseReturnValueObject();
    if (retValueRef == nullptr) {
        JobExecutionContext::OnJobCompletion(returnValue);
        return;
    }

    auto *retObject = HasPendingException() ? nullptr : BoxReturnValue(returnValue);
    auto *completionObj = GetCompletionObject(retValueRef);

    [[maybe_unused]] auto *platformTypes = PlatformTypes(GetExecutionCtx());
    ASSERT(completionObj->IsInstanceOf(platformTypes->corePromise));

    [[maybe_unused]] EtsHandleScope scope(GetExecutionCtx());
    EtsHandle<EtsPromise> completedPromise(GetExecutionCtx(), EtsPromise::FromEtsObject(completionObj));
    CompletePromise(completedPromise.GetPtr(), retObject, job);
}

void EtsExecutionContextWrapper::VisitGCRoots(const GCRootVisitor &cb)
{
    JobExecutionContext::VisitGCRoots(cb);

    if (asyncContext_ != nullptr) {
        cb({mem::RootType::ROOT_THREAD, reinterpret_cast<ObjectHeader **>(&asyncContext_)});
    }
}

}  // namespace ark::ets
