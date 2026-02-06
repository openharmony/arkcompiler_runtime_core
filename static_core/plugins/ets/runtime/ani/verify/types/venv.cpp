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

#include "plugins/ets/runtime/ani/verify/types/venv.h"

#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets::ani::verify {

PandaUniquePtr<VEnv> VEnv::Create()
{
    return MakePandaUnique<VEnv>();
}

ani_env *VEnv::GetEnv()
{
    return EtsCoroutine::GetCurrent()->GetEtsNapiEnv();
}

/*static*/
VEnv *VEnv::GetCurrent()
{
    if (Thread::GetCurrent() == nullptr) {
        return nullptr;
    }
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (coro == nullptr) {
        return nullptr;
    }
    auto venv = coro->GetEtsNapiEnv()->GetEnvANIVerifier()->GetEnv();
    return venv;
}

void VEnv::CreateLocalScope()
{
    GetEnvANIVerifier()->CreateLocalScope();
}

std::optional<PandaString> VEnv::DestroyLocalScope()
{
    return GetEnvANIVerifier()->DestroyLocalScope();
}

void VEnv::CreateEscapeLocalScope()
{
    GetEnvANIVerifier()->CreateEscapeLocalScope();
}

std::optional<PandaString> VEnv::DestroyEscapeLocalScope(VRef *vref)
{
    return GetEnvANIVerifier()->DestroyEscapeLocalScope(vref);
}

// Local refs
template <class RefType>
auto VEnv::AddLocalVerifiedRef(RefType ref)
{
    return GetEnvANIVerifier()->AddLocalVerifiedRef(ref);
}

void VEnv::DeleteLocalVerifiedRef(VRef *vref)
{
    GetEnvANIVerifier()->DeleteLocalVerifiedRef(vref);
}

VMethod *VEnv::GetVerifiedMethod(ani_method method)
{
    return GetEnvANIVerifier()->GetVerifiedMethod(method);
}

VStaticMethod *VEnv::GetVerifiedStaticMethod(ani_static_method staticMethod)
{
    return GetEnvANIVerifier()->GetVerifiedStaticMethod(staticMethod);
}

VFunction *VEnv::GetVerifiedFunction(ani_function function)
{
    return GetEnvANIVerifier()->GetVerifiedFunction(function);
}

VField *VEnv::GetVerifiedField(ani_field field)
{
    return GetEnvANIVerifier()->GetVerifiedField(field);
}

VStaticField *VEnv::GetVerifiedStaticField(ani_static_field staticField)
{
    return GetEnvANIVerifier()->GetVerifiedStaticField(staticField);
}

VRef *VEnv::AddGlobalVerifiedRef(ani_ref gref)
{
    return GetEnvANIVerifier()->AddGlobalVerifiedRef(gref);
}

void VEnv::DeleteGlobalVerifiedRef(VRef *vgref)
{
    GetEnvANIVerifier()->DeleteGlobalVerifiedRef(vgref);
}

bool VEnv::IsValidGlobalVerifiedRef(VRef *vgref)
{
    return GetEnvANIVerifier()->IsValidGlobalVerifiedRef(vgref);
}

VResolver *VEnv::AddGlobalVerifiedResolver(ani_resolver resolver)
{
    return GetEnvANIVerifier()->AddGlobalVerifiedResolver(resolver);
}

void VEnv::DeleteGlobalVerifiedResolver(VResolver *vresolver)
{
    GetEnvANIVerifier()->DeleteGlobalVerifiedResolver(vresolver);
}

bool VEnv::IsValidGlobalVerifiedResolver(VResolver *vresolver)
{
    return GetEnvANIVerifier()->IsValidGlobalVerifiedResolver(vresolver);
}

EnvANIVerifier *VEnv::GetEnvANIVerifier()
{
    return PandaEnv::FromAniEnv(GetEnv())->GetEnvANIVerifier();
}

}  // namespace ark::ets::ani::verify
