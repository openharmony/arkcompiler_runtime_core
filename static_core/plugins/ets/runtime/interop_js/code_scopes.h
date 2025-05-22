/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
            ctx->UpdateInteropStackInfoIfNeeded();
        }
        ctx->CallStack().AllocRecord(coro_->GetCurrentFrame(), descr);
    }

    ~InteropCodeScope()
    {
        auto ctx = InteropCtx::Current(EtsCoroutine::CastFromThread(coro_));

        ctx->CallStack().PopRecord();
    }

private:
    EtsCoroutine *coro_;

    NO_COPY_SEMANTIC(InteropCodeScope);
    NO_MOVE_SEMANTIC(InteropCodeScope);
};

using InteropCodeScopeETS = InteropCodeScope<true>;

class InteropCodeScopeJS {
public:
    explicit InteropCodeScopeJS(EtsCoroutine *coro, char const *descr = nullptr) : codeScope_(coro, descr) {}

    ~InteropCodeScopeJS()
    {
        auto ctx = InteropCtx::Current();
        ctx->UpdateInteropStackInfoIfNeeded();
    }

private:
    NO_COPY_SEMANTIC(InteropCodeScopeJS);
    NO_MOVE_SEMANTIC(InteropCodeScopeJS);

    InteropCodeScope<false> codeScope_;
};

// CC-OFFNXT(G.PRE.02-CPP) for readability and ease of use
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTEROP_CODE_SCOPE_JS(coro) InteropCodeScopeJS codeScope(coro, __PRETTY_FUNCTION__)
// CC-OFFNXT(G.PRE.02-CPP) for readability and ease of use
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTEROP_CODE_SCOPE_ETS(coro) InteropCodeScopeETS codeScope(coro, __PRETTY_FUNCTION__)

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_H
