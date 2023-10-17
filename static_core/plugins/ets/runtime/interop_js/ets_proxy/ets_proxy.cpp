/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"

namespace panda::ets::interop::js::ets_proxy {

napi_value GetETSFunction(napi_env env, std::string_view class_descriptor, std::string_view method_name)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope envscope(ctx, env);
    ScopedManagedCodeThread managed_scope(coro);

    EtsClass *ets_class = coro->GetPandaVM()->GetClassLinker()->GetClass(class_descriptor.data());
    if (UNLIKELY(ets_class == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: unresolved class " + std::string(class_descriptor));
        return nullptr;
    }

    EtsMethod *ets_method = ets_class->GetDirectMethod(method_name.data());
    if (UNLIKELY(ets_method == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: class " + std::string(class_descriptor) + " doesn't contain " +
                                          std::string(method_name) + " method");
        return nullptr;
    }

    EtsMethodWrapper *ets_method_wrapper = EtsMethodWrapper::GetFunction(ctx, ets_method);
    if (UNLIKELY(ets_method_wrapper == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: cannot get EtsMethodWrapper, class_descriptor=" +
                                          std::string(class_descriptor) + " method_name=" + std::string(method_name));
        return nullptr;
    }
    return ets_method_wrapper->GetJsValue(env);
}

napi_value GetETSClass(napi_env env, std::string_view class_descriptor)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope envscope(ctx, env);
    ScopedManagedCodeThread managed_scope(coro);

    EtsClass *ets_klass = coro->GetPandaVM()->GetClassLinker()->GetClass(class_descriptor.data());
    if (UNLIKELY(ets_klass == nullptr)) {
        InteropCtx::ThrowJSError(env, "GetETSClass: unresolved klass " + std::string(class_descriptor));
        return nullptr;
    }

    EtsClassWrapper *ets_class_wrapper = EtsClassWrapper::Get(ctx, ets_klass);
    if (UNLIKELY(ets_class_wrapper == nullptr)) {
        return nullptr;
    }

    return ets_class_wrapper->GetJsCtor(env);
}

}  // namespace panda::ets::interop::js::ets_proxy
