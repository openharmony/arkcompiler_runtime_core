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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_RESOLVE_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_RESOLVE_H

#include "plugins/ets/runtime/ani/verify/env_ani_verifier.h"
#include "plugins/ets/runtime/ani/verify/types/vfield.h"
#include "plugins/ets/runtime/ani/verify/types/vmethod.h"
#include "plugins/ets/runtime/ani/verify/types/internal_ref.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vresolver.h"
#include "plugins/ets/runtime/ani/verify/types/vwref.h"
#include "plugins/ets/runtime/ani/verify/verify_ani_cast_api.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_execution_context.h"

namespace ark::ets {
class EtsMethod;
class EtsField;
}  // namespace ark::ets

namespace ark::ets::ani::verify {

inline EnvANIVerifier *GetCurrentEnvANIVerifier()
{
    ani_env *env = EtsExecutionContext::GetCurrent()->GetPandaAniEnv();
    return PandaAniEnv::FromAniEnv(env)->GetEnvANIVerifier();
}

namespace internal {

template <typename VTy>
using AniType = ConvertToANIType<VTy *>;

}  // namespace internal

template <typename VTy>
inline internal::AniType<VTy> ResolveToAniRef(VTy *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawAniGlobalRef(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<internal::AniType<VTy>>(maybe);
    }
    if (InternalRef::IsVRef(reinterpret_cast<VRef *>(maybe))) {
        return maybe->GetRef();
    }
    return reinterpret_cast<internal::AniType<VTy>>(maybe);
}

template <typename VTy>
inline internal::AniType<VTy> ResolveToAniRef(VTy *maybe)
{
    return ResolveToAniRef(maybe, GetCurrentEnvANIVerifier());
}

inline ani_wref ResolveToAniRef(VWRef *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawAniWeakRef(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<ani_wref>(maybe);
    }
    if (InternalRef::IsVWRef(maybe)) {
        return maybe->GetWRef();
    }
    return reinterpret_cast<ani_wref>(maybe);
}

inline ani_wref ResolveToAniRef(VWRef *maybe)
{
    return ResolveToAniRef(maybe, GetCurrentEnvANIVerifier());
}

inline ani_variable ResolveToAniRef(VVariable *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawEtsField(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<ani_variable>(maybe);
    }
    return maybe->GetVariable();
}

inline ani_variable ResolveToAniRef(VVariable *maybe)
{
    return ResolveToAniRef(maybe, GetCurrentEnvANIVerifier());
}

inline ani_resolver ResolveToAniRef(VResolver *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawAniGlobalRef(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<ani_resolver>(maybe);
    }
    return maybe->GetResolver();
}

inline ani_resolver ResolveToAniRef(VResolver *maybe)
{
    return ResolveToAniRef(maybe, GetCurrentEnvANIVerifier());
}

template <typename VTy>
inline internal::AniType<VTy> ResolveToAniEtsMethodHandle(VTy *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawEtsMethod(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<internal::AniType<VTy>>(maybe);
    }
    return reinterpret_cast<internal::AniType<VTy>>(maybe->GetEtsMethod());
}

template <typename VTy>
inline internal::AniType<VTy> ResolveToAniEtsMethodHandle(VTy *maybe)
{
    return ResolveToAniEtsMethodHandle(maybe, GetCurrentEnvANIVerifier());
}

template <typename VTy>
inline internal::AniType<VTy> ResolveToAniEtsFieldHandle(VTy *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawEtsField(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<internal::AniType<VTy>>(maybe);
    }
    return reinterpret_cast<internal::AniType<VTy>>(maybe->GetEtsField());
}

template <typename VTy>
inline internal::AniType<VTy> ResolveToAniEtsFieldHandle(VTy *maybe)
{
    return ResolveToAniEtsFieldHandle(maybe, GetCurrentEnvANIVerifier());
}

inline EtsMethod *ResolveToEtsMethod(impl::VMethod *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawEtsMethod(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<EtsMethod *>(maybe);
    }
    return maybe->GetEtsMethod();
}

inline EtsMethod *ResolveToEtsMethod(impl::VMethod *maybe)
{
    return ResolveToEtsMethod(maybe, GetCurrentEnvANIVerifier());
}

inline EtsField *ResolveToEtsField(impl::VField *maybe, EnvANIVerifier *verifier)
{
    if (verifier->IsValidRawEtsField(reinterpret_cast<void *>(maybe))) {
        return reinterpret_cast<EtsField *>(maybe);
    }
    return maybe->GetEtsField();
}

inline EtsField *ResolveToEtsField(impl::VField *maybe)
{
    return ResolveToEtsField(maybe, GetCurrentEnvANIVerifier());
}

inline EtsMethod *GetEtsMethodIfPointerValid(impl::VMethod *vmethod, EnvANIVerifier *verifier)
{
    if (!verifier->IsValidMethod(vmethod)) {
        return nullptr;
    }
    return ResolveToEtsMethod(vmethod, verifier);
}

inline EtsMethod *GetEtsMethodIfPointerValid(impl::VMethod *vmethod)
{
    return GetEtsMethodIfPointerValid(vmethod, GetCurrentEnvANIVerifier());
}

inline ani_class ResolveToAniClass(VClass *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_module ResolveToAniModule(VModule *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_namespace ResolveToAniNamespace(VNamespace *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_type ResolveToAniType(VType *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_method ResolveToAniMethod(VMethod *maybe)
{
    return ResolveToAniEtsMethodHandle(maybe);
}

inline ani_static_method ResolveToAniStaticMethod(VStaticMethod *maybe)
{
    return ResolveToAniEtsMethodHandle(maybe);
}

inline ani_function ResolveToAniFunction(VFunction *maybe)
{
    return ResolveToAniEtsMethodHandle(maybe);
}

inline ani_field ResolveToAniField(VField *maybe)
{
    return ResolveToAniEtsFieldHandle(maybe);
}

inline ani_static_field ResolveToAniStaticField(VStaticField *maybe)
{
    return ResolveToAniEtsFieldHandle(maybe);
}

inline ani_variable ResolveToAniVariable(VVariable *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_enum ResolveToAniEnum(VEnum *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_enum_item ResolveToAniEnumItem(VEnumItem *maybe)
{
    return ResolveToAniRef(maybe);
}

inline ani_resolver ResolveToAniResolver(VResolver *maybe)
{
    return ResolveToAniRef(maybe);
}

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ANI_RESOLVE_H
