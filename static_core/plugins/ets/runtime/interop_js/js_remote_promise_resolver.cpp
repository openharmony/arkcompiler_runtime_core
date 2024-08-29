/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/js_remote_promise_resolver.h"

namespace ark::ets::interop::js {

JsRemotePromiseResolver::JsRemotePromiseResolver(napi_deferred deferred, PandaUniquePtr<CallbackPoster> &&poster)
    : deferred_(deferred), poster_(std::move(poster))
{
}

void JsRemotePromiseResolver::ResolveViaCallback(EtsObject *resolveValue, Action action)
{
    [[maybe_unused]] auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != coro->GetPandaVM()->GetCoroutineManager()->GetMainThread());
    // post async call to event loop
    poster_->Post(ResolveRemotePromise, deferred_, resolveValue, action);
}

void JsRemotePromiseResolver::ResolveInPlace(EtsObject *resolveValue, Action action)
{
    [[maybe_unused]] auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro == coro->GetPandaVM()->GetCoroutineManager()->GetMainThread());
    // running in place
    ResolveRemotePromise(deferred_, resolveValue, action);
}

/*static*/
void JsRemotePromiseResolver::ResolveRemotePromise(napi_deferred deferred, EtsObject *resolveValue, Action action)
{
    napi_env env = InteropCtx::Current()->GetJsEnvForEventLoopCallbacks();
    INTEROP_CODE_SCOPE_JS(EtsCoroutine::GetCurrent(), env);
    switch (action) {
        case Action::RESOLVE: {
            PromiseInteropResolve(resolveValue, reinterpret_cast<EtsLong>(deferred));
            break;
        }
        case Action::REJECT: {
            PromiseInteropReject(resolveValue, reinterpret_cast<EtsLong>(deferred));
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

}  // namespace ark::ets::interop::js
