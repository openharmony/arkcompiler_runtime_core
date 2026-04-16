/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "mem/refstorage/global_object_storage.h"
#include "plugins/ets/runtime/ets_call_stack.h"
#include "runtime/include/value.h"
#include "libarkbase/macros.h"
#include "mem/refstorage/reference.h"
#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/panda_vm.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "intrinsics.h"

namespace ark::ets {

EtsCoroutine::EtsCoroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, Job *job,
                           CoroutineContext *context)
    : Coroutine(id, allocator, vm, ark::panda_file::SourceLang::ETS, job, context), executionCtx_(this)
{
    ASSERT(vm != nullptr);
}

PandaEtsVM *EtsCoroutine::GetPandaVM() const
{
    return static_cast<PandaEtsVM *>(GetVM());
}

void EtsCoroutine::Initialize()
{
    executionCtx_.Initialize();
    Coroutine::Initialize();
}

void EtsCoroutine::ReInitialize(Job *job, CoroutineContext *context)
{
    executionCtx_.ReInitialize();
    Coroutine::ReInitialize(job, context);
}

void EtsCoroutine::CleanUp()
{
    executionCtx_.CleanUp();
    Coroutine::CleanUp();
    // add the required local storage entries cleanup here!
}

void EtsCoroutine::FreeInternalMemory()
{
    executionCtx_.GetPandaAniEnv()->FreeInternalMemory();
    ManagedThread::FreeInternalMemory();
}

void EtsCoroutine::RequestCompletion(Value returnValue)
{
    auto *completionObjRef = GetJob()->GetCompletionEvent()->ReleaseReturnValueObject();
    if (completionObjRef == nullptr) {
        Coroutine::RequestCompletion(returnValue);
        return;
    }
    auto *storage = GetVM()->GetGlobalObjectStorage();
    auto *completionObj = EtsObject::FromCoreType(storage->Get(completionObjRef));

    if (completionObj->IsInstanceOf(PlatformTypes(this)->corePromise)) {
        RequestPromiseCompletion(completionObjRef, returnValue);
    } else if (completionObj->IsInstanceOf(PlatformTypes(this)->coreJob)) {
        RequestJobCompletion(completionObjRef, returnValue);
    } else {
        UNREACHABLE();
    }
}

