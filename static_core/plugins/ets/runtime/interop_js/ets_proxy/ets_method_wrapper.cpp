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

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_method_wrapper.h"

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/js_value_call.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"

#include "runtime/mem/vm_handle-inl.h"

namespace panda::ets::interop::js::ets_proxy {

/*static*/
std::unique_ptr<EtsMethodWrapper> EtsMethodWrapper::CreateMethod(EtsMethod *etsMethod, EtsClassWrapper *owner)
{
    return std::unique_ptr<EtsMethodWrapper>(new EtsMethodWrapper(etsMethod, owner));
}

/*static*/
std::unique_ptr<EtsMethodWrapper> EtsMethodWrapper::CreateFunction(InteropCtx *ctx, EtsMethod *etsMethod)
{
    ASSERT(etsMethod->IsStatic());
    auto env = ctx->GetJSEnv();
    auto wrapper = CreateMethod(etsMethod, nullptr);

    napi_value jsValue;
    NAPI_CHECK_FATAL(napi_create_function(env, wrapper->etsMethod_->GetName(), NAPI_AUTO_LENGTH,
                                          &EtsMethodCallHandler<true, true>, wrapper.get(), &jsValue));
    NAPI_CHECK_FATAL(napi_create_reference(env, jsValue, 1, &wrapper->jsRef_));
    NAPI_CHECK_FATAL(NapiObjectSeal(env, jsValue));

    return wrapper;
}

/*static*/
EtsMethodWrapper *EtsMethodWrapper::GetMethod(InteropCtx *ctx, EtsMethod *etsMethod)
{
    EtsMethodWrappersCache *cache = ctx->GetEtsMethodWrappersCache();
    EtsMethodWrapper *wrapper = cache->Lookup(etsMethod);
    if (LIKELY(wrapper != nullptr)) {
        return wrapper;
    }

    auto owner = ctx->GetEtsClassWrappersCache()->Lookup(etsMethod->GetClass());
    ASSERT(owner != nullptr);

    std::unique_ptr<EtsMethodWrapper> etsMethodWrapper = EtsMethodWrapper::CreateMethod(etsMethod, owner);
    return cache->Insert(etsMethod, std::move(etsMethodWrapper));
}

EtsMethodWrapper *EtsMethodWrapper::GetFunction(InteropCtx *ctx, EtsMethod *etsMethod)
{
    EtsMethodWrappersCache *cache = ctx->GetEtsMethodWrappersCache();
    EtsMethodWrapper *wrapper = cache->Lookup(etsMethod);
    if (LIKELY(wrapper != nullptr)) {
        return wrapper;
    }

    std::unique_ptr<EtsMethodWrapper> etsFuncWrapper = EtsMethodWrapper::CreateFunction(ctx, etsMethod);
    return cache->Insert(etsMethod, std::move(etsFuncWrapper));
}

/* static */
napi_property_descriptor EtsMethodWrapper::MakeNapiProperty(Method *method, LazyEtsMethodWrapperLink *lazyLink)
{
    napi_callback callback {};
    if (method->IsStatic()) {
        callback = EtsMethodWrapper::EtsMethodCallHandler<true, false>;
    } else {
        callback = EtsMethodWrapper::EtsMethodCallHandler<false, false>;
    }

    napi_property_descriptor prop {};
    prop.utf8name = utf::Mutf8AsCString(method->GetName().data);
    prop.method = callback;
    prop.attributes = method->IsStatic() ? EtsClassWrapper::STATIC_METHOD_ATTR : EtsClassWrapper::METHOD_ATTR;
    prop.data = lazyLink;

    return prop;
}

template <bool IS_STATIC, bool IS_FUNC>
/*static*/
napi_value EtsMethodWrapper::EtsMethodCallHandler(napi_env env, napi_callback_info cinfo)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);

    [[maybe_unused]] EtsJSNapiEnvScope envscope(ctx, env);
    size_t argc;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, nullptr, nullptr));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, jsArgs->data(), &jsThis, &data));

    EtsMethodWrapper *_this;  // NOLINT(readability-identifier-naming)

    if constexpr (IS_FUNC) {
        _this = reinterpret_cast<EtsMethodWrapper *>(data);
    } else {
        auto lazyLink = reinterpret_cast<LazyEtsMethodWrapperLink *>(data);
        _this = EtsMethodWrapper::ResolveLazyLink(ctx, *lazyLink);
        if (UNLIKELY(_this == nullptr)) {
            return nullptr;
        }
    }

    Method *method = _this->etsMethod_->GetPandaMethod();

    ScopedManagedCodeThread managedScope(coro);
    if constexpr (IS_STATIC) {
        EtsClass *etsClass = _this->etsMethod_->GetClass();
        if (UNLIKELY(!coro->GetPandaVM()->GetClassLinker()->InitializeClass(coro, etsClass))) {
            ctx->ForwardEtsException(coro);
            return nullptr;
        }
        return EtsCallImplStatic(coro, ctx, method, *jsArgs);
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

    return EtsCallImplInstance(coro, ctx, method, *jsArgs, etsThis);
}

// Explicit instantiation
template napi_value EtsMethodWrapper::EtsMethodCallHandler<false, false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<false, true>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<true, false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<true, true>(napi_env, napi_callback_info);

}  // namespace panda::ets::interop::js::ets_proxy
