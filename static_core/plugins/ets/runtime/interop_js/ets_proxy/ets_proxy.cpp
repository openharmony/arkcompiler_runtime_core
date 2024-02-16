/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

namespace ark::ets::interop::js::ets_proxy {

napi_value GetETSFunction(napi_env env, std::string_view classDescriptor, std::string_view methodName)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro, env);

    EtsClass *etsClass = coro->GetPandaVM()->GetClassLinker()->GetClass(classDescriptor.data());
    if (UNLIKELY(etsClass == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: unresolved class " + std::string(classDescriptor));
        return nullptr;
    }

    EtsMethod *etsMethod = etsClass->GetDirectMethod(methodName.data());
    if (UNLIKELY(etsMethod == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: class " + std::string(classDescriptor) + " doesn't contain " +
                                          std::string(methodName) + " method");
        return nullptr;
    }

    EtsMethodWrapper *etsMethodWrapper = EtsMethodWrapper::GetFunction(ctx, etsMethod);
    if (UNLIKELY(etsMethodWrapper == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: cannot get EtsMethodWrapper, classDescriptor=" +
                                          std::string(classDescriptor) + " methodName=" + std::string(methodName));
        return nullptr;
    }
    return etsMethodWrapper->GetJsValue(env);
}

napi_value GetETSClass(napi_env env, std::string_view classDescriptor)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro, env);

    EtsClass *etsKlass = coro->GetPandaVM()->GetClassLinker()->GetClass(classDescriptor.data());
    if (UNLIKELY(etsKlass == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSClass: unresolved klass " + std::string(classDescriptor));
        return nullptr;
    }

    EtsClassWrapper *etsClassWrapper = EtsClassWrapper::Get(ctx, etsKlass);
    if (UNLIKELY(etsClassWrapper == nullptr)) {
        return nullptr;
    }

    return etsClassWrapper->GetJsCtor(env);
}

}  // namespace ark::ets::interop::js::ets_proxy
