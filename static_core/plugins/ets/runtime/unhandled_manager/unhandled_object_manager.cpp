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

#include "plugins/ets/runtime/unhandled_manager/unhandled_object_manager.h"
#include "runtime/execution/job_launch.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/include/method.h"

namespace ark::ets {

static DefaultHandlerMode FromString(const std::string &s)
{
    if (s == "throw") {
        return DefaultHandlerMode::THROW;
    }
    if (s == "warn") {
        return DefaultHandlerMode::WARN;
    }
    if (s == "none") {
        return DefaultHandlerMode::NONE;
    }
    return DefaultHandlerMode::INVALID;
}

UnhandledObjectManager::UnhandledObjectManager(PandaEtsVM *vm)
    : vm_(vm),
      jobHandlerMode_(FromString(vm->GetOptions().GetListUnhandledOnExitJobs())),
      promiseHandlerMode_(FromString(vm->GetOptions().GetListUnhandledOnExitPromises()))
{
    ASSERT(jobHandlerMode_ != DefaultHandlerMode::INVALID);
    ASSERT(promiseHandlerMode_ != DefaultHandlerMode::INVALID);
}

static void UpdateObjectsImpl(PandaUnorderedSet<EtsObject *> &objects, const GCRootUpdater &gcRootUpdater)
{
    PandaVector<PandaUnorderedSet<EtsObject *>::node_type> movedObjects {};
    for (auto iter = objects.begin(); iter != objects.end();) {
        auto *obj = (*iter)->GetCoreType();
        [[maybe_unused]] auto *oldObj = obj;
        // assuming that `gcRootUpdater(&obj) == false` means that `obj` was not modified
        if (gcRootUpdater(&obj)) {
            auto oldIter = iter;
            ++iter;
            auto node = objects.extract(oldIter);
            node.value() = EtsObject::FromCoreType(obj);
            movedObjects.push_back(std::move(node));
        } else {
            ASSERT(obj == oldObj);
            ++iter;
        }
    }
    for (auto &node : movedObjects) {
        objects.insert(std::move(node));
    }
}

void UnhandledObjectManager::UpdateObjects(const GCRootUpdater &gcRootUpdater)
{
    os::memory::LockHolder lh(mutex_);
    UpdateObjectsImpl(failedJobs_, gcRootUpdater);
    for (auto &it : rejectedPromises_) {
        UpdateObjectsImpl(it.second, gcRootUpdater);
    }
}

static void VisitObjectsImpl(PandaUnorderedSet<EtsObject *> &objects, const GCRootVisitor &visitor)
{
    PandaVector<EtsObject *> toInsert;
    for (auto it = objects.begin(); it != objects.end();) {
        ObjectHeader *obj = (*it)->GetCoreType();
        ObjectPointerType pointer = ToObjPtr(obj);
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, &pointer));
        if (pointer != ToObjPtr(obj)) {
            toInsert.push_back(reinterpret_cast<EtsObject *>(pointer));
            auto delIt = it++;
            objects.erase(delIt);
        } else {
            ++it;
        }
    }
    for (auto pointer : toInsert) {
        objects.insert(pointer);
    }
}

void UnhandledObjectManager::VisitObjects(const GCRootVisitor &visitor)
{
    os::memory::LockHolder lh(mutex_);
    VisitObjectsImpl(failedJobs_, visitor);
    for (auto &it : rejectedPromises_) {
        VisitObjectsImpl(it.second, visitor);
    }
}

static PandaUnorderedSet<EtsObject *> FlattenMapOfSets(
    PandaUnorderedMap<JobWorkerThread::Id, PandaUnorderedSet<EtsObject *>> &mapOfSets)
{
    PandaUnorderedSet<EtsObject *> result;
    size_t count = 0;
    for (const auto &[_, set] : mapOfSets) {
        count += set.size();
    }
    result.reserve(count);

    for (auto &[_, set] : mapOfSets) {
        result.merge(std::move(set));
    }
    return result;
}

static void AddObjectImpl(PandaUnorderedSet<EtsObject *> &objects, EtsObject *object)
{
    objects.insert(object);
}

static void RemoveObjectImpl(PandaUnorderedSet<EtsObject *> &objects, EtsObject *object)
{
    objects.erase(object);
}

static PandaVector<EtsHandle<EtsObject>> TransformToVectorOfHandles(EtsExecutionContext *executionCtx,
                                                                    const PandaUnorderedSet<EtsObject *> &objects)
{
    ASSERT(executionCtx != nullptr);
    PandaVector<EtsHandle<EtsObject>> handleVec;
    handleVec.reserve(objects.size());
    for (auto *obj : objects) {
        ASSERT(!obj->GetCoreType()->IsForwarded());
        handleVec.emplace_back(executionCtx, obj);
    }
    return handleVec;
}

