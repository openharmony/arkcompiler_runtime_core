/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_NAPI_ENV_SCOPE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_NAPI_ENV_SCOPE_H

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include <node_api.h>

namespace panda::ets::interop::js {

class EtsJSNapiEnvScope {
public:
    explicit EtsJSNapiEnvScope(InteropCtx *ctx, napi_env newEnv) : ctx_(ctx)
    {
        saved_ = ctx_->jsEnv_;
        ctx_->SetJSEnv(newEnv);
    }

    ~EtsJSNapiEnvScope()
    {
        ctx_->SetJSEnv(saved_);
    }

private:
    NO_COPY_SEMANTIC(EtsJSNapiEnvScope);
    NO_MOVE_SEMANTIC(EtsJSNapiEnvScope);

    InteropCtx *ctx_ {};
    napi_env saved_ {};
};

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_NAPI_ENV_SCOPE_H
