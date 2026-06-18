/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/interop_js/js_convert.h"

#include "runtime/mem/vm_handle-inl.h"

namespace ark::ets::interop::js::ets_proxy {

namespace {

EtsMethod *FindMatchingMethod(napi_env env, const PandaVector<EtsMethod *> &bucket, uint32_t argc,
                              Span<napi_value> jsArgs)
{
    for (auto *method : bucket) {
        if (EtsMethodWrapper::MatchesSignature(env, method, argc, jsArgs)) {
            return method;
        }
        bool isExceptionPending = false;
        NAPI_CHECK_FATAL(napi_is_exception_pending(env, &isExceptionPending));
        if (UNLIKELY(isExceptionPending)) {
            return nullptr;
        }
    }
    return nullptr;
}

napi_value InvokeETSInstance(napi_env env, EtsMethod *etsMethod, Span<napi_value> jsArgs, EtsObject *etsThis)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    Method *method = etsMethod->GetPandaMethod();
    uint32_t argc = jsArgs.size();
    uint32_t actualArgNum = etsMethod->GetParametersNum();
    if (actualArgNum > argc && !method->HasVarArgs()) {
        auto newJsArgs = ctx->GetTempArgs<napi_value>(actualArgNum);
        std::copy(jsArgs.begin(), jsArgs.end(), newJsArgs->begin());
        napi_value result;
        napi_get_undefined(env, &result);
        std::fill(newJsArgs->begin() + argc, newJsArgs->end(), result);
        return CallETSInstance(executionCtx, ctx, method, *newJsArgs, etsThis);
    }
    return CallETSInstance(executionCtx, ctx, method, jsArgs, etsThis);
}

}  // namespace

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
bool EtsMethodWrapper::MatchesSignature(napi_env env, EtsMethod *method, uint32_t argc, Span<napi_value> jsArgs)
{
    uint32_t minArgs = method->TryGetMinArgCount().value_or(method->GetParametersNum());
    uint32_t maxArgs = method->GetParametersNum();
    bool hasVarargs = method->GetPandaMethod()->HasVarArgs();
    if (argc < minArgs || (argc > maxArgs && !hasVarargs)) {
        return false;
    }
    uint32_t argsToMatch = std::min(argc, maxArgs);
    uint32_t methodArgOffset = method->IsStatic() ? 0 : 1;
    for (uint32_t i = 0; i < argsToMatch; ++i) {
        if (!CheckArgMatch(env, method, i + methodArgOffset, jsArgs[i])) {
            return false;
        }
    }
    return true;
}