template <typename T>
static EtsHandle<EtsEscompatArray> CreateEtsObjectArrayFromHandles(EtsExecutionContext *executionCtx,
                                                                   const PandaVector<EtsHandle<EtsObject>> &handles)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    ASSERT(executionCtx != nullptr);
    EtsHandle<EtsEscompatArray> arrayH(executionCtx, EtsEscompatArray::Create(executionCtx, handles.size()));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return EtsHandle<EtsEscompatArray>(executionCtx, nullptr);
    }
    size_t i = 0;
    for (auto hobj : handles) {
        auto *objAndReason = EtsEscompatArray::Create(executionCtx, 2U);
        if (UNLIKELY(objAndReason == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return EtsHandle<EtsEscompatArray>(executionCtx, nullptr);
        }
        objAndReason->EscompatArraySetUnsafe(0, T::FromEtsObject(hobj.GetPtr())->GetValue(executionCtx));
        objAndReason->EscompatArraySetUnsafe(1, hobj.GetPtr());

        arrayH->EscompatArraySetUnsafe(i, objAndReason);
        ++i;
    }
    return arrayH;
}

template <typename T>
static void ListObjectsFromEtsArray(EtsClassLinker *etsClassLinker, EtsExecutionContext *executionCtx,
                                    EtsHandle<EtsEscompatArray> &hobjects, DefaultHandlerMode handlerMode)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    auto *platformTypes = etsClassLinker->GetEtsClassLinkerExtension()->GetPlatformTypes();
    Method *method {};
    if constexpr (std::is_same_v<T, EtsJob>) {
        method = platformTypes->coreStdProcessListUnhandledJobs->GetPandaMethod();
        LOG(DEBUG, COROUTINES) << "List unhandled failed jobs";
    } else if (std::is_same_v<T, EtsPromise>) {
        method = platformTypes->coreStdProcessListUnhandledPromises->GetPandaMethod();
        LOG(DEBUG, COROUTINES) << "List unhandled rejected promises";
    }
    ASSERT(method != nullptr);
    ASSERT(handlerMode != DefaultHandlerMode::INVALID);
    auto defaultHandlerMode = static_cast<int>(handlerMode);
    auto *jobExecCtx = JobExecutionContext::CastFromMutator(executionCtx->GetMT());
    auto *jobMan = jobExecCtx->GetManager();
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, jobMan);
    PandaVector<Value> args = {Value(hobjects->GetCoreType()), Value(defaultHandlerMode)};
    auto epInfo = Job::ManagedEntrypointInfo {evt, method, std::move(args)};
    auto *job =
        jobMan->CreateJob(method->GetFullName(), std::move(epInfo), EtsCoroutine::ASYNC_CALL, Job::Type::MUTATOR, true);
    LaunchResult launchRes = jobMan->Launch(job, LaunchParams {true});
    if UNLIKELY (launchRes != LaunchResult::OK) {
        LOG(DEBUG, COROUTINES) << "Failed to list unhandled rejections";
        ASSERT(launchRes == LaunchResult::RESOURCE_LIMIT_EXCEED);
        jobMan->DestroyJob(job);
        return;
    }
    LOG(DEBUG, COROUTINES) << "List unhandled rejections end";
}

template <typename T>
static void ListUnhandledObjectsImpl(EtsClassLinker *etsClassLinker, EtsExecutionContext *executionCtx,
                                     const PandaUnorderedSet<EtsObject *> &objects, DefaultHandlerMode handlerMode)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    ASSERT(executionCtx != nullptr);
    ASSERT(etsClassLinker != nullptr);
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    auto handles = TransformToVectorOfHandles(executionCtx, objects);
    auto hEtsArray = CreateEtsObjectArrayFromHandles<T>(executionCtx, handles);
    if (UNLIKELY(hEtsArray.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }
    ListObjectsFromEtsArray<T>(etsClassLinker, executionCtx, hEtsArray, handlerMode);
}

void UnhandledObjectManager::AddFailedJob(EtsJob *job)
{
    os::memory::LockHolder lh(mutex_);
    AddObjectImpl(failedJobs_, job->AsObject());
}

void UnhandledObjectManager::RemoveFailedJob(EtsJob *job)
{
    os::memory::LockHolder lh(mutex_);
    RemoveObjectImpl(failedJobs_, job->AsObject());
}