void EtsCoroutine::RequestJobCompletion(mem::Reference *jobRef, Value returnValue)
{
    auto *storage = GetVM()->GetGlobalObjectStorage();
    auto *job = EtsJob::FromCoreType(storage->Get(jobRef));
    storage->Remove(jobRef);
    if (job == nullptr) {
        LOG(DEBUG, COROUTINES)
            << "Coroutine \"" << GetName()
            << "\" has completed, but the associated job has been already collected by the GC. Exception thrown: "
            << HasPendingException();
        Coroutine::RequestCompletion(returnValue);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(GetExecutionCtx());
    EtsHandle<EtsJob> hjob(GetExecutionCtx(), job);
    EtsObject *retObject = nullptr;
    if (!HasPendingException()) {
        panda_file::Type returnType = GetReturnType();
        retObject = GetReturnValueAsObject(returnType, returnValue);
        if (retObject != nullptr) {
            LOG_IF(returnType.IsVoid(), DEBUG, COROUTINES) << "Coroutine \"" << GetName() << "\" has completed";
            LOG_IF(returnType.IsPrimitive(), DEBUG, COROUTINES)
                << "Coroutine \"" << GetName() << "\" has completed with return value 0x" << std::hex
                << returnValue.GetAs<uint64_t>();
            LOG_IF(returnType.IsReference(), DEBUG, COROUTINES)
                << "Coroutine \"" << GetName() << "\" has completed with return value = ObjectPtr<"
                << returnValue.GetAs<ObjectHeader *>() << ">";
        }
    }
    if (HasPendingException()) {
        // An exception may occur while boxin primitive return value in GetReturnValueAsObject
        auto *exc = GetException();
        if (!GetJob()->HasAbortFlag()) {
            ClearException();
        }
        LOG(INFO, COROUTINES) << "Coroutine \"" << GetName()
                              << "\" completed with an exception: " << exc->ClassAddr<Class>()->GetName();
        EtsJob::EtsJobFail(hjob.GetPtr(), EtsObject::FromCoreType(exc));
        return;
    }
    EtsJob::EtsJobFinish(hjob.GetPtr(), retObject);
}

void EtsCoroutine::RequestPromiseCompletion(mem::Reference *promiseRef, Value returnValue)
{
    auto *storage = GetVM()->GetGlobalObjectStorage();
    auto *promise = EtsPromise::FromCoreType(storage->Get(promiseRef));
    storage->Remove(promiseRef);
    if (promise == nullptr) {
        LOG(DEBUG, COROUTINES)
            << "Coroutine " << GetName()
            << " has completed, but the associated promise has been already collected by the GC. Exception thrown: "
            << HasPendingException();
        Coroutine::RequestCompletion(returnValue);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(GetExecutionCtx());
    EtsHandle<EtsPromise> hpromise(GetExecutionCtx(), promise);
    EtsObject *retObject = nullptr;
    if (!HasPendingException()) {
        panda_file::Type returnType = GetReturnType();
        retObject = GetReturnValueAsObject(returnType, returnValue);
        if (retObject != nullptr) {
            LOG_IF(returnType.IsVoid(), DEBUG, COROUTINES) << "Coroutine " << GetName() << " has completed";
            LOG_IF(returnType.IsPrimitive(), DEBUG, COROUTINES)
                << "Coroutine " << GetName() << " has completed with return value 0x" << std::hex
                << returnValue.GetAs<uint64_t>();
            LOG_IF(returnType.IsReference(), DEBUG, COROUTINES)
                << "Coroutine " << GetName() << " has completed with return value = ObjectPtr<"
                << returnValue.GetAs<ObjectHeader *>() << ">";
        }
    }
    if (retObject != nullptr && retObject->IsInstanceOf(PlatformTypes(this)->corePromise)) {
        // If the retObject is a rejected Promise, the reject reason will be set as an exception.
        retObject = GetValueFromPromiseSync(EtsPromise::FromEtsObject(retObject));
        if (retObject == nullptr) {
            LOG(INFO, COROUTINES) << "Coroutine " << GetName() << " completion by a promise retval went wrong";
        }
    }
    if (HasPendingException()) {
        // An exception may occur while boxin primitive return value in GetReturnValueAsObject
        auto *exc = GetException();
        ClearException();
        LOG(INFO, COROUTINES) << "Coroutine " << GetName()
                              << " completed with an exception: " << exc->ClassAddr<Class>()->GetName();
        intrinsics::EtsPromiseReject(hpromise.GetPtr(), EtsObject::FromCoreType(exc), ToEtsBoolean(false));
        return;
    }
    intrinsics::EtsPromiseResolve(hpromise.GetPtr(), retObject, ToEtsBoolean(false));
}

EtsObject *EtsCoroutine::GetValueFromPromiseSync(EtsPromise *promise)
{
    return intrinsics::EtsAwaitPromise(promise);
}

panda_file::Type EtsCoroutine::GetReturnType()
{
    Method *entrypoint = GetJob()->GetManagedEntrypoint();
    ASSERT(entrypoint != nullptr);
    return entrypoint->GetReturnType();
}

// The result will be used to resolve a promise, so this function perfoms a "box" operation on ark::Value
EtsObject *EtsCoroutine::GetReturnValueAsObject(panda_file::Type returnType, Value returnValue)
{
    auto *executionCtx = GetExecutionCtx();
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
            LOG(FATAL, COROUTINES) << "Unsupported return type: " << returnType;
            break;
    }
    return nullptr;
}

void EtsCoroutine::UpdateCachedObjects()
{
    // update the interop context pointer
    auto *worker = GetWorker();
    ASSERT(worker != nullptr);
    auto *ptr = worker->GetLocalStorage().Get<JobWorkerThread::DataIdx::INTEROP_CTX_PTR, void *>();
    executionCtx_.GetLocalStorage().Set<EtsExecutionContext::DataIdx::INTEROP_CTX_PTR>(ptr);

    if (GetType() == Coroutine::Type::MUTATOR) {
        ASSERT_MANAGED_CODE();
        auto *gStorage = GetVM()->GetGlobalObjectStorage();
        auto &lStorage = worker->GetLocalStorage();

        // update the flattenedString cache pointer
        auto *cacheRef = lStorage.Get<CoroutineWorker::DataIdx::FLATTENED_STRING_CACHE, mem::Reference *>();
        SetFlattenedStringCache(gStorage->Get(cacheRef));

        // update toString cache pointers
        if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
            auto *cacheRefD = lStorage.Get<CoroutineWorker::DataIdx::DOUBLE_TO_STRING_CACHE, mem::Reference *>();
            SetDoubleToStringCache(gStorage->Get(cacheRefD));
            auto *cacheRefF = lStorage.Get<CoroutineWorker::DataIdx::FLOAT_TO_STRING_CACHE, mem::Reference *>();
            SetFloatToStringCache(gStorage->Get(cacheRefF));
            auto *cacheRefL = lStorage.Get<CoroutineWorker::DataIdx::LONG_TO_STRING_CACHE, mem::Reference *>();
            SetLongToStringCache(gStorage->Get(cacheRefL));
        }
    }
}

