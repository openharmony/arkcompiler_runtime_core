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

#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/napi/ets_napi_native_interface.h"

namespace panda::ets {
Expected<std::unique_ptr<PandaEtsNapiEnv>, const char *> PandaEtsNapiEnv::Create(EtsCoroutine *coroutine,
                                                                                 mem::InternalAllocatorPtr allocator)
{
    auto ets_vm = coroutine->GetVM();
    auto reference_storage = MakePandaUnique<EtsReferenceStorage>(ets_vm->GetGlobalObjectStorage(), allocator, false);
    if (!reference_storage || !reference_storage->GetAsReferenceStorage()->Init()) {
        return Unexpected("Cannot allocate EtsReferenceStorage");
    }

    // Do not use PandaUniquePtr here as the environment could be accessed from daemon threads after destroy of runtime
    auto ets_napi_env = std::make_unique<PandaEtsNapiEnv>(coroutine, std::move(reference_storage));
    if (!ets_napi_env) {
        return Unexpected("Cannot allocate PandaEtsNapiEnv");
    }

    return Expected<std::unique_ptr<PandaEtsNapiEnv>, const char *>(std::move(ets_napi_env));
}

PandaEtsNapiEnv *PandaEtsNapiEnv::GetCurrent()
{
    return EtsCoroutine::GetCurrent()->GetEtsNapiEnv();
}

PandaEtsNapiEnv::PandaEtsNapiEnv(EtsCoroutine *coroutine, PandaUniquePtr<EtsReferenceStorage> reference_storage)
    : EtsEnv {napi::GetNativeInterface()}, coroutine_(coroutine), reference_storage_(std::move(reference_storage))
{
}

PandaEtsVM *PandaEtsNapiEnv::GetEtsVM() const
{
    return coroutine_->GetPandaVM();
}

void PandaEtsNapiEnv::FreeInternalMemory()
{
    reference_storage_.reset();
}

void PandaEtsNapiEnv::SetException(EtsThrowable *thr)
{
    ASSERT_MANAGED_CODE();
    coroutine_->SetException(thr->GetCoreType());
}

EtsThrowable *PandaEtsNapiEnv::GetThrowable() const
{
    ASSERT_MANAGED_CODE();
    return reinterpret_cast<EtsThrowable *>(coroutine_->GetException());
}

bool PandaEtsNapiEnv::HasPendingException() const
{
    return coroutine_->HasPendingException();
}

void PandaEtsNapiEnv::ClearException()
{
    ASSERT_MANAGED_CODE();
    coroutine_->ClearException();
}

}  // namespace panda::ets