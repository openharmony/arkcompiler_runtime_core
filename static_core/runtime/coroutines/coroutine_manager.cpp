/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "runtime/coroutines/coroutine_manager.h"

namespace ark {

CoroutineManager::CoroutineManager(CoroutineFactory factory) : coFactory_(factory)
{
    ASSERT(factory != nullptr);
}

Coroutine *CoroutineManager::CreateMainCoroutine(Runtime *runtime, PandaVM *vm)
{
    CoroutineContext *ctx = CreateCoroutineContext(false);
    auto *main = coFactory_(runtime, vm, "_main_", ctx, std::nullopt);
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
                                                           PandaString name)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    CoroutineContext *ctx = CreateCoroutineContext(false);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    auto *co = coFactory_(runtime, vm, std::move(name), ctx, std::nullopt);
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
    ASSERT(!co->HasManagedEntrypoint() && !co->HasNativeEntrypoint());

    auto *context = co->GetContext<CoroutineContext>();
    co->Destroy();
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(co);
    DeleteCoroutineContext(context);
}

Coroutine *CoroutineManager::CreateCoroutineInstance(CompletionEvent *completionEvent, Method *entrypoint,
                                                     PandaVector<Value> &&arguments, PandaString name)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    CoroutineContext *ctx = CreateCoroutineContext(true);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    // assuming that the main coro is already created and Coroutine::GetCurrent is not nullptr
    ASSERT(Coroutine::GetCurrent() != nullptr);
    auto *coro = coFactory_(Runtime::GetCurrent(), Coroutine::GetCurrent()->GetVM(), std::move(name), ctx,
                            Coroutine::ManagedEntrypointInfo {completionEvent, entrypoint, std::move(arguments)});
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

uint32_t CoroutineManager::AllocateCoroutineId()
{
    // Taken by copy-paste from MTThreadManager. Need to generalize if possible.
    // NOTE(konstanting, #I67QXC): try to generalize internal ID allocation
    os::memory::LockHolder lock(idsLock_);
    for (size_t i = 0; i < coroutineIds_.size(); i++) {
        lastCoroutineId_ = (lastCoroutineId_ + 1) % coroutineIds_.size();
        if (!coroutineIds_[lastCoroutineId_]) {
            coroutineIds_.set(lastCoroutineId_);
            return lastCoroutineId_ + 1;  // 0 is reserved as uninitialized value.
        }
    }
    LOG(FATAL, COROUTINES) << "Out of coroutine ids";
    UNREACHABLE();
}

void CoroutineManager::FreeCoroutineId(uint32_t id)
{
    // Taken by copy-paste from MTThreadManager. Need to generalize if possible.
    // NOTE(konstanting, #I67QXC): try to generalize internal ID allocation
    id--;  // 0 is reserved as uninitialized value.
    os::memory::LockHolder lock(idsLock_);
    ASSERT(coroutineIds_[id]);
    coroutineIds_.reset(id);
}

void CoroutineManager::SetSchedulingPolicy(CoroutineSchedulingPolicy policy)
{
    schedulingPolicy_ = policy;
}

CoroutineSchedulingPolicy CoroutineManager::GetSchedulingPolicy() const
{
    return schedulingPolicy_;
}

}  // namespace ark
