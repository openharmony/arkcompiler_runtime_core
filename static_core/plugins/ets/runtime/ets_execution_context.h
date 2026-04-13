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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_EXECUTION_CONTEXT_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_EXECUTION_CONTEXT_H

#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/panda_vm.h"
#include "runtime/execution/local_storage.h"
#include "plugins/ets/runtime/external_iface_table.h"
#include "plugins/ets/runtime/ets_ani_env.h"

namespace ark::ets {
class PandaEtsNapiEnv;
class PandaEtsVM;

class EtsExecutionContext {
public:
    NO_COPY_SEMANTIC(EtsExecutionContext);
    NO_MOVE_SEMANTIC(EtsExecutionContext);

    enum class DataIdx { ETS_PLATFORM_TYPES_PTR, INTEROP_CTX_PTR, INTEROP_CALL_STACK_PTR, LAST_ID };
    using LocalStorage = StaticLocalStorage<DataIdx>;

    explicit EtsExecutionContext(ManagedThread *mThread);

    ~EtsExecutionContext()
    {
        auto allocator = GetMT()->GetVM()->GetHeapManager()->GetInternalAllocator();
        allocator->Delete(etsAniEnv_);
    }

    void Initialize();
    void CleanUp();
    void ReInitialize();

    void CacheBuiltinClasses();

    static EtsExecutionContext *GetCurrent()
    {
        return FromMT(ManagedThread::GetCurrent());
    }

    PANDA_PUBLIC_API static EtsExecutionContext *FromMT(ManagedThread *mThread);

    ManagedThread *GetMT() const
    {
        return mThread_;
    }

    ExternalIfaceTable *GetExternalIfaceTable();

    void SetPromiseClass(void *promiseClass)
    {
        promiseClassPtr_ = promiseClass;
    }

    void *GetPromiseClass() const
    {
        return promiseClassPtr_;
    }

    void SetJobClass(void *jobClass)
    {
        jobClassPtr_ = jobClass;
    }

    // Returns a unique object representing "null" reference
    ALWAYS_INLINE ObjectHeader *GetNullValue() const
    {
        return nullValue_;
    }

    // For mainthread initializer
    void SetupNullValue(ObjectHeader *obj)
    {
        nullValue_ = obj;
    }

    static constexpr uint32_t GetTlsPromiseClassPointerOffset()
    {
        return MEMBER_OFFSET(EtsExecutionContext, promiseClassPtr_);
    }

    static constexpr uint32_t GetTlsNullValueOffset()
    {
        return MEMBER_OFFSET(EtsExecutionContext, nullValue_);
    }

    static constexpr uint32_t GetTlsAniEnvOffset()
    {
        return MEMBER_OFFSET(EtsExecutionContext, etsAniEnv_);
    }

    static constexpr uint32_t GetLocalStorageOffset()
    {
        return MEMBER_OFFSET(EtsExecutionContext, localStorage_);
    }

    PandaAniEnv *GetPandaAniEnv() const
    {
        return etsAniEnv_;
    }

    PANDA_PUBLIC_API PandaEtsVM *GetPandaVM() const;

    LocalStorage &GetLocalStorage()
    {
        return localStorage_;
    }

    const LocalStorage &GetLocalStorage() const
    {
        return localStorage_;
    }

    void SetTaskpoolTaskId(int32_t taskid);

    int32_t GetTaskpoolTaskId() const;

    /// @brief traverse current unhandled failed jobs with custom handler
    void ProcessUnhandledFailedJobs();

    /// @brief traverse current unhandled rejected promises with custom handler
    void ProcessUnhandledRejectedPromises(bool listAllObjects);

private:
    PandaAniEnv *etsAniEnv_ {nullptr};
    void *promiseClassPtr_ {nullptr};
    void *jobClassPtr_ {nullptr};

    ObjectHeader *nullValue_ {};

    static ExternalIfaceTable emptyExternalIfaceTable_;

    LocalStorage localStorage_;

    static_assert(std::is_pointer_v<decltype(etsAniEnv_)>,
                  "we load a raw pointer in compiled code, please don't change the type!");

    // NOTE(atlantiswang, #32562): this member variable should be moved to the execution context's local storage
    static constexpr int32_t INVALID_TASKPOOL_TASK_ID = 0;
    int32_t taskpoolTaskid_ {INVALID_TASKPOOL_TASK_ID};

    ManagedThread *mThread_;
};

}  // namespace ark::ets

#endif
