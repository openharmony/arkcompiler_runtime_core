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
std::unique_ptr<EtsMethodWrapper> EtsMethodWrapper::CreateMethod(EtsMethod *ets_method, EtsClassWrapper *owner)
{
    return std::unique_ptr<EtsMethodWrapper>(new EtsMethodWrapper(ets_method, owner));
}

/*static*/
std::unique_ptr<EtsMethodWrapper> EtsMethodWrapper::CreateFunction(InteropCtx *ctx, EtsMethod *ets_method)
{
    ASSERT(ets_method->IsStatic());
    auto env = ctx->GetJSEnv();
    auto wrapper = CreateMethod(ets_method, nullptr);

    napi_value js_value;
    NAPI_CHECK_FATAL(napi_create_function(env, wrapper->ets_method_->GetName(), NAPI_AUTO_LENGTH,
                                          &EtsMethodCallHandler</*IS_STATIC=*/true, /*IS_FUNC=*/true>, wrapper.get(),
                                          &js_value));
    NAPI_CHECK_FATAL(napi_create_reference(env, js_value, 1, &wrapper->js_ref_));
    NAPI_CHECK_FATAL(NapiObjectSeal(env, js_value));

    return wrapper;
}

/*static*/
EtsMethodWrapper *EtsMethodWrapper::GetMethod(InteropCtx *ctx, EtsMethod *ets_method)
{
    EtsMethodWrappersCache *cache = ctx->GetEtsMethodWrappersCache();
    EtsMethodWrapper *wrapper = cache->Lookup(ets_method);
    if (LIKELY(wrapper != nullptr)) {
        return wrapper;
    }

    auto owner = ctx->GetEtsClassWrappersCache()->Lookup(ets_method->GetClass());
    ASSERT(owner != nullptr);

    std::unique_ptr<EtsMethodWrapper> ets_method_wrapper = EtsMethodWrapper::CreateMethod(ets_method, owner);
    return cache->Insert(ets_method, std::move(ets_method_wrapper));
}

EtsMethodWrapper *EtsMethodWrapper::GetFunction(InteropCtx *ctx, EtsMethod *ets_method)
{
    EtsMethodWrappersCache *cache = ctx->GetEtsMethodWrappersCache();
    EtsMethodWrapper *wrapper = cache->Lookup(ets_method);
    if (LIKELY(wrapper != nullptr)) {
        return wrapper;
    }

    std::unique_ptr<EtsMethodWrapper> ets_func_wrapper = EtsMethodWrapper::CreateFunction(ctx, ets_method);
    return cache->Insert(ets_method, std::move(ets_func_wrapper));
}

/* static */
napi_property_descriptor EtsMethodWrapper::MakeNapiProperty(Method *method, LazyEtsMethodWrapperLink *lazy_link)
{
    napi_callback callback {};
    if (method->IsStatic()) {
        callback = EtsMethodWrapper::EtsMethodCallHandler</*IS_STATIC=*/true, /*IS_FUNC=*/false>;
    } else {
        callback = EtsMethodWrapper::EtsMethodCallHandler</*IS_STATIC=*/false, /*IS_FUNC=*/false>;
    }

    napi_property_descriptor prop {};
    prop.utf8name = utf::Mutf8AsCString(method->GetName().data);
    prop.method = callback;
    prop.attributes = method->IsStatic() ? EtsClassWrapper::STATIC_METHOD_ATTR : EtsClassWrapper::METHOD_ATTR;
    prop.data = lazy_link;

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
    napi_value js_this;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, nullptr, nullptr, nullptr));
    auto js_args = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cinfo, &argc, js_args->data(), &js_this, &data));

    EtsMethodWrapper *_this;  // NOLINT(readability-identifier-naming)

    if constexpr (IS_FUNC) {
        _this = reinterpret_cast<EtsMethodWrapper *>(data);
    } else {
        auto lazy_link = reinterpret_cast<LazyEtsMethodWrapperLink *>(data);
        _this = EtsMethodWrapper::ResolveLazyLink(ctx, *lazy_link);
        if (UNLIKELY(_this == nullptr)) {
            return nullptr;
        }
    }

    Method *method = _this->ets_method_->GetPandaMethod();

    ScopedManagedCodeThread managed_scope(coro);
    if constexpr (IS_STATIC) {
        EtsClass *ets_class = _this->ets_method_->GetClass();
        if (UNLIKELY(!coro->GetPandaVM()->GetClassLinker()->InitializeClass(coro, ets_class))) {
            ctx->ForwardEtsException(coro);
            return nullptr;
        }
        return EtsCallImplStatic(coro, ctx, method, *js_args);
    }

    if (UNLIKELY(IsNullOrUndefined(env, js_this))) {
        ctx->ThrowJSTypeError(env, "ets this in instance method cannot be null or undefined");
        return nullptr;
    }

    EtsObject *ets_this = _this->owner_->UnwrapEtsProxy(ctx, js_this);
    if (UNLIKELY(ets_this == nullptr)) {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    return EtsCallImplInstance(coro, ctx, method, *js_args, ets_this);
}

// Explicit instantiation
template napi_value EtsMethodWrapper::EtsMethodCallHandler<false, false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<false, true>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<true, false>(napi_env, napi_callback_info);
template napi_value EtsMethodWrapper::EtsMethodCallHandler<true, true>(napi_env, napi_callback_info);

}  // namespace panda::ets::interop::js::ets_proxy
