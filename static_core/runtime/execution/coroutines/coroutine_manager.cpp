/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <optional>
#include <variant>
#include "runtime/execution/affinity_mask.h"
#include "runtime/execution/coroutines/coroutine_manager.h"

namespace ark {

CoroutineManager::CoroutineManager(const CoroutineManagerConfig &config, CoroutineFactory factory)
    : coFactory_(factory), config_(config)
{
    ASSERT(factory != nullptr);
}

Coroutine *CoroutineManager::CreateMainCoroutine(Runtime *runtime, PandaVM *vm)
{
    CoroutineContext *ctx = CreateCoroutineContext(false);
    auto *job = CreateJob("_main_", std::monostate());
    ASSERT(job != nullptr);
    auto *main = coFactory_(runtime, vm, job, ctx);
    ASSERT(main != nullptr);

    Coroutine::SetCurrent(main);
    main->InitBuffers();
    main->RequestResume();
    main->NativeCodeBegin();

    SetMainThread(main);
    return main;
}

void CoroutineManager::DestroyMainCoroutine()
{
    auto *main = static_cast<Coroutine *>(GetMainThread());
    auto *context = main->GetContext<CoroutineContext>();
    main->Destroy();
    Coroutine::SetCurrent(nullptr);
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(main);
    DeleteCoroutineContext(context);
}

Coroutine *CoroutineManager::CreateEntrypointlessCoroutine(Runtime *runtime, PandaVM *vm, bool makeCurrent,
                                                           PandaString name, Coroutine::Type type,
                                                           CoroutinePriority priority)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    // In current design user (not system) coroutine is EP-ful mutator
    ASSERT(CoroutineManager::IsSystemCoroutine(type, false));
    CoroutineContext *ctx = CreateCoroutineContext(false);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    auto *job = CreateJob(std::move(name), std::monostate(), priority, type);
    ASSERT(job != nullptr);
    auto *co = coFactory_(runtime, vm, job, ctx);
    ASSERT(co != nullptr);
    co->InitBuffers();
    if (makeCurrent) {
        Coroutine::SetCurrent(co);
        co->RequestResume();
        co->NativeCodeBegin();
    }
    return co;
}

void CoroutineManager::DestroyEntrypointlessCoroutine(Coroutine *co)
{
    ASSERT(co != nullptr);
    ASSERT(!co->GetJob()->HasManagedEntrypoint() && !co->GetJob()->HasNativeEntrypoint());

    auto *context = co->GetContext<CoroutineContext>();
    co->Destroy();
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(co);
    DeleteCoroutineContext(context);
}

Coroutine *CoroutineManager::CreateCoroutineInstance(Job *job)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    if (!IsSystemCoroutine(job->GetType(), true) && IsUserCoroutineLimitReached()) {
        // user coro limit reached
        return nullptr;
    }
    CoroutineContext *ctx = CreateCoroutineContext(true);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    // assuming that the main coro is already created and Coroutine::GetCurrent is not nullptr
    ASSERT(Coroutine::GetCurrent() != nullptr);
    auto *coro = coFactory_(Runtime::GetCurrent(), Coroutine::GetCurrent()->GetVM(), job, ctx);
    ASSERT(coro != nullptr);
    return coro;
}

void CoroutineManager::DestroyEntrypointfulCoroutine(Coroutine *co)
{
    DeleteCoroutineContext(co->GetContext<CoroutineContext>());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(co);
}

CoroutineManager::CoroutineFactory CoroutineManager::GetCoroutineFactory()
{
    return coFactory_;
}

void CoroutineManager::SetSchedulingPolicy(CoroutineSchedulingPolicy policy)
{
    os::memory::LockHolder lock(policyLock_);
    schedulingPolicy_ = policy;
}

CoroutineSchedulingPolicy CoroutineManager::GetSchedulingPolicy() const
{
    os::memory::LockHolder lock(policyLock_);
    return schedulingPolicy_;
}

}  // namespace ark
