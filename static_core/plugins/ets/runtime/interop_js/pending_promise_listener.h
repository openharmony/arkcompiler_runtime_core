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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_PENDING_PROMISE_LISTENER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_PENDING_PROMISE_LISTENER_H_

#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "libpandabase/macros.h"
#include <node_api.h>

namespace ark::ets::interop::js {
class PendingPromiseListener final : public PromiseListener {
public:
    explicit PendingPromiseListener(napi_deferred deferred) : deferred_(deferred) {}

    ~PendingPromiseListener() override;

    void OnPromiseStateChanged(EtsHandle<EtsPromise> &promise) override;

    NO_COPY_SEMANTIC(PendingPromiseListener);
    NO_MOVE_SEMANTIC(PendingPromiseListener);

private:
    void OnPromiseStateChangedImpl(EtsHandle<EtsPromise> &promise);

    napi_deferred deferred_;
    bool completed_ = false;
};
}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_PENDING_PROMISE_LISTENER_H_