void EtsCoroutine::OnContextSwitchedTo()
{
    if ((GetPriority() == CoroutinePriority::MEDIUM_PRIORITY) && (GetType() == Coroutine::Type::MUTATOR)) {
        executionCtx_.ProcessUnhandledRejectedPromises(false);
    }
}

void EtsCoroutine::OnChildCoroutineCreated(Coroutine *child)
{
    Coroutine::OnChildCoroutineCreated(child);
    EtsCoroutine::CastFromThread(child)->GetExecutionCtx()->SetTaskpoolTaskId(executionCtx_.GetTaskpoolTaskId());
}

void EtsCoroutine::HandleUncaughtException()
{
    ASSERT(HasPendingException());
    GetPandaVM()->HandleUncaughtException();
}

void EtsCoroutine::ListUnhandledEventsOnProgramExit()
{
    ProcessUnhandledFailedJobs();
    executionCtx_.ProcessUnhandledRejectedPromises(true);
}

void EtsCoroutine::ProcessUnhandledFailedJobs()
{
    executionCtx_.ProcessUnhandledFailedJobs();
}

bool EtsCoroutine::IsContextSwitchRisky() const
{
    auto *callStk =
        executionCtx_.GetLocalStorage().Get<EtsExecutionContext::DataIdx::INTEROP_CALL_STACK_PTR, EtsCallStack *>();
    return callStk != nullptr && callStk->Current() != nullptr;
}

void EtsCoroutine::PrintCallStack() const
{
    auto *callStk =
        executionCtx_.GetLocalStorage().Get<EtsExecutionContext::DataIdx::INTEROP_CALL_STACK_PTR, EtsCallStack *>();
    if (callStk == nullptr) {
        Coroutine::PrintCallStack();
        return;
    }
    auto istk = callStk->GetRecords();
    auto istkIt = istk.rbegin();

    auto printIstkFrames = [&istkIt, &istk](void *fp) {
        while (istkIt != istk.rend() && fp == istkIt->frame) {
            LOG(ERROR, COROUTINES) << "<interop> " << (istkIt->descr != nullptr ? istkIt->descr : "unknown");
            istkIt++;
        }
    };

    for (auto stack = StackWalker::Create(this); stack.HasFrame(); stack.NextFrame()) {
        printIstkFrames((istkIt != istk.rend()) ? istkIt->frame : nullptr);
        Method *method = stack.GetMethod();
        ASSERT(method != nullptr);
        LOG(ERROR, COROUTINES) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                               << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    ASSERT(istkIt == istk.rend() || !istkIt->isStaticFrame);
    printIstkFrames(nullptr);
}

}  // namespace ark::ets
