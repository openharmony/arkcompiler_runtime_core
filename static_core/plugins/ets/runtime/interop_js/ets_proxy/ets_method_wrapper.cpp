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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

#include "runtime/mem/vm_handle-inl.h"

namespace ark::ets::interop::js::ets_proxy {

/*static*/
std::unique_ptr<EtsMethodWrapper> EtsMethodWrapper::CreateMethod(EtsMethodSet *etsMethodSet, EtsClassWrapper *owner)
{
    auto wrapper = new EtsMethodWrapper(etsMethodSet, owner);
    if (UNLIKELY(nullptr == wrapper)) {
        return nullptr;
    }
    return std::unique_ptr<EtsMethodWrapper>(wrapper);
}

/*static*/
EtsMethodWrapper *EtsMethodWrapper::GetMethod(InteropCtx *ctx, EtsMethodSet *etsMethodSet)
{
    EtsMethodWrappersCache *cache = ctx->GetEtsMethodWrappersCache();
    EtsMethodWrapper *wrapper = cache->Lookup(etsMethodSet);
    if (LIKELY(wrapper != nullptr)) {
        return wrapper;
    }

    auto owner = ctx->GetEtsClassWrappersCache()->Lookup(etsMethodSet->GetEnclosingClass());
    ASSERT(owner != nullptr);

    std::unique_ptr<EtsMethodWrapper> etsMethodWrapper = EtsMethodWrapper::CreateMethod(etsMethodSet, owner);
    return cache->Insert(etsMethodSet, std::move(etsMethodWrapper));
}

/* static */
napi_property_descriptor EtsMethodWrapper::MakeNapiProperty(EtsMethodSet *method, LazyEtsMethodWrapperLink *lazyLink)
{
    napi_callback callback {};
    if (method->IsStatic()) {
        callback = EtsMethodWrapper::EtsMethodCallHandler<true>;
    } else {
        callback = EtsMethodWrapper::EtsMethodCallHandler<false>;
    }

    napi_property_descriptor prop {};
    prop.utf8name = method->GetName();
    prop.method = callback;
    prop.attributes = method->IsStatic() ? EtsClassWrapper::STATIC_METHOD_ATTR : EtsClassWrapper::METHOD_ATTR;
    prop.data = lazyLink;

    return prop;
}

static std::tuple<EtsMethod *, const char *> FindSuitableMethod(const EtsMethodSet *methodSet, uint32_t parametersNum)
{
    if (nullptr == methodSet) {
        return {nullptr, "no method found"};
    }

    EtsMethod *etsMethod = methodSet->GetMethod(parametersNum);

    // Methods with matched number of arguments might present in base classes
    while (nullptr == etsMethod && nullptr != methodSet) {
        methodSet = methodSet->GetBaseMethodSet();
        if (nullptr != methodSet) {
            etsMethod = methodSet->GetMethod(parametersNum);
        }
    }

    if (UNLIKELY(nullptr != methodSet && !methodSet->IsValid())) {
        return {nullptr, "method has unsupported overloads"};
    }

    if (UNLIKELY(nullptr == etsMethod)) {
        return {nullptr, "no suitable method found for this number of arguments"};
    }

    return {etsMethod, nullptr};
}

template <bool IS_STATIC>
/*static*/
napi_value EtsMethodWrapper::EtsMethodCallHandler(napi_env env, napi_callback_info cinfo)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);

    INTEROP_CODE_SCOPE_JS(coro, env);
    size_t argc;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, jsArgs->data(), &jsThis, &data));

    EtsMethodWrapper *_this;  // NOLINT(readability-identifier-naming)

    auto lazyLink = reinterpret_cast<LazyEtsMethodWrapperLink *>(data);
    _this = EtsMethodWrapper::ResolveLazyLink(ctx, *lazyLink);
    if (UNLIKELY(_this == nullptr)) {
        return nullptr;
    }

    auto [etsMethod, errorMessage] = FindSuitableMethod(_this->etsMethodSet_, argc);
    ASSERT(nullptr != etsMethod || nullptr != errorMessage);
    if (UNLIKELY(nullptr == etsMethod)) {
        ctx->ThrowJSTypeError(env, errorMessage);
        return nullptr;
    }

    Method *method = etsMethod->GetPandaMethod();

    if constexpr (IS_STATIC) {
        EtsClass *etsClass = etsMethod->GetClass();
        ASSERT(method->GetClass() == etsClass->GetRuntimeClass());
        if (UNLIKELY(!coro->GetPandaVM()->GetClassLinker()->InitializeClass(coro, etsClass))) {
            ctx->ForwardEtsException(coro);
            return nullptr;
        }
        return CallETSStatic(coro, ctx, method, *jsArgs);
    }

    if (UNLIKELY(IsNullOrUndefined(env, jsThis))) {
        ctx->ThrowJSTypeError(env, "ets this in instance method cannot be null or undefined");
        return nullptr;
    }

    EtsObject *etsThis = _this->owner_->UnwrapEtsProxy(ctx, jsThis);
    if (UNLIKELY(etsThis == nullptr)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    return CallETSInstance(coro, ctx, method, *jsArgs, etsThis);
}

// Explicit instantiation
template napi_value EtsMethodWrapper::EtsMethodCallHandler<false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<true>(napi_env, napi_callback_info);

}  // namespace ark::ets::interop::js::ets_proxy
