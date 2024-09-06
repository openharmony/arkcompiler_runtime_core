/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <node_api.h>
#include "runtime/handle_scope-inl.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/mem/refstorage/reference.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/interop_js/js_job_queue.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "intrinsics.h"

namespace ark::ets::interop::js {
static napi_value ThenCallback(napi_env env, napi_callback_info info)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    INTEROP_CODE_SCOPE_JS(coro, env);

    JsJobQueue::JsCallback *jsCallback = nullptr;
    [[maybe_unused]] napi_status status =
        napi_get_cb_info(env, info, nullptr, nullptr, nullptr, reinterpret_cast<void **>(&jsCallback));
    ASSERT(status == napi_ok);

    jsCallback->Run();
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(jsCallback);

    if (coro->HasPendingException()) {
        napi_throw_error(env, nullptr, "EtsVM internal error");
    }
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

void JsJobQueue::Post(EtsObject *callback)
{
    auto postProc = [callback] {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        napi_env env = InteropCtx::Current(coro)->GetJSEnv();
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

        auto *jsCallback = JsCallback::Create(coro, callback);
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
    };

    auto *mainT = EtsCoroutine::GetCurrent()->GetPandaVM()->GetCoroutineManager()->GetMainThread();
    Coroutine *mainCoro = Coroutine::CastFromThread(mainT);
    if (Coroutine::GetCurrent() != mainCoro) {
        // NOTE(konstanting, #I67QXC): figure out if we need to ExecuteOnThisContext() for OHOS
        mainCoro->GetContext<StackfulCoroutineContext>()->ExecuteOnThisContext(
            &postProc, EtsCoroutine::GetCurrent()->GetContext<StackfulCoroutineContext>());
    } else {
        postProc();
    }
}

static napi_value OnJsPromiseResolved(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro, env);

    mem::Reference *promiseRef = nullptr;
    size_t argc = 1;
    napi_value value;
    napi_status status = napi_get_cb_info(env, info, &argc, &value, nullptr, reinterpret_cast<void **>(&promiseRef));
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call napi_get_cb_info!");
    }

    EtsHandleScope hScope(coro);
    EtsHandle<EtsPromise> promiseHandle(coro, EtsPromise::FromCoreType(vm->GetGlobalObjectStorage()->Get(promiseRef)));
    vm->GetGlobalObjectStorage()->Remove(promiseRef);

    auto jsval = JSValue::Create(coro, ctx, value);
    ark::ets::intrinsics::EtsPromiseResolve(promiseHandle.GetPtr(), jsval->AsObject());

    vm->GetCoroutineManager()->Schedule();

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

void JsJobQueue::CreatePromiseLink(EtsObject *jsObject, EtsPromise *etsPromise)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    InteropCtx *ctx = InteropCtx::Current(coro);
    napi_env env = ctx->GetJSEnv();
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    napi_value jsPromise = storage->GetReference(jsObject)->GetJsObject(env);

    napi_value thenFn;
    napi_status status = napi_get_named_property(env, jsPromise, "then", &thenFn);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call then() from a JS promise");
    }

    mem::Reference *promiseRef = vm->GetGlobalObjectStorage()->Add(etsPromise, mem::Reference::ObjectType::GLOBAL);

    // NOTE(konstanting, #I67QXC): OnJsPromiseRejected
    napi_value thenCallback;
    status = napi_create_function(env, nullptr, 0, OnJsPromiseResolved, promiseRef, &thenCallback);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot create a function");
    }

    napi_value thenResult;
    status = napi_call_function(env, jsPromise, thenFn, 1, &thenCallback, &thenResult);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call then() from a JS Promise");
    }
}

void JsJobQueue::CreateLink(EtsObject *source, EtsObject *target)
{
    auto addLinkProc = [&]() { CreatePromiseLink(source, EtsPromise::FromEtsObject(target)); };

    auto *mainT = EtsCoroutine::GetCurrent()->GetPandaVM()->GetCoroutineManager()->GetMainThread();
    Coroutine *mainCoro = Coroutine::CastFromThread(mainT);
    if (Coroutine::GetCurrent() != mainCoro) {
        // NOTE(konstanting, #I67QXC): figure out if we need to ExecuteOnThisContext() for OHOS
        mainCoro->GetContext<StackfulCoroutineContext>()->ExecuteOnThisContext(
            &addLinkProc, EtsCoroutine::GetCurrent()->GetContext<StackfulCoroutineContext>());
    } else {
        addLinkProc();
    }
}

/* static */
JsJobQueue::JsCallback *JsJobQueue::JsCallback::Create(EtsCoroutine *coro, const EtsObject *callback)
{
    auto *refStorage = coro->GetPandaVM()->GetGlobalObjectStorage();
    auto *jsCallbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(jsCallbackRef != nullptr);
    auto *jsCallback = Runtime::GetCurrent()->GetInternalAllocator()->New<JsCallback>(jsCallbackRef);
    return jsCallback;
}

void JsJobQueue::JsCallback::Run()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *refStorage = coro->GetPandaVM()->GetGlobalObjectStorage();
    auto *callback = EtsObject::FromCoreType(refStorage->Get(jsCallbackRef_));
    LambdaUtils::InvokeVoid(coro, callback);
}

JsJobQueue::JsCallback::~JsCallback()
{
    auto *refStorage = EtsCoroutine::GetCurrent()->GetPandaVM()->GetGlobalObjectStorage();
    refStorage->Remove(jsCallbackRef_);
}

}  // namespace ark::ets::interop::js
