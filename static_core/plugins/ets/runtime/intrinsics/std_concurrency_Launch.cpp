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

#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_priority.h"
#include "runtime/execution/job_worker_group.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "runtime/include/runtime.h"
#include <libarkfile/include/source_lang_enum.h>
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_field.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_type.h"
#include "types/ets_job.h"
#include "types/ets_promise.h"
#include "types/ets_type_comptime_traits.h"
#include "mem/vm_handle.h"

#include <type_traits>

namespace ark::ets::intrinsics {

static EtsMethod *ResolveInvokeMethod(EtsExecutionContext *executionCtx, VMHandle<EtsObject> func)
{
    if (func.GetPtr() == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return nullptr;
    }
    if (!func->GetClass()->IsFunction()) {
        ThrowEtsException(executionCtx, PlatformTypes()->coreTypeError,
                          "Method have to be instance of std.core.Function");
    }
    auto *method = func->GetClass()->ResolveVirtualMethod(PlatformTypes(executionCtx)->coreFunctionUnsafeCall);
    ASSERT(method != nullptr);

    return method;
}

template <typename CoroResult>
// CC-OFFNXT(huge_method) solid logic
CoroResult *Launch(EtsObject *func, bool abortFlag, JobWorkerThreadGroup::Id groupId = JobWorkerThreadGroup::AnyId(),
                   bool postToMain = false)
{
    static_assert(std::is_same<CoroResult, EtsJob>::value || std::is_same<CoroResult, EtsPromise>::value);

    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    ASSERT(executionCtx != nullptr);
    if (func == nullptr) {
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return nullptr;
    }

    auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    if (jobMan->IsJobSwitchDisabled()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreInvalidJobOperationError,
                          "Cannot launch coroutines in the current context!");
        return nullptr;
    }
    if (groupId == JobWorkerThreadGroup::InvalidId()) {
        ThrowRuntimeException("Cannot launch with invalid group id!");
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    VMHandle<EtsObject> function(executionCtx->GetMT(), func->GetCoreType());
    EtsMethod *method = ResolveInvokeMethod(executionCtx, function);
    if (method == nullptr) {
        return nullptr;
    }

    // create the coro and put it to the ready queue
    EtsHandle<CoroResult> coroResultHandle(executionCtx, CoroResult::Create(executionCtx));
    if (UNLIKELY(coroResultHandle.GetPtr() == nullptr)) {
        return nullptr;
    }

    auto *storage = executionCtx->GetPandaVM()->GetGlobalObjectStorage();
    auto *ref = storage->Add(coroResultHandle.GetPtr()->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    auto *evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(ref, jobMan);

    // since transferring arguments from frame registers (which are local roots for GC) to a C++ vector
    // introduces the potential risk of pointer invalidation in case GC moves the referenced objects,
    // we would like to do this transfer below all potential GC invocation points
    auto argArray = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, 0U);
    if (UNLIKELY(argArray == nullptr)) {
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return nullptr;
    }

    auto realArgs = PandaVector<Value> {Value(function->GetCoreType()), Value(argArray->GetCoreType())};
    groupId = postToMain ? JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::MAIN) : groupId;

    auto epInfo = Job::ManagedEntrypointInfo {evt, method->GetPandaMethod(), std::move(realArgs)};
    auto *job = jobMan->CreateJob(method->GetFullName(), std::move(epInfo), EtsCoroutine::LAUNCH, Job::Type::MUTATOR,
                                  abortFlag);
    auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId});
    if (UNLIKELY(launchResult != LaunchResult::OK)) {
        // Launch failed. The exception in the current coro should be already set by Launch(),
        // just return null as the result and clean up the allocated resources.
        ASSERT(executionCtx->GetMT()->HasPendingException());
        if (launchResult == LaunchResult::RESOURCE_LIMIT_EXCEED) {
            jobMan->DestroyJob(job);
        }
        storage->Remove(ref);
        return nullptr;
    }

    return coroResultHandle.GetPtr();
}

