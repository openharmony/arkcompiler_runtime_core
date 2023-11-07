/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "runtime/mem/refstorage/reference.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/lambda_utils.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/interop_js/js_job_queue.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "intrinsics.h"

namespace panda::ets::interop::js {
static napi_value ThenCallback(napi_env env, napi_callback_info info)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    auto ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope napi_scope(ctx, env);

    mem::Reference *callback_ref = nullptr;
    [[maybe_unused]] napi_status status =
        napi_get_cb_info(env, info, nullptr, nullptr, nullptr, reinterpret_cast<void **>(&callback_ref));
    ASSERT(status == napi_ok);

    ScopedManagedCodeThread scope(coro);
    [[maybe_unused]] HandleScope<ObjectHeader *> handle_scope(ManagedThread::GetCurrent());
    VMHandle<EtsObject> callback(coro, vm->GetGlobalObjectStorage()->Get(callback_ref));
    vm->GetGlobalObjectStorage()->Remove(callback_ref);

    LambdaUtils::InvokeVoid(coro, callback.GetPtr());
    if (coro->HasPendingException()) {
        napi_throw_error(env, nullptr, "EtsVM internal error");
    }
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

void JsJobQueue::AddJob(EtsObject *callback)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    napi_env env = InteropCtx::Current(coro)->GetJSEnv();
    napi_deferred deferred;
    napi_value undefined;
    napi_value js_promise;
    napi_value then_fn;

    napi_get_undefined(env, &undefined);
    napi_status status = napi_create_promise(env, &deferred, &js_promise);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot allocate a Promise instance");
    }
    status = napi_get_named_property(env, js_promise, "then", &then_fn);
    ASSERT(status == napi_ok);
    (void)status;

    mem::Reference *callback_ref =
        vm->GetGlobalObjectStorage()->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    ASSERT(callback_ref != nullptr);

    napi_value then_callback;
    status = napi_create_function(env, nullptr, 0, ThenCallback, callback_ref, &then_callback);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot create a function");
    }

    napi_value then_promise;
    status = napi_call_function(env, js_promise, then_fn, 1, &then_callback, &then_promise);
    ASSERT(status == napi_ok);
    (void)status;

    napi_resolve_deferred(env, deferred, undefined);
}

static napi_value OnJsPromiseResolved(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    auto ctx = InteropCtx::Current(coro);
    [[maybe_unused]] EtsJSNapiEnvScope napi_scope(ctx, env);

    mem::Reference *promise_ref = nullptr;
    size_t argc = 1;
    napi_value value;
    napi_status status = napi_get_cb_info(env, info, &argc, &value, nullptr, reinterpret_cast<void **>(&promise_ref));
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call napi_get_cb_info!");
    }

    ScopedManagedCodeThread scope(coro);
    EtsHandleScope h_scope(coro);
    EtsHandle<EtsPromise> promise_handle(coro,
                                         EtsPromise::FromCoreType(vm->GetGlobalObjectStorage()->Get(promise_ref)));
    vm->GetGlobalObjectStorage()->Remove(promise_ref);
    ASSERT(promise_handle.GetPtr()->GetEventPtr() != nullptr);
    promise_handle.GetPtr()->GetEventPtr()->SetHappened();

    auto jsval = JSValue::Create(coro, ctx, value);
    panda::ets::intrinsics::EtsPromiseResolve(promise_handle.GetPtr(), jsval->AsObject());

    vm->GetCoroutineManager()->UnblockWaiters(promise_handle.GetPtr()->GetEventPtr());
    vm->GetCoroutineManager()->Schedule();

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

void JsJobQueue::CreatePromiseLink(JSValue *js_promise, EtsPromise *ets_promise)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    PandaEtsVM *vm = coro->GetPandaVM();
    napi_env env = InteropCtx::Current(coro)->GetJSEnv();

    napi_value then_fn;
    napi_status status = napi_get_named_property(env, js_promise->GetNapiValue(env), "then", &then_fn);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call then() from a JS promise");
    }

    mem::Reference *promise_ref = vm->GetGlobalObjectStorage()->Add(ets_promise, mem::Reference::ObjectType::GLOBAL);

    // NOTE(konstanting, #I67QXC): OnJsPromiseRejected
    napi_value then_callback;
    status = napi_create_function(env, nullptr, 0, OnJsPromiseResolved, promise_ref, &then_callback);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot create a function");
    }

    napi_value then_result;
    status = napi_call_function(env, js_promise->GetNapiValue(env), then_fn, 1, &then_callback, &then_result);
    if (status != napi_ok) {
        InteropCtx::Fatal("Cannot call then() from a JS Promise");
    }
}

void JsJobQueue::CreateLink(EtsObject *source, EtsObject *target)
{
    auto add_link_proc = [&]() {
        CreatePromiseLink(JSValue::FromEtsObject(source), EtsPromise::FromEtsObject(target));
    };

    auto *main_t = EtsCoroutine::GetCurrent()->GetPandaVM()->GetCoroutineManager()->GetMainThread();
    Coroutine *main_coro = Coroutine::CastFromThread(main_t);
    if (Coroutine::GetCurrent() != main_coro) {
        // NOTE(konstanting, #I67QXC): figure out if we need to ExecuteOnThisContext() for OHOS
        main_coro->GetContext<StackfulCoroutineContext>()->ExecuteOnThisContext(
            &add_link_proc, EtsCoroutine::GetCurrent()->GetContext<StackfulCoroutineContext>());
    } else {
        add_link_proc();
    }
}
}  // namespace panda::ets::interop::js
