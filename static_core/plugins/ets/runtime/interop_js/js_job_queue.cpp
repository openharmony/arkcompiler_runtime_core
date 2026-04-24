/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/js_job_queue.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include <node_api.h>
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/execution/coroutines/stackful/stackful_coroutine.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/mem/refstorage/reference.h"
#include "intrinsics.h"

namespace ark::ets::interop::js {
static napi_value ThenCallback(napi_env env, napi_callback_info info)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    INTEROP_CODE_SCOPE_JS_TO_ETS(executionCtx);

    JsJobQueue::JsCallback *jsCallback = nullptr;
    [[maybe_unused]] napi_status status =
        napi_get_cb_info(env, info, nullptr, nullptr, nullptr, reinterpret_cast<void **>(&jsCallback));
    ASSERT(status == napi_ok);

    bool hasException = false;
    {
        ScopedManagedCodeThread managedScope(executionCtx->GetMT());
        jsCallback->Run();
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(jsCallback);
        hasException = executionCtx->GetMT()->HasPendingException();
    }
    if (hasException) {
        napi_throw_error(env, nullptr, "EtsVM internal error");
    }
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

void JsJobQueue::Post(EtsObject *callback)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);

    auto *ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();

    napi_deferred deferred;
    napi_value undefined;
    napi_value jsPromise;
    napi_value thenFn;

    napi_get_undefined(env, &undefined);
    napi_status status = napi_create_promise(env, &deferred, &jsPromise);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot allocate a Promise instance");
    }
    status = napi_get_named_property(env, jsPromise, "then", &thenFn);
    ASSERT(status == napi_ok);
    (void)status;

    auto *jsCallback = JsCallback::Create(executionCtx, callback);
    napi_value thenCallback;
    status = napi_create_function(env, nullptr, 0, ThenCallback, jsCallback, &thenCallback);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot create a function");
    }

    napi_value thenPromise;
    status = napi_call_function(env, jsPromise, thenFn, 1, &thenCallback, &thenPromise);
    ASSERT(status == napi_ok);
    (void)status;

    napi_resolve_deferred(env, deferred, undefined);
}

static napi_value OnJsPromiseCompleted(napi_env env, [[maybe_unused]] napi_callback_info info, bool isResolved)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    PandaEtsVM *vm = executionCtx->GetPandaVM();
    auto ctx = InteropCtx::Current(executionCtx);
    INTEROP_CODE_SCOPE_JS_TO_ETS(executionCtx);

    mem::Reference *promiseRef = nullptr;
    size_t argc = 1;
    napi_value value;
    napi_status status = napi_get_cb_info(env, info, &argc, &value, nullptr, reinterpret_cast<void **>(&promiseRef));
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call napi_get_cb_info!");
    }
    ASSERT(promiseRef != nullptr);

    {
        ScopedManagedCodeThread managedScope(executionCtx->GetMT());
        EtsHandleScope hScope(executionCtx);
        EtsHandle<EtsPromise> promiseHandle(executionCtx,
                                            EtsPromise::FromCoreType(vm->GetGlobalObjectStorage()->Get(promiseRef)));
        vm->GetGlobalObjectStorage()->Remove(promiseRef);

        if (isResolved) {
            auto etsVal = JSConvertAny::UnwrapWithNullCheck(ctx, env, value).value();
            ark::ets::intrinsics::EtsPromiseResolve(promiseHandle.GetPtr(), etsVal, ark::ets::ToEtsBoolean(false));
        } else {
            auto refconv = JSRefConvertResolve<true>(ctx, PlatformTypes(executionCtx)->coreError->GetRuntimeClass());
            ASSERT(refconv != nullptr);
            bool isInstanceof = false;
            EtsObject *error = nullptr;
            NAPI_CHECK_FATAL(napi_is_error(env, value, &isInstanceof));
            if (!isInstanceof) {
                auto res = JSConvertESError::UnwrapImpl(ctx, env, value);
                if (LIKELY(res.has_value())) {
                    error = AsEtsObject(res.value());
                }
            } else {
                error = refconv->Unwrap(ctx, value);
            }
            ASSERT(error != nullptr);
            ark::ets::intrinsics::EtsPromiseReject(promiseHandle.GetPtr(), error, ark::ets::ToEtsBoolean(false));
        }
    }

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static napi_value OnJsPromiseResolved(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    return OnJsPromiseCompleted(env, info, true);
}

static napi_value OnJsPromiseRejected(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    return OnJsPromiseCompleted(env, info, false);
}

void JsJobQueue::CreatePromiseLink(EtsObject *jsObject, EtsPromise *etsPromise)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    PandaEtsVM *vm = executionCtx->GetPandaVM();
    InteropCtx *ctx = InteropCtx::Current(executionCtx);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return;
    }
    napi_env env = ctx->GetJSEnv();
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    napi_value jsPromise = storage->GetJsObject(jsObject, env);

    napi_value thenFn;
    napi_status status = napi_get_named_property(env, jsPromise, "then", &thenFn);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call then() from a JS promise");
    }

    mem::Reference *promiseRef =
        vm->GetGlobalObjectStorage()->Add(etsPromise->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
    std::array<napi_value, 2U> thenCallback {};

    status = napi_create_function(env, nullptr, 0, OnJsPromiseResolved, promiseRef, &thenCallback[0]);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot create a function");
    }

    status = napi_create_function(env, nullptr, 0, OnJsPromiseRejected, promiseRef, &thenCallback[1]);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot create a function");
    }

    napi_value thenResult;
    status = napi_call_function(env, jsPromise, thenFn, 2U, thenCallback.data(), &thenResult);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call then() from a JS Promise");
    }
}

void JsJobQueue::CreateLink(EtsObject *source, EtsObject *target)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    INTEROP_CODE_SCOPE_ETS_TO_JS(executionCtx);
    CreatePromiseLink(source, EtsPromise::FromEtsObject(target));
}

/* static */
JsJobQueue::JsCallback *JsJobQueue::JsCallback::Create(EtsExecutionContext *executionCtx, const EtsObject *callback)
{
    auto *refStorage = executionCtx->GetPandaVM()->GetGlobalObjectStorage();
    auto *jsCallbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(jsCallbackRef != nullptr);
    auto *jsCallback = Runtime::GetCurrent()->GetInternalAllocator()->New<JsCallback>(jsCallbackRef);
    return jsCallback;
}

void JsJobQueue::JsCallback::Run()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *refStorage = executionCtx->GetPandaVM()->GetGlobalObjectStorage();
    auto *callback = EtsObject::FromCoreType(refStorage->Get(jsCallbackRef_));
    LambdaUtils::InvokeVoid(executionCtx->GetMT(), callback);
}

JsJobQueue::JsCallback::~JsCallback()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *refStorage = executionCtx->GetPandaVM()->GetGlobalObjectStorage();
    refStorage->Remove(jsCallbackRef_);
}

}  // namespace ark::ets::interop::js
