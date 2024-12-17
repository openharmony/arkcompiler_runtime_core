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

#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_context.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/panda_vm.h"

namespace ark {

Coroutine *Coroutine::Create(Runtime *runtime, PandaVM *vm, PandaString name, CoroutineContext *context,
                             std::optional<EntrypointInfo> &&epInfo, Type type)
{
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    auto *co = allocator->New<Coroutine>(os::thread::GetCurrentThreadId(), allocator, vm,
                                         ark::panda_file::SourceLang::PANDA_ASSEMBLY, std::move(name), context,
                                         std::move(epInfo), type);
    co->Initialize();
    return co;
}

Coroutine::Coroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                     ark::panda_file::SourceLang threadLang, PandaString name, CoroutineContext *context,
                     std::optional<EntrypointInfo> &&epInfo, Type type)
    : ManagedThread(id, allocator, vm, Thread::ThreadType::THREAD_TYPE_TASK, threadLang),
      name_(std::move(name)),
      context_(context),
      manager_(static_cast<CoroutineManager *>(GetVM()->GetThreadManager())),
      startSuspended_(epInfo.has_value()),
      type_(type)
{
    ASSERT(vm != nullptr);
    ASSERT(context != nullptr);
    ASSERT(manager_ != nullptr);
    SetEntrypointData(std::move(epInfo));
    coroutineId_ = GetManager()->AllocateCoroutineId();
}

Coroutine::~Coroutine()
{
    GetManager()->FreeCoroutineId(coroutineId_);
}

void Coroutine::ReInitialize(PandaString name, CoroutineContext *context, std::optional<EntrypointInfo> &&epInfo)
{
    ASSERT(context != nullptr);
    name_ = std::move(name);
    startSuspended_ = epInfo.has_value();
    context_ = context;

    SetEntrypointData(std::move(epInfo));
    context_->AttachToCoroutine(this);
}

void Coroutine::SetEntrypointData(std::optional<EntrypointInfo> &&epInfo)
{
    if (epInfo.has_value()) {
        auto &info = epInfo.value();
        if (std::holds_alternative<ManagedEntrypointInfo>(info)) {
            auto &managedEp = std::get<ManagedEntrypointInfo>(info);
            entrypoint_.emplace<ManagedEntrypointData>(managedEp.completionEvent, managedEp.entrypoint,
                                                       std::move(managedEp.arguments));
        } else if (std::holds_alternative<NativeEntrypointInfo>(info)) {
            auto &nativeEp = std::get<NativeEntrypointInfo>(info);
            entrypoint_ = NativeEntrypointData(nativeEp.entrypoint, nativeEp.param);
        }
    }
}

void Coroutine::CleanUp()
{
    ManagedThread::CleanUp();
    name_ = "";
    entrypoint_ = std::monostate();
    startSuspended_ = false;
    worker_ = nullptr;
    context_->CleanUp();
}

Coroutine::ManagedEntrypointData::~ManagedEntrypointData()
{
    // delete the event as it is owned by the ManagedEntrypointData instance
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(completionEvent);
}

PandaString Coroutine::GetName() const
{
    return name_;
}

Coroutine::Status Coroutine::GetCoroutineStatus() const
{
    return context_->GetStatus();
}

void Coroutine::SetCoroutineStatus(Coroutine::Status newStatus)
{
    context_->SetStatus(newStatus);
}

void Coroutine::OnStatusChanged(Status oldStatus, Status newStatus)
{
    // issue required status change events to the CoroutineManager
    bool hasWorker = (GetWorker() != nullptr);
    bool wasActive = hasWorker && (oldStatus == Coroutine::Status::RUNNABLE || oldStatus == Coroutine::Status::RUNNING);
    bool isActive = hasWorker && (newStatus == Coroutine::Status::RUNNABLE || newStatus == Coroutine::Status::RUNNING);
    if (UNLIKELY(wasActive != isActive)) {
        if (wasActive && !isActive) {
            GetManager()->OnCoroBecameNonActive(this);
        } else if (!wasActive && isActive) {
            GetManager()->OnCoroBecameActive(this);
        }
    }
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

bool Coroutine::RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize)
{
    if (HasManagedEntrypoint() || HasNativeEntrypoint()) {
        // has EP and separate native context for its execution
        return context_->RetrieveStackInfo(stackAddr, stackSize, guardSize);
    }
    // does not have EP, executes on OS-provided context and stack
    return ManagedThread::RetrieveStackInfo(stackAddr, stackSize, guardSize);
}

void Coroutine::RequestSuspend(bool getsBlocked)
{
    context_->RequestSuspend(getsBlocked);
}

void Coroutine::RequestResume()
{
    context_->RequestResume();
}

void Coroutine::RequestUnblock()
{
    context_->RequestUnblock();
}

void Coroutine::RequestCompletion([[maybe_unused]] Value returnValue)
{
    auto *e = GetCompletionEvent();
    e->Happen();
}

bool Coroutine::IsActive()
{
    bool workerAssigned = (GetWorker() != nullptr);
    auto status = GetCoroutineStatus();
    bool isRunnableOrRunning = (status == Status::RUNNABLE) || (status == Status::RUNNING);
    return (workerAssigned && isRunnableOrRunning);
}

void Coroutine::SetWorker(CoroutineWorker *w)
{
    auto *oldWorker = worker_;
    worker_ = w;
    if (w != oldWorker) {
        OnHostWorkerChanged();
    }
    // being ACTIVE requires an assigned worker!
    if ((oldWorker == nullptr) && (w != nullptr)) {
        GetManager()->OnCoroBecameActive(this);
    } else if ((oldWorker != nullptr) && (w == nullptr)) {
        GetManager()->OnCoroBecameNonActive(this);
    }
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

}  // namespace ark
