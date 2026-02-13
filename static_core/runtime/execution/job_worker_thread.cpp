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

#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/job_execution_context.h"

#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/flattened_string_cache.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace ark {

JobWorkerThread::~JobWorkerThread()
{
    DestroyCallbackPoster();
}

void JobWorkerThread::InitializeManagedStructures(const CreatePluginObjFunc &createEtsObj)
{
    ASSERT_MANAGED_CODE();
    InitWorkerLocalObjects(CreateWorkerLocalObjects(createEtsObj));
    CacheLocalObjectsInExecutionCtx();
}

void JobWorkerThread::OnBeforeWorkerStartup(const CreatePluginObjFunc &createEtsObj)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx->GetWorker() == this);
    if (GetRuntime()->IsInitialized()) {
        ScopedManagedCodeThread msc(executionCtx);
        InitializeManagedStructures(createEtsObj);
        executionCtx->UpdateCachedObjects();
    }
}

void JobWorkerThread::OnStartup()
{
    ASSERT(JobExecutionContext::GetCurrent()->GetWorker() == this);
}

/*static*/
PandaVector<JobWorkerThread::LocalObjectData> JobWorkerThread::CreateWorkerLocalObjects(
    const CreatePluginObjFunc &createEtsObj)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    ASSERT_MANAGED_CODE();
    PandaVector<LocalObjectData> objectRefs {};
    auto *refStorage = executionCtx->GetVM()->GetGlobalObjectStorage();

    auto *flattenedStringCache = FlattenedStringCache::Create(executionCtx->GetVM());
    ASSERT(flattenedStringCache != nullptr);
    auto *flattenedStringCacheRef = refStorage->Add(flattenedStringCache, mem::Reference::ObjectType::GLOBAL);
    ASSERT(flattenedStringCacheRef != nullptr);

    objectRefs.push_back(JobWorkerThread::LocalObjectData {
        JobWorkerThread::DataIdx::FLATTENED_STRING_CACHE, flattenedStringCacheRef,
        [refStorage](void *ref) { refStorage->Remove(static_cast<mem::Reference *>(ref)); }});

    if (createEtsObj != nullptr) {
        createEtsObj(objectRefs);
    }

    return objectRefs;
}

void JobWorkerThread::InitWorkerLocalObjects(PandaVector<LocalObjectData> &&objectRefs)
{
    for (auto &objectData : objectRefs) {
        ASSERT(objectData.objectRef != nullptr);
        switch (objectData.type) {
            case JobWorkerThread::DataIdx::FLATTENED_STRING_CACHE:
                ASSERT((GetLocalStorage().Get<JobWorkerThread::DataIdx::FLATTENED_STRING_CACHE, mem::Reference *>() ==
                        nullptr));
                GetLocalStorage().Set<JobWorkerThread::DataIdx::FLATTENED_STRING_CACHE>(
                    objectData.objectRef, std::move(objectData.finalizer));
                break;
            case JobWorkerThread::DataIdx::DOUBLE_TO_STRING_CACHE:
                if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
                    ASSERT(
                        (GetLocalStorage().Get<JobWorkerThread::DataIdx::DOUBLE_TO_STRING_CACHE, mem::Reference *>() ==
                         nullptr));
                    GetLocalStorage().Set<JobWorkerThread::DataIdx::DOUBLE_TO_STRING_CACHE>(
                        objectData.objectRef, std::move(objectData.finalizer));
                }
                break;
            case JobWorkerThread::DataIdx::FLOAT_TO_STRING_CACHE:
                if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
                    ASSERT(
                        (GetLocalStorage().Get<JobWorkerThread::DataIdx::FLOAT_TO_STRING_CACHE, mem::Reference *>() ==
                         nullptr));
                    GetLocalStorage().Set<JobWorkerThread::DataIdx::FLOAT_TO_STRING_CACHE>(
                        objectData.objectRef, std::move(objectData.finalizer));
                }
                break;
            case JobWorkerThread::DataIdx::LONG_TO_STRING_CACHE:
                if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
                    ASSERT((GetLocalStorage().Get<JobWorkerThread::DataIdx::LONG_TO_STRING_CACHE, mem::Reference *>() ==
                            nullptr));
                    GetLocalStorage().Set<JobWorkerThread::DataIdx::LONG_TO_STRING_CACHE>(
                        objectData.objectRef, std::move(objectData.finalizer));
                }
                break;
            default:
                UNREACHABLE();
        }
    }
}

}  // namespace ark
