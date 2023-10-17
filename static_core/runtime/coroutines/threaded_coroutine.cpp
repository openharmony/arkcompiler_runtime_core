/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "runtime/coroutines/coroutine_context.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/threaded_coroutine.h"

namespace panda {

os::thread::NativeHandleType ThreadedCoroutineContext::GetCoroutineNativeHandle()
{
    return native_handle_;
}

void ThreadedCoroutineContext::SetCoroutineNativeHandle(os::thread::NativeHandleType h)
{
    native_handle_ = h;
}

Coroutine::Status ThreadedCoroutineContext::GetStatus() const
{
    return status_;
}

void ThreadedCoroutineContext::SetStatus(Coroutine::Status new_status)
{
#ifndef NDEBUG
    PandaString setter = (Thread::GetCurrent() == nullptr) ? "null" : Coroutine::GetCurrent()->GetName();
    LOG(DEBUG, COROUTINES) << GetCoroutine()->GetName() << ": " << status_ << " -> " << new_status << " by " << setter;
#endif
    status_ = new_status;
}

void ThreadedCoroutineContext::AttachToCoroutine(Coroutine *co)
{
    CoroutineContext::AttachToCoroutine(co);

    auto *thread_manager = static_cast<CoroutineManager *>(co->GetVM()->GetThreadManager());
    thread_manager->RegisterCoroutine(co);
    if (co->HasManagedEntrypoint()) {
        std::thread t(ThreadProc, this);
        SetCoroutineNativeHandle(t.native_handle());
        t.detach();
    } else {
        // coro has no EP, will execute in the current thread context
        SetCoroutineNativeHandle(os::thread::GetNativeHandle());
    }
}

void ThreadedCoroutineContext::Destroy()
{
    auto *co = GetCoroutine();
    if (co->HasManagedEntrypoint()) {
        // coroutines with an entry point should not be destroyed manually!
        UNREACHABLE();
    }
    ASSERT(co == Coroutine::GetCurrent());
    ASSERT(co->GetStatus() != ThreadStatus::FINISHED);

    co->UpdateStatus(ThreadStatus::TERMINATING);

    auto *thread_manager = static_cast<CoroutineManager *>(co->GetVM()->GetThreadManager());
    if (thread_manager->TerminateCoroutine(co)) {
        // detach
        Coroutine::SetCurrent(nullptr);
    }
}

bool ThreadedCoroutineContext::RetrieveStackInfo(void *&stack_addr, size_t &stack_size, size_t &guard_size)
{
    int error = os::thread::ThreadGetStackInfo(GetCoroutineNativeHandle(), &stack_addr, &stack_size, &guard_size);
    if (error != 0) {
        LOG(ERROR, RUNTIME) << "ThreadedCoroutineContext::RetrieveStackInfo: fail to get stack info, error = "
                            << strerror(errno);
    }
    return error == 0;
}

/*static*/ void ThreadedCoroutineContext::ThreadProc(ThreadedCoroutineContext *ctx)
{
    auto *co = ctx->GetCoroutine();
    Coroutine::SetCurrent(co);
    os::thread::SetThreadName(os::thread::GetNativeHandle(), co->GetName().c_str());
    // TODO(konstanting, #I67QXC): find a workaround to access this method
    // co->UpdateId(os::thread::GetCurrentThreadId());
    co->NativeCodeBegin();
    {
        ctx->InitializationDone();
        if (co->IsSuspendOnStartup()) {
            ctx->WaitUntilResumed();
        } else {
            ctx->SetStatus(Coroutine::Status::RUNNING);
        }

        ScopedManagedCodeThread s(co);
        PandaVector<Value> args = std::move(co->GetManagedEntrypointArguments());
        LOG(DEBUG, COROUTINES) << "ThreadProc: invoking the EP for coro " << co->GetName();
        Value res = co->GetManagedEntrypoint()->Invoke(co, args.data());
        LOG(DEBUG, COROUTINES) << "ThreadProc: invoke() finished for the EP of coro " << co->GetName();
        co->RequestCompletion(res);
    }
    ctx->SetStatus(Coroutine::Status::TERMINATING);

    auto *thread_manager = static_cast<CoroutineManager *>(co->GetVM()->GetThreadManager());
    if (thread_manager->TerminateCoroutine(co)) {
        Coroutine::SetCurrent(nullptr);
    }
}

void ThreadedCoroutineContext::WaitUntilInitialized()
{
    os::memory::LockHolder l(cv_mutex_);
    while (GetStatus() != Coroutine::Status::RUNNABLE) {
        cv_.Wait(&cv_mutex_);
    }
}

void ThreadedCoroutineContext::WaitUntilResumed()
{
    os::memory::LockHolder l(cv_mutex_);
    while (GetStatus() != Coroutine::Status::RUNNING) {
        cv_.Wait(&cv_mutex_);
    }
}

void ThreadedCoroutineContext::InitializationDone()
{
    if (GetCoroutine()->IsSuspendOnStartup()) {
        os::memory::LockHolder l(cv_mutex_);
        SetStatus(Coroutine::Status::RUNNABLE);
        cv_.Signal();
    } else {
        SetStatus(Coroutine::Status::RUNNABLE);
    }
}

void ThreadedCoroutineContext::RequestSuspend(bool gets_blocked)
{
    os::memory::LockHolder l(cv_mutex_);
    SetStatus(gets_blocked ? Coroutine::Status::BLOCKED : Coroutine::Status::RUNNABLE);
}

void ThreadedCoroutineContext::RequestResume()
{
    os::memory::LockHolder l(cv_mutex_);
    SetStatus(Coroutine::Status::RUNNING);
    cv_.Signal();
}

void ThreadedCoroutineContext::RequestUnblock()
{
    SetStatus(Coroutine::Status::RUNNABLE);
}

void ThreadedCoroutineContext::MainThreadFinished()
{
    SetStatus(Coroutine::Status::TERMINATING);
}

void ThreadedCoroutineContext::EnterAwaitLoop()
{
    SetStatus(Coroutine::Status::AWAIT_LOOP);
}

}  // namespace panda
