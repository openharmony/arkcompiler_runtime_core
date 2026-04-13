/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_JOB_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_JOB_H_

#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/include/object_header.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark {
class ManagedThread;
}  // namespace ark

namespace ark::ets {

namespace test {
class EtsJobTest;
}  // namespace test

class EtsJob : public EtsObject {
public:
    static constexpr EtsInt STATE_RUNNING = 0;
    static constexpr EtsInt STATE_FINISHED = 1;
    static constexpr EtsInt STATE_FAILED = 2;

    PANDA_PUBLIC_API static EtsJob *Create(EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent());

    static void EtsJobFinish(EtsJob *job, EtsObject *value);

    static void EtsJobFail(EtsJob *job, EtsObject *error);

    static EtsJob *FromCoreType(ObjectHeader *job)
    {
        return reinterpret_cast<EtsJob *>(job);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsJob *FromEtsObject(EtsObject *job)
    {
        return static_cast<EtsJob *>(job);
    }

    EtsMutex *GetMutex(EtsExecutionContext *executionCtx)
    {
        auto *obj = ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, mutex_));
        return EtsMutex::FromCoreType(obj);
    }

    void SetMutex(EtsExecutionContext *executionCtx, EtsMutex *mutex)
    {
        ASSERT(mutex != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, mutex_), mutex->GetCoreType());
    }

    EtsEvent *GetEvent(EtsExecutionContext *executionCtx)
    {
        auto *obj = ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, event_));
        return EtsEvent::FromCoreType(obj);
    }

    void SetEvent(EtsExecutionContext *executionCtx, EtsEvent *event)
    {
        ASSERT(event != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, event_), event->GetCoreType());
    }

    EtsObject *GetValue(EtsExecutionContext *executionCtx)
    {
        return EtsObject::FromCoreType(
            ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, value_)));
    }

    /// Allows to get exclusive access to the job state
    void Lock()
    {
        auto *mutex = GetMutex(EtsExecutionContext::GetCurrent());
        ASSERT(mutex != nullptr);
        mutex->Lock();
    }

    void Unlock()
    {
        auto *mutex = GetMutex(EtsExecutionContext::GetCurrent());
        ASSERT(mutex != nullptr);
        ASSERT(mutex->IsHeld());
        mutex->Unlock();
    }

    bool IsLocked()
    {
        auto *mutex = GetMutex(EtsExecutionContext::GetCurrent());
        ASSERT(mutex != nullptr);
        return mutex->IsHeld();
    }

    /// Blocks current executionCtxutine until job is resolved/rejected
    void Wait()
    {
        auto *event = GetEvent(EtsExecutionContext::GetCurrent());
        ASSERT(event != nullptr);
        event->Wait();
    }

    bool IsRunning() const
    {
        return state_ == STATE_RUNNING;
    }

    bool IsFinished() const
    {
        return state_ == STATE_FINISHED;
    }

    bool IsFailed() const
    {
        return state_ == STATE_FAILED;
    }

    void Finish(EtsExecutionContext *executionCtx, EtsObject *value)
    {
        ASSERT(state_ == STATE_RUNNING);
        auto coreValue = (value == nullptr) ? nullptr : value->GetCoreType();
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, value_), coreValue);
        state_ = STATE_FINISHED;
        // Unblock awaitee executionCtxs
        GetEvent(executionCtx)->Fire();
    }

    void Fail(EtsExecutionContext *executionCtx, EtsObject *error)
    {
        ASSERT(error != nullptr);
        ASSERT(state_ == STATE_RUNNING);
        ASSERT(error != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsJob, value_), error->GetCoreType());
        state_ = STATE_FAILED;
        executionCtx->GetPandaVM()->GetUnhandledObjectManager()->AddFailedJob(this);
        // Unblock awaitee executionCtxs
        GetEvent(executionCtx)->Fire();
    }

private:
    ObjectPointer<EtsObject> value_;  // the completion value of the Job
    ObjectPointer<EtsMutex> mutex_;
    ObjectPointer<EtsEvent> event_;
    uint32_t state_;  // the Job state

    friend class test::EtsJobTest;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_JOB_H_
