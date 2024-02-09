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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/pending_promise_listener.h"
#include "runtime/coroutines/stackful_coroutine.h"

namespace ark::ets::interop::js {
PendingPromiseListener::~PendingPromiseListener()
{
    if (!completed_) {
        // This situation may lead to leaks on JS VM side.
        LOG(ERROR, ETS) << "Not completed ETS promise is unreachable";
    }
}

void PendingPromiseListener::OnPromiseStateChanged(EtsHandle<EtsPromise> &promise)
{
    auto *main = EtsCoroutine::GetCurrent()->GetPandaVM()->GetCoroutineManager()->GetMainThread();
    auto *mainCoro = static_cast<EtsCoroutine *>(main);
    auto *curCtx = EtsCoroutine::GetCurrent()->GetContext<StackfulCoroutineContext>();

    auto onChangedProc = [&]() { OnPromiseStateChangedImpl(promise); };

    if (EtsCoroutine::GetCurrent() != mainCoro) {
        // NOTE(konstanting, #I67QXC): figure out if we need to ExecuteOnThisContext() for OHOS
        mainCoro->GetContext<StackfulCoroutineContext>()->ExecuteOnThisContext(&onChangedProc, curCtx);
    } else {
        onChangedProc();
    }
}

void PendingPromiseListener::OnPromiseStateChangedImpl(EtsHandle<EtsPromise> &promise)
{
    ASSERT(promise->GetState() != EtsPromise::STATE_PENDING);
    auto *coro = EtsCoroutine::GetCurrent();
    auto *ctx = InteropCtx::Current(coro);
    EtsObject *value = promise->GetValue(coro);
    napi_env env = ctx->GetJSEnv();
    napi_value completionValue;
    if (value == nullptr) {
        napi_get_null(env, &completionValue);
    } else if (value->GetClass()->IsStringClass()) {
        completionValue = JSConvertString::Wrap(env, reinterpret_cast<EtsString *>(value));
    } else if (value->GetClass() == EtsClass::FromRuntimeClass(ctx->GetPromiseClass())) {
        completionValue = JSConvertPromise::Wrap(env, reinterpret_cast<EtsPromise *>(value));
    } else if (value->GetClass() == EtsClass::FromRuntimeClass(ctx->GetJSValueClass())) {
        completionValue = JSConvertJSValue::Wrap(env, reinterpret_cast<JSValue *>(value));
    } else {
        auto refconv = JSRefConvertResolve(ctx, value->GetClass()->GetRuntimeClass());
        completionValue = refconv->Wrap(ctx, value);
    }
    napi_status status;
    if (promise->IsResolved()) {
        status = napi_resolve_deferred(env, deferred_, completionValue);
    } else {
        status = napi_reject_deferred(env, deferred_, completionValue);
    }
    if (status == napi_ok) {
        completed_ = true;
    } else {
        napi_throw_error(env, nullptr, "Cannot resolve promise");
    }
}

}  // namespace ark::ets::interop::js
