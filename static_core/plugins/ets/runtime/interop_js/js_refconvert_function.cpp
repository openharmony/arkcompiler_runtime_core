/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/interop_js/js_proxy/js_proxy.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/types/ets_type.h"
namespace ark::ets::interop::js {

napi_value EtsLambdaProxyInvoke(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }
    INTEROP_CODE_SCOPE_JS(coro);

    size_t argc;
    napi_value athis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, &athis, &data));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), &athis, &data));

    // Atomic with acquire order reason: load visibility after shared reference initialization
    auto *sharedRef = AtomicLoad(static_cast<ets_proxy::SharedReference **>(data), std::memory_order_acquire);
    ASSERT(sharedRef != nullptr);

    ScopedManagedCodeThread managedScope(coro);
    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);
    EtsMethod *method = etsThis->GetClass()->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    ASSERT(method != nullptr);

    return CallETSInstance(coro, ctx, method->GetPandaMethod(), *jsArgs, etsThis);
}

napi_value JSRefConvertFunction::WrapImpl(InteropCtx *ctx, EtsObject *obj)
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(ctx == InteropCtx::Current(coro));
    auto env = ctx->GetJSEnv();

    ASSERT(obj->GetClass() == klass_);
    [[maybe_unused]] EtsMethod *method = klass_->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    method = method == nullptr ? klass_->GetStaticMethod(INVOKE_METHOD_NAME, nullptr) : method;
    ASSERT(method != nullptr);

    JSValue *jsValue;
    {
        NapiScope jsHandleScope(env);

        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (LIKELY(storage->HasReference(obj, env))) {
            jsValue = JSValue::CreateRefValue(coro, ctx, storage->GetJsObject(obj, env), napi_function);
        } else {
            napi_value jsFn;
            auto preInitCallback = [&env, &jsFn](ets_proxy::SharedReference **uninitializedRef) {
                ASSERT(uninitializedRef != nullptr);
                NAPI_CHECK_FATAL(napi_create_function(env, ark::ets::INVOKE_METHOD_NAME, NAPI_AUTO_LENGTH,
                                                      EtsLambdaProxyInvoke, uninitializedRef, &jsFn));
                return jsFn;
            };
            ets_proxy::SharedReference *sharedRef = storage->CreateETSObjectRef(ctx, obj, jsFn, preInitCallback);
            if (UNLIKELY(sharedRef == nullptr)) {
                ASSERT(InteropCtx::SanityJSExceptionPending());
                return nullptr;
            }
            jsValue = JSValue::CreateRefValue(coro, ctx, jsFn, napi_function);
        }
    }
    return jsValue->GetRefValue(env);
}

EtsObject *JSRefConvertFunction::UnwrapImpl(InteropCtx *ctx, napi_value jsFun)
{
    // Check if object has SharedReference
    ets_proxy::SharedReference *sharedRef = ctx->GetSharedRefStorage()->GetReference(ctx->GetJSEnv(), jsFun);
    if (LIKELY(sharedRef != nullptr)) {
        EtsObject *jsFunctionProxy = sharedRef->GetEtsObject();
        return jsFunctionProxy;
    }

    return this->CreateJSFunctionProxy(ctx, jsFun);
}

void JSRefConvertFunction::LazyInitJsFunctionProxyWrapper(InteropCtx *ctx)
{
    // register the function interface
    auto etsClass = EtsClass::FromRuntimeClass(this->klass_->GetRuntimeClass());

    // create a JSProxy wrapper for the function
    this->jsFunctionProxyWrapper_ = js_proxy::JSProxy::CreateFunctionProxy(etsClass);
    ctx->SetJsProxyInstance(etsClass, this->jsFunctionProxyWrapper_);
}

EtsObject *JSRefConvertFunction::CreateJSFunctionProxy(InteropCtx *ctx, napi_value jsFun)
{
    // lazy init the function proxy wrapper
    this->LazyInitJsFunctionProxyWrapper(ctx);

    // JS Function => ETS Function Object
    ASSERT(this->jsFunctionProxyWrapper_ != nullptr);
    auto *storage = ctx->GetSharedRefStorage();
    ASSERT(storage->GetReference(ctx->GetJSEnv(), jsFun) == nullptr);

    EtsObject *etsObject = EtsObject::Create(jsFunctionProxyWrapper_->GetProxyClass());
    if (UNLIKELY(etsObject == nullptr)) {
        ctx->ForwardEtsException(EtsCoroutine::GetCurrent());
        return nullptr;
    }

    auto *sharedRef = storage->CreateJSObjectRef(ctx, etsObject, jsFun);
    if (UNLIKELY(sharedRef == nullptr)) {
        ASSERT(InteropCtx::SanityJSExceptionPending());
        return nullptr;
    }
    return sharedRef->GetEtsObject();  // fetch again after gc
}
}  // namespace ark::ets::interop::js
