/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H

#include "runtime/execution/coroutines/coroutine_context.h"
#include "runtime/execution/coroutines/coroutine.h"
#include "runtime/execution/coroutines/coroutine_manager.h"
#include "runtime/include/panda_vm.h"
#include "plugins/ets/runtime/ets_execution_context.h"

namespace ark::ets {
class PandaEtsVM;
class EtsReference;
class EtsObject;
class EtsPromise;

/// @brief The eTS coroutine. It is aware of the native interface and reference storage existance.
class EtsCoroutine : public Coroutine {
public:
    NO_COPY_SEMANTIC(EtsCoroutine);
    NO_MOVE_SEMANTIC(EtsCoroutine);

    /**
     * @brief EtsCoroutine factory: the preferrable way to create a coroutine. See
     * CoroutineManager::CoroutineFactory for details.
     *
     * Since C++ requires function type to exactly match the formal parameter type, we have to make this factory a
     * template. The sole purpose for this is to be able to return both Coroutine* and EtsCoroutine*
     */
    template <class T>
    static T *Create(Runtime *runtime, PandaVM *vm, Job *job, CoroutineContext *context)
    {
        mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
        auto co = allocator->New<EtsCoroutine>(os::thread::GetCurrentThreadId(), allocator, vm, job, context);
        ASSERT(co != nullptr);
        co->Initialize();
        return co;
    }

    ~EtsCoroutine() override = default;

    static EtsCoroutine *CastFromThread(ManagedThread *thread)
    {
        ASSERT(thread != nullptr);
        return static_cast<EtsCoroutine *>(thread);
    }

    static EtsCoroutine *GetCurrent()
    {
        Coroutine *co = Coroutine::GetCurrent();
        if ((co != nullptr) && co->GetThreadLang() == ark::panda_file::SourceLang::ETS) {
            return CastFromThread(co);
        }
        return nullptr;
    }

    EtsExecutionContext *GetExecutionCtx()
    {
        return &executionCtx_;
    }

    static constexpr uint32_t GetExecutionContextOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, executionCtx_);
    }

    ALWAYS_INLINE EtsReference *GetAsyncContext() const
    {
        return asyncContext_;
    }

    void SetAsyncContext(EtsReference *ctxRef)
    {
        asyncContext_ = ctxRef;
    }

    PANDA_PUBLIC_API PandaEtsVM *GetPandaVM() const;

    void Initialize() override;
    void ReInitialize(Job *job, CoroutineContext *context) override;
    void CleanUp() override;
    void RequestCompletion(Value returnValue) override;
    void FreeInternalMemory() override;
    void UpdateCachedObjects() override;
    void ListUnhandledEventsOnProgramExit() override;

    /// @brief traverse current unhandled failed jobs with custom handler
    void ProcessUnhandledFailedJobs();

    // event handlers
    void OnContextSwitchedTo() override;
    void OnChildCoroutineCreated(Coroutine *child) override;

    /// @brief print stack and exit the program
    void HandleUncaughtException() override;

    /// The method returns true if there are interop JS code frames in the coroutine call stack
    bool IsContextSwitchRisky() const override;

    void PrintCallStack() const override;

    static constexpr CoroutinePriority ASYNC_CALL = CoroutinePriority::HIGH_PRIORITY;
    static constexpr CoroutinePriority PROMISE_CALLBACK = CoroutinePriority::HIGH_PRIORITY;
    static constexpr CoroutinePriority TIMER_CALLBACK = CoroutinePriority::MEDIUM_PRIORITY;
    static constexpr CoroutinePriority LAUNCH = CoroutinePriority::MEDIUM_PRIORITY;

protected:
    // we would like everyone to use the factory to create a EtsCoroutine
    explicit EtsCoroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, Job *job,
                          CoroutineContext *context);

private:
    EtsExecutionContext executionCtx_;
    EtsReference *asyncContext_ {nullptr};

    panda_file::Type GetReturnType();
    EtsObject *GetReturnValueAsObject(panda_file::Type returnType, Value returnValue);
    EtsObject *GetValueFromPromiseSync(EtsPromise *promise);

    void RequestPromiseCompletion(mem::Reference *promiseRef, Value returnValue);
    void RequestJobCompletion(mem::Reference *jobRef, Value returnValue);

    // Allocator calls our protected ctor
    friend class mem::Allocator;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H