void UnhandledObjectManager::ListFailedJobs(EtsExecutionContext *executionCtx)
{
    ASSERT_MANAGED_CODE();
    ASSERT(executionCtx != nullptr);
    PandaUnorderedSet<EtsObject *> unhandledObjects {};
    {
        os::memory::LockHolder lh(mutex_);
        if (failedJobs_.empty()) {
            return;
        }
        unhandledObjects.swap(failedJobs_);
    }
    ListUnhandledObjectsImpl<EtsJob>(vm_->GetClassLinker(), executionCtx, unhandledObjects, jobHandlerMode_);
}

void UnhandledObjectManager::AddRejectedPromise(EtsPromise *promise, EtsExecutionContext *adderExecutionCtx)
{
    os::memory::LockHolder lh(mutex_);
    auto workerId = JobExecutionContext::CastFromMutator(adderExecutionCtx->GetMT())->GetWorker()->GetId();
    AddObjectImpl(rejectedPromises_[workerId], promise->AsObject());
}

void UnhandledObjectManager::RemoveRejectedPromise(EtsPromise *promise, EtsExecutionContext *removerExecutionCtx)
{
    os::memory::LockHolder lh(mutex_);
    auto workerId = JobExecutionContext::CastFromMutator(removerExecutionCtx->GetMT())->GetWorker()->GetId();
    auto it = rejectedPromises_.find(workerId);
    if (it != rejectedPromises_.end()) {
        it->second.erase(promise->AsObject());
        if (it->second.empty()) {
            rejectedPromises_.erase(it);
        }
    }
}

void UnhandledObjectManager::ListRejectedPromises(EtsExecutionContext *executionCtx, bool listAllObjects)
{
    ASSERT_MANAGED_CODE();
    ASSERT(executionCtx != nullptr);
    PandaUnorderedSet<EtsObject *> unhandledObjects {};
    if (listAllObjects) {
        PandaUnorderedMap<JobWorkerThread::Id, PandaUnorderedSet<EtsObject *>> rejectedPromisesLocal {};
        {
            os::memory::LockHolder lh(mutex_);
            if (rejectedPromises_.empty()) {
                return;
            }
            rejectedPromisesLocal.swap(rejectedPromises_);
        }
        unhandledObjects = FlattenMapOfSets(rejectedPromisesLocal);
    } else {
        auto workerId = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetWorker()->GetId();
        {
            os::memory::LockHolder lh(mutex_);
            auto it = rejectedPromises_.find(workerId);
            if (it == rejectedPromises_.end() || it->second.empty()) {
                return;
            }
            unhandledObjects.swap(it->second);
            rejectedPromises_.erase(it);
        }
    }

    ListUnhandledObjectsImpl<EtsPromise>(vm_->GetClassLinker(), executionCtx, unhandledObjects, promiseHandlerMode_);
}

bool UnhandledObjectManager::HasFailedJobObjects() const
{
    os::memory::LockHolder lh(mutex_);
    return !(failedJobs_.empty());
}

bool UnhandledObjectManager::HasRejectedPromiseObjects(EtsExecutionContext *executionCtx, bool listAllObjects) const
{
    os::memory::LockHolder lh(mutex_);
    if (listAllObjects) {
        return !(rejectedPromises_.empty());
    }
    auto workerId = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetWorker()->GetId();
    auto it = rejectedPromises_.find(workerId);
    if (it != rejectedPromises_.end()) {
        return !it->second.empty();
    }
    return false;
}

static void InvokeErrorHandlerImpl(EtsClassLinker *etsClassLinker, EtsExecutionContext *executionCtx,
                                   EtsHandle<EtsObject> &exception)
{
    ASSERT(executionCtx != nullptr);
    auto *exceptionClass = exception->GetClass();
    auto *platformTypes = etsClassLinker->GetEtsClassLinkerExtension()->GetPlatformTypes();
    if (exceptionClass == platformTypes->coreOutOfMemoryError ||
        exceptionClass == platformTypes->coreStackOverflowError) {
        LOG(ERROR, RUNTIME) << "Unhandled exception: " << exception->GetCoreType()->ClassAddr<Class>()->GetName();
        PROCESS_EXIT(1);
        UNREACHABLE();
    }
    executionCtx->GetMT()->ClearException();
    auto *method = platformTypes->coreStdProcessHandleUncaughtError->GetPandaMethod();
    ASSERT(method != nullptr);
    std::array args = {Value(exception->GetCoreType())};
    LOG(DEBUG, COROUTINES) << "Invoking error handler";
    method->InvokeVoid(executionCtx->GetMT(), args.data());
}

void UnhandledObjectManager::InvokeErrorHandler(EtsExecutionContext *executionCtx, EtsHandle<EtsObject> exception)
{
    ASSERT(executionCtx != nullptr);
    ASSERT_MANAGED_CODE();

    InvokeErrorHandlerImpl(vm_->GetClassLinker(), executionCtx, exception);
}

}  // namespace ark::ets
