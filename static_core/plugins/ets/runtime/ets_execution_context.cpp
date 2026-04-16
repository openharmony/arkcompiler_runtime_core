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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/execution/job_execution_context.h"
#include "plugins/ets/runtime/ets_execution_context_wrapper.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {

// ExternalIfaceTable contains std::function, which cannot be trivially constructed even for nullptr
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
ExternalIfaceTable EtsExecutionContext::emptyExternalIfaceTable_ = ExternalIfaceTable();

EtsExecutionContext::EtsExecutionContext(ManagedThread *mThread) : mThread_(mThread) {}

void EtsExecutionContext::Initialize()
{
    auto *mThread = GetMT();
    auto allocator = mThread->GetVM()->GetHeapManager()->GetInternalAllocator();
    auto etsAniEnv = PandaAniEnv::Create(this, allocator);
    if (!etsAniEnv) {
        LOG(FATAL, RUNTIME) << "Cannot create PandaEtsNapiEnv: " << etsAniEnv.Error();
    }
    etsAniEnv_ = etsAniEnv.Value();
    // Main EtsExecutionContext is Initialized before class linker and promise_class_ptr_ will be set after creating the
    // class
    if (Runtime::GetCurrent()->IsInitialized()) {
        CacheBuiltinClasses();
    }
}

void EtsExecutionContext::CacheBuiltinClasses()
{
    auto platformTypes = PlatformTypes(GetPandaVM());
    GetLocalStorage().Set<EtsExecutionContext::DataIdx::ETS_PLATFORM_TYPES_PTR>(ToUintPtr(platformTypes));
    auto *mThread = GetMT();
    promiseClassPtr_ = platformTypes->corePromise->GetRuntimeClass();
    SetupNullValue(GetPandaVM()->GetNullValue());
    // NOTE (electronick, #15938): Refactor the managed class-related pseudo TLS fields
    // initialization in MT ManagedThread ctor and EtsCoroutine::Initialize
    auto *linkExt = GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension();
    mThread->SetStringClassPtr(linkExt->GetClassRoot(ClassRoot::LINE_STRING));
    mThread->SetArrayU16ClassPtr(linkExt->GetClassRoot(ClassRoot::ARRAY_U16));
    mThread->SetArrayU8ClassPtr(linkExt->GetClassRoot(ClassRoot::ARRAY_U8));
}

void EtsExecutionContext::CleanUp()
{
    taskpoolTaskid_ = INVALID_TASKPOOL_TASK_ID;
    etsAniEnv_->CleanUp();
}

void EtsExecutionContext::ReInitialize()
{
    etsAniEnv_->ReInitialize();
}

/* static */
EtsExecutionContext *EtsExecutionContext::FromMT(ManagedThread *mThread)
{
    ASSERT(mThread != nullptr);
    if (Coroutine::MutatorIsCoroutine(mThread) && mThread->GetThreadLang() == ark::panda_file::SourceLang::ETS) {
        return EtsCoroutine::CastFromThread(mThread)->GetExecutionCtx();
    }
    return EtsExecutionContextWrapper::CastFromMutator(mThread)->GetExecutionCtx();
}

PandaEtsVM *EtsExecutionContext::GetPandaVM() const
{
    return static_cast<PandaEtsVM *>(GetMT()->GetVM());
}

ExternalIfaceTable *EtsExecutionContext::GetExternalIfaceTable()
{
    auto *worker = JobExecutionContext::CastFromMutator(GetMT())->GetWorker();
    auto *table = worker->GetLocalStorage().Get<JobWorkerThread::DataIdx::EXTERNAL_IFACES, ExternalIfaceTable *>();
    if (table != nullptr) {
        return table;
    }
    return &emptyExternalIfaceTable_;
}

void EtsExecutionContext::SetTaskpoolTaskId(int32_t taskid)
{
    taskpoolTaskid_ = taskid;
}

int32_t EtsExecutionContext::GetTaskpoolTaskId() const
{
    return taskpoolTaskid_;
}

void EtsExecutionContext::ProcessUnhandledFailedJobs()
{
    if (Runtime::GetOptions().IsArkAot()) {
        return;
    }
    auto *umanager = GetPandaVM()->GetUnhandledObjectManager();
    ASSERT(umanager != nullptr);
    ASSERT_NATIVE_CODE();
    if (umanager->HasFailedJobObjects()) {
        {
            [[maybe_unused]] ScopedManagedCodeThread sc(GetMT());
            umanager->ListFailedJobs(this);
        }
        if (GetMT()->HasPendingException()) {
            GetPandaVM()->HandleUncaughtException();
        }
    }
}

void EtsExecutionContext::ProcessUnhandledRejectedPromises(bool listAllObjects)
{
    if (Runtime::GetOptions().IsArkAot()) {
        return;
    }
    auto *umanager = GetPandaVM()->GetUnhandledObjectManager();
    ASSERT(umanager != nullptr);
    ASSERT_NATIVE_CODE();
    if (umanager->HasRejectedPromiseObjects(this, listAllObjects)) {
        LOG(DEBUG, EXECUTION) << "Start processing unhandled promises in "
                              << JobExecutionContext::CastFromMutator(GetMT())->GetJob()->GetName();
        {
            [[maybe_unused]] ScopedManagedCodeThread sc(GetMT());
            umanager->ListRejectedPromises(this, listAllObjects);
        }
        if (GetMT()->HasPendingException()) {
            GetPandaVM()->HandleUncaughtException();
        }
    }
}

}  // namespace ark::ets
