/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_EXECUTION_CONTEXT_WRAPPER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_EXECUTION_CONTEXT_WRAPPER_H

#include "runtime/execution/job_execution_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/types/ets_async_context.h"

namespace ark {
class CompletionEvent;
}  // namespace ark

namespace ark::ets {

// NOTE(konstanting): The stackless execution context.
// Will eventually be merged with the EtsExecutionContext.
class EtsExecutionContextWrapper : public JobExecutionContext {
public:
    NO_COPY_SEMANTIC(EtsExecutionContextWrapper);
    NO_MOVE_SEMANTIC(EtsExecutionContextWrapper);

    static JobExecutionContext *Create(Runtime *runtime, PandaVM *vm, Job *job)
    {
        mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
        auto ectxw = allocator->New<EtsExecutionContextWrapper>(os::thread::GetCurrentThreadId(), allocator, vm, job);
        ASSERT(ectxw != nullptr);
        ectxw->Initialize();
        return ectxw;
    }

    ~EtsExecutionContextWrapper() override = default;

    static EtsExecutionContextWrapper *CastFromMutator(Mutator *mutator)
    {
        ASSERT(mutator != nullptr);
        return static_cast<EtsExecutionContextWrapper *>(mutator);
    }

    EtsExecutionContext *GetExecutionCtx()
    {
        return &executionCtx_;
    }

    static constexpr uint32_t GetExecutionContextOffset()
    {
        return MEMBER_OFFSET(EtsExecutionContextWrapper, executionCtx_);
    }

    ObjectHeader *GetSuspensionContext() const override
    {
        return EtsAsyncContext::ToCoreType(asyncContext_);
    }

    void SetSuspensionContext(mem::Reference *asyncCtxRef) override
    {
        if (asyncCtxRef == nullptr) {
            asyncContext_ = nullptr;
            return;
        }
        auto *etsAsyncCtxRef = EtsReference::CastFromReference(asyncCtxRef);
        auto *refStorage = executionCtx_.GetPandaAniEnv()->GetEtsReferenceStorage();
        asyncContext_ = EtsAsyncContext::FromEtsObject(refStorage->GetEtsObject(etsAsyncCtxRef));
    }

    void Initialize();

    void UpdateCachedObjects() override;

    void CacheBuiltinClasses() override;

    void OnJobCompletion(Value returnValue) override;

    void VisitGCRoots(const GCRootVisitor &cb) override;

protected:
    // we would like everyone to use the factory to create a EtsExecutionContextWrapper
    explicit EtsExecutionContextWrapper(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, Job *job);

private:
    EtsObject *BoxReturnValue(Value returnValue);
    EtsObject *GetCompletionObject(mem::Reference *retValueRef);
    EtsObject *TakePendingException(Job *job);
    void CompleteJob(EtsJob *completedJob, EtsObject *retObject, Job *job);
    void CompletePromise(EtsPromise *completedPromise, EtsObject *retObject, Job *job);

    EtsExecutionContext executionCtx_;
    EtsAsyncContext *asyncContext_ = nullptr;

    // Allocator calls our protected ctor
    friend class mem::Allocator;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_EXECUTION_CONTEXT_WRAPPER_H
