/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "runtime/include/method.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {

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
    UpdateObjectsImpl(rejectedPromises_, gcRootUpdater);
}

static void VisitObjectsImpl(PandaUnorderedSet<EtsObject *> &objects, const GCRootVisitor &visitor)
{
    for (auto *obj : objects) {
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, obj->GetCoreType()));
    }
}

void UnhandledObjectManager::VisitObjects(const GCRootVisitor &visitor)
{
    os::memory::LockHolder lh(mutex_);
    VisitObjectsImpl(failedJobs_, visitor);
    VisitObjectsImpl(rejectedPromises_, visitor);
}

static void AddObjectImpl(PandaUnorderedSet<EtsObject *> &objects, EtsObject *object)
{
    objects.insert(object);
}

static void RemoveObjectImpl(PandaUnorderedSet<EtsObject *> &objects, EtsObject *object)
{
    objects.erase(object);
}

static PandaVector<EtsHandle<EtsObject>> TransformToVectorOfHandles(EtsCoroutine *coro,
                                                                    const PandaUnorderedSet<EtsObject *> &objects)
{
    ASSERT(coro != nullptr);
    PandaVector<EtsHandle<EtsObject>> handleVec;
    handleVec.reserve(objects.size());
    for (auto *obj : objects) {
        ASSERT(!obj->GetCoreType()->IsForwarded());
        handleVec.emplace_back(coro, obj);
    }
    return handleVec;
}

template <typename T>
static EtsHandle<EtsEscompatArray> CreateEtsObjectArrayFromHandles(EtsCoroutine *coro,
                                                                   const PandaVector<EtsHandle<EtsObject>> &handles)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    ASSERT(coro != nullptr);
    auto *array = EtsEscompatArray::Create(handles.size());
    ASSERT(array != nullptr);
    EtsHandle<EtsEscompatArray> harray(coro, array);
    size_t i = 0;
    for (auto hobj : handles) {
        ASSERT(hobj.GetPtr() != nullptr);
        ASSERT(!hobj->GetCoreType()->IsForwarded());

        auto *objAndReason = EtsEscompatArray::Create(2U);
        ASSERT(objAndReason != nullptr);
        objAndReason->SetRef(0, T::FromEtsObject(hobj.GetPtr())->GetValue(coro));
        objAndReason->SetRef(1, hobj.GetPtr());

        harray->SetRef(i, objAndReason);
        ++i;
    }
    return harray;
}

template <typename T>
static void ListObjectsFromEtsArray(EtsClassLinker *etsClassLinker, EtsCoroutine *coro,
                                    EtsHandle<EtsEscompatArray> &hobjects)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    auto *platformTypes = etsClassLinker->GetEtsClassLinkerExtension()->GetPlatformTypes();
    Method *method {};
    if constexpr (std::is_same_v<T, EtsJob>) {
        method = platformTypes->escompatProcessListUnhandledJobs->GetPandaMethod();
        LOG(DEBUG, COROUTINES) << "List unhandled failed jobs";
    } else if (std::is_same_v<T, EtsPromise>) {
        method = platformTypes->escompatProcessListUnhandledPromises->GetPandaMethod();
        LOG(DEBUG, COROUTINES) << "List unhandled rejected promises";
    }
    ASSERT(method != nullptr);
    std::array args = {Value(hobjects->GetCoreType())};
    method->InvokeVoid(coro, args.data());
    LOG(DEBUG, COROUTINES) << "List unhandled rejections end";
}

template <typename T>
static void ListUnhandledObjectsImpl(EtsClassLinker *etsClassLinker, EtsCoroutine *coro,
                                     const PandaUnorderedSet<EtsObject *> &objects)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    ASSERT(coro != nullptr);
    ASSERT(etsClassLinker != nullptr);
    {
        [[maybe_unused]] ScopedManagedCodeThread s(coro);
        [[maybe_unused]] EtsHandleScope scope(coro);

        auto handles = TransformToVectorOfHandles(coro, objects);
        auto hEtsArray = CreateEtsObjectArrayFromHandles<T>(coro, handles);
        ListObjectsFromEtsArray<T>(etsClassLinker, coro, hEtsArray);
    }
    if (coro->HasPendingException()) {
        coro->HandleUncaughtException();
        UNREACHABLE();
    }
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

void UnhandledObjectManager::ListFailedJobs(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    PandaUnorderedSet<EtsObject *> unhandledObjects {};
    {
        os::memory::LockHolder lh(mutex_);
        if (failedJobs_.empty()) {
            return;
        }
        unhandledObjects.swap(failedJobs_);
    }
    ListUnhandledObjectsImpl<EtsJob>(vm_->GetClassLinker(), coro, unhandledObjects);
}

void UnhandledObjectManager::AddRejectedPromise(EtsPromise *promise)
{
    os::memory::LockHolder lh(mutex_);
    AddObjectImpl(rejectedPromises_, promise->AsObject());
}

void UnhandledObjectManager::RemoveRejectedPromise(EtsPromise *promise)
{
    os::memory::LockHolder lh(mutex_);
    RemoveObjectImpl(rejectedPromises_, promise->AsObject());
}

void UnhandledObjectManager::ListRejectedPromises(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    PandaUnorderedSet<EtsObject *> unhandledObjects {};
    {
        os::memory::LockHolder lh(mutex_);
        if (rejectedPromises_.empty()) {
            return;
        }
        unhandledObjects.swap(rejectedPromises_);
    }
    ListUnhandledObjectsImpl<EtsPromise>(vm_->GetClassLinker(), coro, unhandledObjects);
}

bool UnhandledObjectManager::HasObjects() const
{
    os::memory::LockHolder lh(mutex_);
    return !(rejectedPromises_.empty() && failedJobs_.empty());
}

}  // namespace ark::ets
