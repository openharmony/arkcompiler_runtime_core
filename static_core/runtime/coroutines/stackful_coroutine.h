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
#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_H

#include "runtime/fibers/fiber_context.h"
#include "runtime/coroutines/coroutine_context.h"
#include "runtime/include/panda_vm.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"

namespace panda {

/**
 * @brief Native context of a coroutine. "Fiber"-based implementation.
 *
 * This implementation uses panda fibers library to implement native coroutine context.
 */
class StackfulCoroutineContext : public CoroutineContext {
public:
    NO_COPY_SEMANTIC(StackfulCoroutineContext);
    NO_MOVE_SEMANTIC(StackfulCoroutineContext);

    /**
     * @param stack specifies the lowest address of the stack region to use;
     * it should have at least @param stack_size_bytes bytes accessible. If the stack grows down on the
     * target architecture, then the initial stack pointer of the coroutine will be set to
     * (stack + stack_size_bytes)
     */
    explicit StackfulCoroutineContext(uint8_t *stack, size_t stack_size_bytes);
    ~StackfulCoroutineContext() override = default;

    /**
     * Prepares the context for execution, links it to the managed context part (Coroutine instance) and registers the
     * created coroutine in the CoroutineManager (in the RUNNABLE status)
     */
    void AttachToCoroutine(Coroutine *co) override;
    /**
     * Manually destroys the context. Should be called by the Coroutine instance as a part of main coroutine
     * destruction. All other coroutines and their contexts are destroyed by the CoroutineManager once the coroutine
     * entrypoint execution finishes
     */
    void Destroy() override;

    void CleanUp() override;

    bool RetrieveStackInfo(void *&stack_addr, size_t &stack_size, size_t &guard_size) override;

    /**
     * Suspends the execution context, sets its status to either Status::RUNNABLE or Status::BLOCKED, depending on the
     * suspend reason.
     */
    void RequestSuspend(bool gets_blocked) override;
    /// Resumes the suspended context, sets status to RUNNING.
    void RequestResume() override;
    /// Unblock the coroutine and set its status to Status::RUNNABLE
    void RequestUnblock() override;

    // should be called then the main thread is done executing the program entrypoint
    void MainThreadFinished();
    /// Moves the main coroutine to Status::AWAIT_LOOP
    void EnterAwaitLoop();

    /// Coroutine status is a part of native context, because it might require some synchronization on access
    Coroutine::Status GetStatus() const override;

    /**
     * Transfer control to the target context
     * NB: this method will return only after the control is transferred back to the caller context
     */
    bool SwitchTo(StackfulCoroutineContext *target);

    /// @return the lowest address of the coroutine native stack (provided in the ctx contructor)
    uint8_t *GetStackLoAddrPtr() const
    {
        return stack_;
    }

    /// Executes a foreign lambda function within this context (does not corrupt the saved context)
    template <class L>
    bool ExecuteOnThisContext(L *lambda, StackfulCoroutineContext *requester)
    {
        ASSERT(requester != nullptr);
        return rpc_call_context_.Execute(lambda, &requester->context_, &context_);
    }

    /// assign this coroutine to a worker thread
    void SetWorker(StackfulCoroutineWorker *w)
    {
        worker_ = w;
    }

    /// get the currently assigned worker thread
    StackfulCoroutineWorker *GetWorker() const
    {
        return worker_;
    }

protected:
    void SetStatus(Coroutine::Status new_status) override;

private:
    void ThreadProcImpl();
    static void CoroThreadProc(void *ctx);

    /// @brief The remote lambda call functionality implementation.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    class RemoteCall {
    public:
        NO_COPY_SEMANTIC(RemoteCall);
        NO_MOVE_SEMANTIC(RemoteCall);

        template <class L>
        bool Execute(L *lambda, fibers::FiberContext *requester_context_ptr, fibers::FiberContext *host_context_ptr)
        {
            ASSERT(Coroutine::GetCurrent()->GetVM()->GetThreadManager()->GetMainThread() !=
                   ManagedThread::GetCurrent());

            call_in_progress_ = true;
            requester_context_ptr_ = requester_context_ptr;
            lambda_ = lambda;

            fibers::CopyContext(&guest_context_, host_context_ptr);
            fibers::UpdateContextKeepStack(&guest_context_, RemoteCall::Proxy<L>, this);
            fibers::SwitchContext(requester_context_ptr_, &guest_context_);

            return true;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        RemoteCall() = default;
        ~RemoteCall() = default;

    private:
        template <class L>
        static void Proxy(void *ctx)
        {
            auto *this_instance = static_cast<RemoteCall *>(ctx);
            ASSERT(this_instance->call_in_progress_);

            (*static_cast<L *>(this_instance->lambda_))();

            this_instance->call_in_progress_ = false;
            fibers::SwitchContext(&this_instance->guest_context_, this_instance->requester_context_ptr_);
        }

        bool call_in_progress_ = false;
        fibers::FiberContext *requester_context_ptr_ = nullptr;
        fibers::FiberContext guest_context_;
        void *lambda_ = nullptr;
    } rpc_call_context_;

    uint8_t *stack_ = nullptr;
    size_t stack_size_bytes_ = 0;
    fibers::FiberContext context_;
    Coroutine::Status status_ {Coroutine::Status::CREATED};
    StackfulCoroutineWorker *worker_ = nullptr;
};

}  // namespace panda

#endif  // PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_H
