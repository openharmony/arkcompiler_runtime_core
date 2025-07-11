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
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"

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

template <typename T>
static EtsEscompatArray *CreateEtsObjectArrayFromNativeSet(EtsCoroutine *coro,
                                                           const PandaUnorderedSet<EtsObject *> &objects)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    const auto objCount = objects.size();
    auto *array = EtsEscompatArray::Create(objCount);
    ASSERT(array != nullptr);
    size_t i = 0;
    for (auto *obj : objects) {
        ASSERT(obj != nullptr);
        ASSERT(!obj->GetCoreType()->IsForwarded());
        array->SetRef(i, T::FromEtsObject(obj)->GetValue(coro));
        ++i;
    }
    return array;
}

template <typename T>
static void ListObjectsImpl(EtsClassLinker *etsClassLinker, napi::ScopedManagedCodeFix *s, EtsEscompatArray *errors)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    auto *coro = s->Coroutine();
    EtsHandle<EtsEscompatArray> herrors(coro, errors);
    auto *platformTypes = etsClassLinker->GetEtsClassLinkerExtension()->GetPlatformTypes();
    EtsMethod *method {};
    if constexpr (std::is_same_v<T, EtsJob>) {
        method = platformTypes->escompatProcessListUnhandledJobs;
        LOG(DEBUG, COROUTINES) << "List unhandled failed jobs";
    } else if (std::is_same_v<T, EtsPromise>) {
        method = platformTypes->escompatProcessListUnhandledPromises;
        LOG(DEBUG, COROUTINES) << "List unhandled rejected promises";
    }
    ASSERT(method != nullptr);
    std::array args = {Value(herrors->GetCoreType())};
    method->InvokeVoid(s, args.data());
    LOG(DEBUG, COROUTINES) << "List unhandled rejections end";
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
    os::memory::LockHolder lh(mutex_);
    if (failedJobs_.empty()) {
        return;
    }
    {
        [[maybe_unused]] napi::ScopedManagedCodeFix s(coro->GetEtsNapiEnv());
        [[maybe_unused]] EtsHandleScope scope(coro);

        auto *errors = CreateEtsObjectArrayFromNativeSet<EtsJob>(coro, failedJobs_);
        ListObjectsImpl<EtsJob>(vm_->GetClassLinker(), &s, errors);
        failedJobs_.clear();
    }
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
    os::memory::LockHolder lh(mutex_);
    if (rejectedPromises_.empty()) {
        return;
    }
    {
        [[maybe_unused]] napi::ScopedManagedCodeFix s(coro->GetEtsNapiEnv());
        [[maybe_unused]] EtsHandleScope scope(coro);

        auto *errors = CreateEtsObjectArrayFromNativeSet<EtsPromise>(coro, rejectedPromises_);
        ListObjectsImpl<EtsPromise>(vm_->GetClassLinker(), &s, errors);
        rejectedPromises_.clear();
    }
}

}  // namespace ark::ets
