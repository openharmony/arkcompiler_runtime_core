/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_set.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

namespace ark::ets::interop::js::ets_proxy {

napi_value GetETSFunction(napi_env env, std::string_view packageName, std::string_view methodName)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    INTEROP_CODE_SCOPE_JS(coro);

    std::ostringstream classDescriptorBuilder;
    classDescriptorBuilder << "L" << packageName << (packageName.empty() ? "ETSGLOBAL;" : "/ETSGLOBAL;");
    std::string classDescriptor = classDescriptorBuilder.str();

    if (IsEtsGlobalClassName(packageName.data())) {
        // Old-style value for legacy code
        // NOTE remove this check after all tests fixed
        classDescriptor = packageName;
    }

    napi_value jsClass = GetETSClass(env, classDescriptor);
    ASSERT(GetValueType(env, jsClass) == napi_function);

    napi_value jsMethod;
    const napi_status resolveStatus = napi_get_named_property(env, jsClass, methodName.data(), &jsMethod);
    if (UNLIKELY(napi_ok != resolveStatus || GetValueType(env, jsMethod) != napi_function)) {
        InteropCtx::ThrowJSError(env, "GetETSFunction: class " + std::string(classDescriptor) + " doesn't contain " +
                                          std::string(methodName) + " method");
        return nullptr;
    }
    NAPI_CHECK_FATAL(napi_object_seal(env, jsMethod));

    return jsMethod;
}

napi_value GetETSClass(napi_env env, std::string_view classDescriptor)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro);
    ScopedManagedCodeThread managedScope(coro);

    EtsClass *etsKlass = coro->GetPandaVM()->GetClassLinker()->GetClass(classDescriptor.data(), true, ctx->LinkerCtx());
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