extern "C" {
EtsJob *EtsLaunchInternalJobNative(EtsObject *func, EtsBoolean abortFlag, EtsLongArray *groupId)
{
    ASSERT(groupId->GetLength() == 2U);
    return Launch<EtsJob>(func, abortFlag != 0U, JobWorkerThreadGroup::FromTuple({groupId->Get(0), groupId->Get(1)}));
}

void EtsLaunchSameWorker(EtsObject *callback)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    EtsHandleScope scope(executionCtx);

    VMHandle<EtsObject> hCallback(executionCtx->GetMT(), callback->GetCoreType());
    ASSERT(hCallback.GetPtr() != nullptr);

    auto *method = ResolveInvokeMethod(executionCtx, hCallback);
    auto argArray = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, 0U);
    if (UNLIKELY(argArray == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    auto args = PandaVector<Value> {Value(hCallback->GetCoreType()), Value(argArray->GetCoreType())};
    auto *jobExecCtx = JobExecutionContext::CastFromMutator(executionCtx->GetMT());
    auto *jobMan = jobExecCtx->GetManager();
    auto *evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, jobMan);
    auto epInfo = Job::ManagedEntrypointInfo {evt, method->GetPandaMethod(), std::move(args)};
    auto groupId = JobWorkerThreadGroup::GenerateExactWorkerId(jobExecCtx->GetWorker()->GetId());
    auto *job = jobMan->CreateJob(method->GetFullName(), std::move(epInfo), EtsCoroutine::TIMER_CALLBACK,
                                  Job::Type::MUTATOR, true);
    auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId});
    if (UNLIKELY(launchResult == LaunchResult::RESOURCE_LIMIT_EXCEED)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        jobMan->DestroyJob(job);
    }
}

static PandaVector<JobWorkerThread::Id> ConvertEtsHintToNativeHint(EtsObject *compatArray)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> hCompatArray(executionCtx, compatArray);

    EtsField *actualLengthField = hCompatArray->GetClass()->GetFieldIDByName("actualLength", nullptr);
    ASSERT(actualLengthField != nullptr);
    auto actualLength = hCompatArray->GetFieldPrimitive<int32_t>(actualLengthField);

    EtsField *bufferField = hCompatArray->GetClass()->GetFieldIDByName("buffer", nullptr);
    ASSERT(bufferField != nullptr);
    EtsObjectArray *etsBuffer = EtsObjectArray::FromEtsObject(hCompatArray->GetFieldObject(bufferField));
    ASSERT(etsBuffer != nullptr);
    EtsHandle<EtsObjectArray> hEtsBuffer(executionCtx, etsBuffer);
    ASSERT(static_cast<size_t>(actualLength) <= hEtsBuffer->GetLength());

    PandaVector<JobWorkerThread::Id> hint;
    hint.reserve(actualLength);
    for (int i = 0; i < actualLength; i++) {
        hint.emplace_back(GetUnboxedValue(executionCtx, hEtsBuffer->Get(i)).GetAs<int32_t>());
    }
    return hint;
}

EtsLongArray *WorkerGroupGenerateGroupIdImpl(EtsInt domain, EtsObject *etsHint)
{
    auto domainId = static_cast<JobWorkerThreadDomain>(domain);
    auto hint = ConvertEtsHintToNativeHint(etsHint);
    auto groupId = JobWorkerThreadGroup::FromDomain(JobExecutionContext::GetCurrent()->GetManager(), domainId, hint);
    const auto [lower, upper] = JobWorkerThreadGroup::ToTuple(groupId);
    auto *array = EtsPrimitiveArray<EtsLong, EtsClassRoot::LONG_ARRAY>::Create(2U);
    array->Set(0, lower);
    array->Set(1, upper);
    return array;
}

}  // extern "C"

}  // namespace ark::ets::intrinsics