/*static*/
std::tuple<EtsMethod *, const char *> EtsMethodWrapper::FindSuitableMethod(napi_env env, const EtsMethodSet *methodSet,
                                                                           uint32_t argc, Span<napi_value> jsArgs)
{
    if (nullptr == methodSet) {
        return {nullptr, "no method found"};
    }

    auto currentSet = methodSet;
    while (nullptr != currentSet) {
        auto &bucket = currentSet->GetMethods(argc);
        if (bucket.size() == 1) {
            return {bucket[0], nullptr};
        }
        auto *matched = FindMatchingMethod(env, bucket, argc, jsArgs);
        if (matched != nullptr) {
            return {matched, nullptr};
        }
        bool isExceptionPending = false;
        NAPI_CHECK_FATAL(napi_is_exception_pending(env, &isExceptionPending));
        if (UNLIKELY(isExceptionPending)) {
            return {nullptr, nullptr};
        }
        currentSet = currentSet->GetBaseMethodSet();
    }

    return {nullptr, "no suitable method found for these arguments"};
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
napi_property_descriptor EtsMethodWrapper::MakeNapiIteratorProperty(napi_value &iterator, EtsMethodSet *method,
                                                                    LazyEtsMethodWrapperLink *lazyLink)
{
    napi_property_descriptor prop = MakeNapiProperty(method, lazyLink);
    prop.utf8name = nullptr;
    prop.name = iterator;
    return prop;
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

void EtsMethodWrapper::AttachGetterSetterToProperty(EtsMethodSet *method, napi_property_descriptor &property)
{
    napi_callback callback {};
    if (method->IsStatic()) {
        callback = EtsMethodWrapper::GetterSetterCallHandler<true>;
    } else {
        callback = EtsMethodWrapper::GetterSetterCallHandler<false>;
    }
    auto fieldWrapper = reinterpret_cast<EtsFieldWrapper *>(property.data);
    if (method->IsGetter()) {
        property.getter = callback;
        fieldWrapper->SetGetterMethod(method);
    } else {
        property.setter = callback;
        fieldWrapper->SetSetterMethod(method);
    }
}

template <bool IS_STATIC>
/*static*/
napi_value EtsMethodWrapper::GetterSetterCallHandler(napi_env env, napi_callback_info cinfo)
{
    FindMethodFunc findMethodFunc = [](napi_env jsEnv, void *data, size_t argc, Span<napi_value> jsArgs) {
        auto fieldWrapper = reinterpret_cast<EtsFieldWrapper *>(data);
        if (fieldWrapper == nullptr) {
            return EtsMethodAndClassWrapper(nullptr, "method didn't bind a EtsFieldWrapper", nullptr);
        }
        auto etsMethodSet = fieldWrapper->GetGetterSetterMethod(argc);
        if (etsMethodSet == nullptr) {
            return EtsMethodAndClassWrapper(nullptr, "no getter/setter found", nullptr);
        }
        auto result = FindSuitableMethod(jsEnv, etsMethodSet, argc, jsArgs);
        return EtsMethodAndClassWrapper(std::get<0>(result), std::get<1>(result), fieldWrapper->GetOwner());
    };

    return EtsMethodWrapper::DoEtsMethodCall<IS_STATIC>(env, cinfo, findMethodFunc);
}

template <bool IS_STATIC>
/*static*/
napi_value EtsMethodWrapper::DoEtsMethodCall(napi_env env, napi_callback_info cinfo, FindMethodFunc &findMethodFunc)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    INTEROP_CODE_SCOPE_JS_TO_ETS(executionCtx);
    size_t argc;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, jsArgs->data(), &jsThis, &data));
    ScopedManagedCodeThread managedScope(executionCtx->GetMT());
    auto [etsMethod, errorMessage, etsClassWrapper] =
        findMethodFunc(env, data, argc, Span<napi_value>(jsArgs->data(), argc));

    bool isExceptionPending = false;
    NAPI_CHECK_FATAL(napi_is_exception_pending(env, &isExceptionPending));
    ASSERT(isExceptionPending || nullptr != etsMethod || nullptr != errorMessage || etsClassWrapper != nullptr);
    if (UNLIKELY(nullptr == etsMethod || nullptr == etsClassWrapper)) {
        if (!isExceptionPending && errorMessage != nullptr) {
            ctx->ThrowJSTypeError(env, errorMessage);
        }
        return nullptr;
    }

    Method *method = etsMethod->GetPandaMethod();
    if constexpr (IS_STATIC) {
        EtsClass *etsClass = etsMethod->GetClass();
        ASSERT(method->GetClass() == etsClass->GetRuntimeClass());
        if (UNLIKELY(!executionCtx->GetPandaVM()->GetClassLinker()->InitializeClass(executionCtx, etsClass))) {
            ctx->ForwardEtsException(executionCtx);
            return nullptr;
        }
        return CallETSStatic(executionCtx, ctx, method, *jsArgs);
    }

    EtsObject *etsThis = etsClassWrapper->UnwrapEtsProxy(ctx, jsThis);
    if (UNLIKELY(etsThis == nullptr)) {
        if (executionCtx->GetMT()->HasPendingException()) {
            ctx->ForwardEtsException(executionCtx);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }
    return InvokeETSInstance(env, etsMethod, *jsArgs, etsThis);
}

template <bool IS_STATIC>
/*static*/
napi_value EtsMethodWrapper::EtsMethodCallHandler(napi_env env, napi_callback_info cinfo)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);

    FindMethodFunc findMethodFunc = [&](napi_env jsEnv, void *data, size_t argc, Span<napi_value> jsArgs) {
        auto lazyLink = reinterpret_cast<LazyEtsMethodWrapperLink *>(data);
        auto methodWrapper = EtsMethodWrapper::ResolveLazyLink(ctx, *lazyLink);
        if (methodWrapper == nullptr) {
            return EtsMethodAndClassWrapper(nullptr, "method didn't bind a LazyEtsMethodWrapperLink", nullptr);
        }
        auto result = FindSuitableMethod(jsEnv, methodWrapper->etsMethodSet_, argc, jsArgs);
        auto etsMethod = std::get<0>(result);
        auto errorMessage = std::get<1>(result);
        auto classWrapper = methodWrapper->owner_;
        if (etsMethod == nullptr) {
            return EtsMethodAndClassWrapper(nullptr, errorMessage, nullptr);
        }
        return EtsMethodAndClassWrapper(etsMethod, errorMessage, classWrapper);
    };

    return EtsMethodWrapper::DoEtsMethodCall<IS_STATIC>(env, cinfo, findMethodFunc);
}

// Explicit instantiation
template napi_value EtsMethodWrapper::EtsMethodCallHandler<false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<true>(napi_env, napi_callback_info);

// Explicit instantiation
template napi_value EtsMethodWrapper::GetterSetterCallHandler<false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::GetterSetterCallHandler<true>(napi_env, napi_callback_info);

}  // namespace ark::ets::interop::js::ets_proxy
