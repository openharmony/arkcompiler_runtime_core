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

#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

void FinalizationRegistryManager::SortInstancies()
{
    auto *objArray = EtsObjectArray::FromCoreType(vm_->GetGlobalObjectStorage()->Get(finRegInstancies_));
    ASSERT(objArray != nullptr);
    size_t head = 0;
    size_t localIndex = objArray->GetLength();
    size_t tail = localIndex - 1;
    while (head < tail) {
        while (head < tail && objArray->Get(head) != nullptr) {
            head++;
        }
        while (head < tail && objArray->Get(tail) == nullptr) {
            tail--;
        }
        if (head < tail) {
            objArray->Set(head, objArray->Get(tail));
            objArray->Set(tail, nullptr);
            localIndex = tail;
        }
    }
    finRegLastIndex_ = localIndex;
}

void FinalizationRegistryManager::EnsureCapacity(EtsCoroutine *coro)
{
    EtsClass *objectClass = vm_->GetClassLinker()->GetClassRoot(EtsClassRoot::OBJECT);
    if (finRegInstancies_ == nullptr) {
        auto *objArray = EtsObjectArray::Create(objectClass, INIT_SIZE, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
        ASSERT(objArray != nullptr);
        finRegInstancies_ =
            vm_->GetGlobalObjectStorage()->Add(objArray->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
        finRegLastIndex_ = 0;
    }
    auto *objArray = EtsObjectArray::FromCoreType(vm_->GetGlobalObjectStorage()->Get(finRegInstancies_));
    EtsHandle<EtsObjectArray> objArrayHandle(coro, objArray);
    ASSERT(objArrayHandle.GetPtr() != nullptr);
    size_t finRegCapacity = objArrayHandle->GetLength();
    if (finRegLastIndex_ >= finRegCapacity) {
        SortInstancies();
        if (finRegLastIndex_ >= finRegCapacity) {
            auto *newFinalizationRegistryInstances = EtsObjectArray::Create(objectClass, (finRegCapacity * 2U) + 1U,
                                                                            SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
            objArrayHandle->CopyDataTo(newFinalizationRegistryInstances);
            vm_->GetGlobalObjectStorage()->Remove(finRegInstancies_);
            finRegInstancies_ = vm_->GetGlobalObjectStorage()->Add(newFinalizationRegistryInstances->GetCoreType(),
                                                                   ark::mem::Reference::ObjectType::GLOBAL);
        }
    }
}

void FinalizationRegistryManager::RegisterInstance(EtsObject *instance)
{
    ASSERT_MANAGED_CODE();
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> instanceHandle(coroutine, instance);
    EnsureCapacity(coroutine);
    auto *objArray = EtsObjectArray::FromCoreType(vm_->GetGlobalObjectStorage()->Get(finRegInstancies_));
    ASSERT(objArray != nullptr);
    ASSERT(objArray->GetLength() != 0);
    objArray->Set(finRegLastIndex_, instanceHandle.GetPtr());
    finRegLastIndex_++;
}

void FinalizationRegistryManager::CleanupCoroFinished()
{
    // Atomic with acq_rel order reason: other threads should see correct value
    [[maybe_unused]] uint32_t oldCnt = finRegCleanupCoroCount_.fetch_sub(1, std::memory_order_acq_rel);
    ASSERT(oldCnt > 0);
}

bool FinalizationRegistryManager::UpdateFinRegCoroCountAndCheckIfCleanupNeeded()
{
    // Atomic with acquire order reason: getting correct value
    uint32_t cnt = finRegCleanupCoroCount_.load(std::memory_order_acquire);
    uint32_t oldCnt = cnt;
    // Atomic with acq_rel order reason: sync for counter
    while (cnt < MAX_FINREG_CLEANUP_COROS && !finRegCleanupCoroCount_.compare_exchange_weak(
                                                 // CC-OFFNXT(G.FMT.06-CPP) project code style
                                                 cnt, cnt + 1U, std::memory_order_acq_rel, std::memory_order_acquire)) {
        oldCnt = cnt;
    }
    return oldCnt < MAX_FINREG_CLEANUP_COROS;
}

void FinalizationRegistryManager::StartCleanupCoroIfNeeded(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    auto *coroManager = coro->GetCoroutineManager();

    if (finRegLastIndex_ != 0 && UpdateFinRegCoroCountAndCheckIfCleanupNeeded()) {
        auto *objArray = EtsObjectArray::FromCoreType(vm_->GetGlobalObjectStorage()->Get(finRegInstancies_));
        auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, coroManager);
        Method *cleanup = PlatformTypes(vm_)->coreFinalizationRegistryExecCleanup->GetPandaMethod();
        auto launchMode =
            coroManager->IsMainWorker(coro) ? CoroutineLaunchMode::MAIN_WORKER : CoroutineLaunchMode::DEFAULT;
        auto args = PandaVector<Value> {Value(objArray->GetCoreType()), Value(static_cast<uint32_t>(launchMode))};
        [[maybe_unused]] bool launchResult = coroManager->Launch(event, cleanup, std::move(args), launchMode,
                                                                 CoroutinePriority::DEFAULT_PRIORITY, false);
        ASSERT(launchResult);
    }
}

}  // namespace ark::ets
