/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_context.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/panda_vm.h"

namespace panda {

Coroutine *Coroutine::Create(Runtime *runtime, PandaVM *vm, PandaString name, CoroutineContext *context,
                             std::optional<EntrypointInfo> &&ep_info)
{
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    auto *co = allocator->New<Coroutine>(os::thread::GetCurrentThreadId(), allocator, vm,
                                         panda::panda_file::SourceLang::PANDA_ASSEMBLY, std::move(name), context,
                                         std::move(ep_info));
    co->Initialize();
    return co;
}

Coroutine::Coroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                     panda::panda_file::SourceLang thread_lang, PandaString name, CoroutineContext *context,
                     std::optional<EntrypointInfo> &&ep_info)
    : ManagedThread(id, allocator, vm, Thread::ThreadType::THREAD_TYPE_TASK, thread_lang),
      name_(std::move(name)),
      context_(context),
      start_suspended_(ep_info.has_value())
{
    ASSERT(vm != nullptr);
    ASSERT(context != nullptr);
    SetEntrypointData(std::move(ep_info));
    coroutine_id_ = static_cast<CoroutineManager *>(GetVM()->GetThreadManager())->AllocateCoroutineId();
}

Coroutine::~Coroutine()
{
    static_cast<CoroutineManager *>(GetVM()->GetThreadManager())->FreeCoroutineId(coroutine_id_);
}

void Coroutine::ReInitialize(PandaString name, CoroutineContext *context, std::optional<EntrypointInfo> &&ep_info)
{
    ASSERT(context != nullptr);
    name_ = std::move(name);
    start_suspended_ = ep_info.has_value();
    context_ = context;

    SetEntrypointData(std::move(ep_info));
    context_->AttachToCoroutine(this);
}

void Coroutine::SetEntrypointData(std::optional<EntrypointInfo> &&ep_info)
{
    if (ep_info.has_value()) {
        auto &info = ep_info.value();
        if (std::holds_alternative<ManagedEntrypointInfo>(info)) {
            auto &managed_ep = std::get<ManagedEntrypointInfo>(info);
            entrypoint_.emplace<ManagedEntrypointData>(managed_ep.completion_event, managed_ep.entrypoint,
                                                       std::move(managed_ep.arguments));
        } else if (std::holds_alternative<NativeEntrypointInfo>(info)) {
            auto &native_ep = std::get<NativeEntrypointInfo>(info);
            entrypoint_ = NativeEntrypointData(native_ep.entrypoint, native_ep.param);
        }
    }
}

void Coroutine::CleanUp()
{
    ManagedThread::CleanUp();
    name_ = "";
    entrypoint_ = std::monostate();
    start_suspended_ = false;
    context_->CleanUp();
}

Coroutine::ManagedEntrypointData::~ManagedEntrypointData()
{
    // delete the event as it is owned by the ManagedEntrypointData instance
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(completion_event);
}

PandaString Coroutine::GetName() const
{
    return name_;
}

Coroutine::Status Coroutine::GetCoroutineStatus() const
{
    return context_->GetStatus();
}

void Coroutine::SetCoroutineStatus(Coroutine::Status new_status)
{
    context_->SetStatus(new_status);
}

void Coroutine::Destroy()
{
    context_->Destroy();
}

void Coroutine::Initialize()
{
    context_->AttachToCoroutine(this);
    InitForStackOverflowCheck(ManagedThread::STACK_OVERFLOW_RESERVED_SIZE,
                              ManagedThread::STACK_OVERFLOW_PROTECTED_SIZE);
}

bool Coroutine::RetrieveStackInfo(void *&stack_addr, size_t &stack_size, size_t &guard_size)
{
    if (HasManagedEntrypoint() || HasNativeEntrypoint()) {
        // has EP and separate native context for its execution
        return context_->RetrieveStackInfo(stack_addr, stack_size, guard_size);
    }
    // does not have EP, executes on OS-provided context and stack
    return ManagedThread::RetrieveStackInfo(stack_addr, stack_size, guard_size);
}

void Coroutine::RequestSuspend(bool gets_blocked)
{
    context_->RequestSuspend(gets_blocked);
}

void Coroutine::RequestResume()
{
    context_->RequestResume();
}

void Coroutine::RequestUnblock()
{
    context_->RequestUnblock();
}

void Coroutine::RequestCompletion([[maybe_unused]] Value return_value)
{
    auto *e = GetCompletionEvent();
    e->SetHappened();
}

std::ostream &operator<<(std::ostream &os, Coroutine::Status status)
{
    switch (status) {
        case Coroutine::Status::CREATED:
            os << "CREATED";
            break;
        case Coroutine::Status::RUNNABLE:
            os << "RUNNABLE";
            break;
        case Coroutine::Status::RUNNING:
            os << "RUNNING";
            break;
        case Coroutine::Status::BLOCKED:
            os << "BLOCKED";
            break;
        case Coroutine::Status::TERMINATING:
            os << "TERMINATING";
            break;
        case Coroutine::Status::AWAIT_LOOP:
            os << "AWAIT_LOOP";
            break;
        default:
            break;
    }
    return os;
}

}  // namespace panda
