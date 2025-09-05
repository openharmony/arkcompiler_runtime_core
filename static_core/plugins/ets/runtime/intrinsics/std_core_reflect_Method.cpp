/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cctype>
#include "intrinsics.h"
#include "types/ets_method.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"

namespace ark::ets::intrinsics {

extern "C" EtsClass *ReflectMethodGetReturnTypeImpl(EtsLong etsMethodPtr)
{
    auto *retCls = reinterpret_cast<EtsMethod *>(etsMethodPtr)->ResolveReturnType();
    return retCls->ResolvePublicClass();
}

extern "C" EtsClass *ReflectMethodGetParameterTypeByIdxImpl(EtsLong etsFunctionPtr, EtsInt i)
{
    auto *function = reinterpret_cast<EtsMethod *>(etsFunctionPtr);
    ASSERT(function != nullptr);
    ASSERT(i >= 0 && static_cast<uint32_t>(i) < function->GetNumArgs());
    // 0 is recevier type
    i = function->IsStatic() ? i : i + 1;
    auto *resolvedType = function->ResolveArgType(i)->ResolvePublicClass();

    return resolvedType;
}

extern "C" EtsInt ReflectMethodGetParametersNumImpl(EtsLong etsFunctionPtr)
{
    auto *function = reinterpret_cast<EtsMethod *>(etsFunctionPtr);
    ASSERT(function != nullptr);
    return function->GetParametersNum();
}

extern "C" ObjectHeader *ReflectMethodGetParametersTypesImpl(EtsLong etsFunctionPtr)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto *function = reinterpret_cast<EtsMethod *>(etsFunctionPtr);
    ASSERT(function != nullptr);
    auto numParams = function->GetParametersNum();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(PlatformTypes(coro)->coreClass, numParams));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(arrayH.GetPtr() != nullptr);

    for (uint32_t idx = 0; idx < numParams; ++idx) {
        // 0 is recevier type
        auto i = function->IsStatic() ? idx : idx + 1;
        auto *resolvedParameterType = function->ResolveArgType(i)->ResolvePublicClass();
        arrayH->Set(idx, resolvedParameterType->AsObject());
    }

    return arrayH->AsObject()->GetCoreType();
}

extern "C" EtsString *ReflectMethodGetNameInternal(EtsLong etsMethodPtr)
{
    auto *method = reinterpret_cast<EtsMethod *>(etsMethodPtr);
    EtsString *name;
    if (method->IsInstanceConstructor()) {
        name = EtsString::CreateFromMUtf8(CONSTRUCTOR_NAME);
    } else {
        name = method->GetNameString();
    }
    if (UNLIKELY(name == nullptr)) {
        [[maybe_unused]] auto *coro = EtsCoroutine::GetCurrent();
        ASSERT(coro != nullptr);
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ASSERT(name != nullptr);
    return name;
}

}  // namespace ark::ets::intrinsics
