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

#ifndef PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REMOTE_PROMISE_RESOLVER_H
#define PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REMOTE_PROMISE_RESOLVER_H

#include "runtime/include/external_callback_poster.h"
#include "plugins/ets/runtime/ets_remote_promise_resolver.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include <node_api.h>

namespace ark::ets::interop::js {

/// @brief Class should be used to resolve/reject JS promises from any
class JsRemotePromiseResolver : public RemotePromiseResolver {
public:
    explicit JsRemotePromiseResolver(napi_deferred deferred);
    NO_COPY_SEMANTIC(JsRemotePromiseResolver);
    NO_MOVE_SEMANTIC(JsRemotePromiseResolver);
    ~JsRemotePromiseResolver() override = default;

    void ResolveViaCallback(EtsObject *resolveValue, Action action) override;
    void ResolveInPlace(EtsObject *resolveValue, Action action) override;

private:
    static void ResolveRemotePromise(napi_deferred deferred, EtsObject *resolveValue, Action action);

    napi_deferred deferred_ = nullptr;
    PandaUniquePtr<CallbackPoster> poster_;
};

}  // namespace ark::ets::interop::js

#endif  // PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REMOTE_PROMISE_RESOLVER_H