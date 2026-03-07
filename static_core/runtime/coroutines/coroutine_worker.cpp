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

#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine_worker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/flattened_string_cache.h"
#include "runtime/include/thread-inl.h"
#include "mem/refstorage/global_object_storage.h"

namespace ark {

void CoroutineWorker::TriggerSchedulerExternally(Coroutine *requester, int64_t delayMs)
{
    if (requester->GetType() == Coroutine::Type::MUTATOR) {
        os::memory::LockHolder l(posterLock_);
        if (extSchedulingPoster_ != nullptr) {
            extSchedulingPoster_->Post(delayMs);
        }
    }
}

void CoroutineWorker::OnCoroBecameActive(Coroutine *co)
{
    TriggerSchedulerExternally(co);
}

void CoroutineWorker::DestroyCallbackPoster()
{
    os::memory::LockHolder l(posterLock_);
    extSchedulingPoster_.reset(nullptr);
}

void CoroutineWorker::InitializeManagedStructures()
{
    ASSERT_MANAGED_CODE();
    InitWorkerLocalObjects(CreateWorkerLocalObjects());
    CacheLocalObjectsInCoroutines();
}

void CoroutineWorker::OnBeforeWorkerStartup()
{
    auto *currCoro = Coroutine::GetCurrent();
    ASSERT(currCoro->GetWorker() == this);
    if (GetRuntime()->IsInitialized()) {
        ScopedManagedCodeThread msc(currCoro);
        InitializeManagedStructures();
        currCoro->UpdateCachedObjects();
    }
}

void CoroutineWorker::OnWorkerStartup()
{
    ASSERT(Coroutine::GetCurrent()->GetWorker() == this);
}

/*static*/
PandaVector<CoroutineWorker::LocalObjectData> CoroutineWorker::CreateWorkerLocalObjects()
{
    auto *coro = Coroutine::GetCurrent();
    ASSERT(coro != nullptr);
    ASSERT_MANAGED_CODE();
    PandaVector<LocalObjectData> objectRefs {};
    auto *refStorage = coro->GetVM()->GetGlobalObjectStorage();

    auto *flattenedStringCache = FlattenedStringCache::Create(coro->GetVM());
    ASSERT(flattenedStringCache != nullptr);
    auto *flattenedStringCacheRef = refStorage->Add(flattenedStringCache, mem::Reference::ObjectType::GLOBAL);
    ASSERT(flattenedStringCacheRef != nullptr);

    objectRefs.push_back(CoroutineWorker::LocalObjectData {
        CoroutineWorker::DataIdx::FLATTENED_STRING_CACHE, flattenedStringCacheRef,
        [refStorage](void *ref) { refStorage->Remove(static_cast<mem::Reference *>(ref)); }});

    return objectRefs;
}

void CoroutineWorker::InitWorkerLocalObjects(PandaVector<LocalObjectData> &&objectRefs)
{
    for (auto &objectData : objectRefs) {
        ASSERT(objectData.objectRef != nullptr);
        switch (objectData.type) {
            case CoroutineWorker::DataIdx::FLATTENED_STRING_CACHE:
                ASSERT((GetLocalStorage().Get<CoroutineWorker::DataIdx::FLATTENED_STRING_CACHE, mem::Reference *>() ==
                        nullptr));
                GetLocalStorage().Set<CoroutineWorker::DataIdx::FLATTENED_STRING_CACHE>(
                    objectData.objectRef, std::move(objectData.finalizer));
                break;
            default:
                UNREACHABLE();
        }
    }
}

}  // namespace ark
