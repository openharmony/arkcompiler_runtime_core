/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H

#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js {

template <bool IN_ETS_MANAGED = false>
class InteropCodeScope {
public:
    explicit InteropCodeScope(EtsCoroutine *coro, char const *descr = nullptr) : coro_(coro)
    {
        ASSERT(coro_ == ManagedThread::GetCurrent());
        auto ctx = InteropCtx::Current(coro_);

        if constexpr (IN_ETS_MANAGED) {
            ASSERT(coro_->IsManagedCode());
        } else if (UNLIKELY(!coro_->IsManagedCode())) {
            coro_->ManagedCodeBegin();
            switched_ = true;
        }
        ctx->CallStack().AllocRecord(coro_->GetCurrentFrame(), descr);
    }

    ~InteropCodeScope()
    {
        auto ctx = InteropCtx::Current(EtsCoroutine::CastFromThread(coro_));

        ctx->CallStack().PopRecord();
        if constexpr (IN_ETS_MANAGED) {
            ASSERT(coro_->IsManagedCode());
        } else if (UNLIKELY(switched_)) {
            coro_->ManagedCodeEnd();
        }
    }

private:
    EtsCoroutine *coro_;
    bool switched_ = false;

    NO_COPY_SEMANTIC(InteropCodeScope);
    NO_MOVE_SEMANTIC(InteropCodeScope);
};

using InteropCodeScopeETS = InteropCodeScope<true>;

class JSNapiEnvScope {
public:
    explicit JSNapiEnvScope(InteropCtx *ctx, napi_env newEnv) : ctx_(ctx)
    {
        saved_ = ctx_->jsEnv_;
        ctx_->SetJSEnv(newEnv);
    }

    ~JSNapiEnvScope()
    {
        ctx_->SetJSEnv(saved_);
    }

private:
    NO_COPY_SEMANTIC(JSNapiEnvScope);
    NO_MOVE_SEMANTIC(JSNapiEnvScope);

    InteropCtx *ctx_ {};
    napi_env saved_ {};
};

class InteropCodeScopeJS {
public:
    InteropCodeScopeJS(EtsCoroutine *coro, napi_env env, char const *descr = nullptr)
        : codeScope_(coro, descr), jsEnvScope_(InteropCtx::Current(coro), env)
    {
    }

    ~InteropCodeScopeJS() = default;

private:
    NO_COPY_SEMANTIC(InteropCodeScopeJS);
    NO_MOVE_SEMANTIC(InteropCodeScopeJS);

    InteropCodeScope<false> codeScope_;
    JSNapiEnvScope jsEnvScope_;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTEROP_CODE_SCOPE_JS(coro, env) InteropCodeScopeJS codeScope(coro, env, __PRETTY_FUNCTION__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTEROP_CODE_SCOPE_ETS(coro) InteropCodeScopeETS codeScope(coro, __PRETTY_FUNCTION__)

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H
