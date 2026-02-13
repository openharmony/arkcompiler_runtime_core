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

#include "plugins/ets/runtime/ets_ani_env.h"

#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_interaction_api.h"
#include "plugins/ets/runtime/ani/verify/types/venv.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

/* static */
Expected<PandaAniEnv *, const char *> PandaAniEnv::Create(EtsExecutionContext *executionCtx,
                                                          mem::InternalAllocatorPtr allocator)
{
    auto *etsVm = executionCtx->GetPandaVM();
    auto referenceStorage = MakePandaUnique<EtsReferenceStorage>(etsVm->GetGlobalObjectStorage(), allocator, false);
    if (!referenceStorage || !referenceStorage->GetAsReferenceStorage()->Init()) {
        return Unexpected("Cannot allocate EtsReferenceStorage");
    }

    auto *aniEnv = allocator->New<PandaAniEnv>(executionCtx, std::move(referenceStorage));
    if (aniEnv == nullptr) {
        return Unexpected("Cannot allocate PandaAniEnv");
    }

    return Expected<PandaAniEnv *, const char *>(aniEnv);
}

/* static */
PandaAniEnv *PandaAniEnv::FromAniEnv(ani_env *env)
{
    ASSERT(env != nullptr);
    if (UNLIKELY(ani::verify::VEnv::IsVEnv(env))) {
        env = ani::verify::VEnv::FromAniEnv(env)->GetEnv();
    }
    return static_cast<PandaAniEnv *>(env);
}

PandaAniEnv::PandaAniEnv(EtsExecutionContext *executionCtx, PandaUniquePtr<EtsReferenceStorage> referenceStorage)
    : ani_env {ani::GetInteractionAPI()}, executionCtx_(executionCtx), referenceStorage_(std::move(referenceStorage))
{
    auto *vm = executionCtx->GetPandaVM();
    if (vm->IsVerifyANI()) {
        ani::verify::ANIVerifier *verifier = vm->GetANIVerifier();
        envANIVerifier_ = MakePandaUnique<ani::verify::EnvANIVerifier>(this, verifier, c_api);
    }
}

PandaEtsVM *PandaAniEnv::GetEtsVM() const
{
    return executionCtx_->GetPandaVM();
}

void PandaAniEnv::FreeInternalMemory()
{
    referenceStorage_.reset();
}

void PandaAniEnv::CleanUp()
{
    referenceStorage_->CleanUp();
}

void PandaAniEnv::ReInitialize()
{
    referenceStorage_->ReInitialize();
}

void PandaAniEnv::SetException(EtsThrowable *thr)
{
    ASSERT_MANAGED_CODE();
    executionCtx_->GetMT()->SetException(thr->GetCoreType());
}

EtsThrowable *PandaAniEnv::GetThrowable() const
{
    ASSERT_MANAGED_CODE();
    return reinterpret_cast<EtsThrowable *>(executionCtx_->GetMT()->GetException());
}

bool PandaAniEnv::HasPendingException() const
{
    return executionCtx_->GetMT()->HasPendingException();
}

void PandaAniEnv::ClearException()
{
    ASSERT_MANAGED_CODE();
    executionCtx_->GetMT()->ClearException();
}

}  // namespace ark::ets
