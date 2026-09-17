/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_INL_H

#include "ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js {

inline InteropCtx *GetInteropScopeCtx(EtsExecutionContext *executionCtx)
{
    if (UNLIKELY(executionCtx == nullptr || executionCtx->GetMT() != ManagedThread::GetCurrent())) {
        return nullptr;
    }
    return InteropCtx::Current(executionCtx);
}

inline void AllocInteropRecord(InteropCtx *ctx, char const *descr)
{
    if (ctx->GetInteropHybridStackEnabled()) {
        auto &callStack = ctx->GetOrCreateCallStack();
        callStack.AllocRecord(callStack.GetDynamicTopFrameSP(), callStack.GetStaticTopFrame(), descr);
    }
}

inline void PopInteropRecord(InteropCtx *ctx)
{
    if (ctx->GetInteropHybridStackEnabled()) {
        ctx->GetOrCreateCallStack().PopRecord();
    }
}

inline void UpdateStackInfoAndPopRecord(InteropCtx *ctx, bool recordStack)
{
    ctx->UpdateInteropStackInfoIfNeeded();
    if (recordStack) {
        PopInteropRecord(ctx);
    }
}

template <InteropScopeKind KIND>
inline bool OpenInteropCodeScope(EtsExecutionContext *executionCtx, char const *descr, bool recordStack = true)
{
    auto *ctx = GetInteropScopeCtx(executionCtx);
    if (UNLIKELY(ctx == nullptr)) {
        return false;
    }
    if constexpr (KIND == InteropScopeKind::ETS_TO_JS) {
        return ctx->PushAndUpdateInteropStackInfoIfNeeded(executionCtx);
    } else if (recordStack) {
        AllocInteropRecord(ctx, descr);
    }
    return true;
}

template <InteropScopeKind KIND>
inline bool CloseInteropCodeScope(EtsExecutionContext *executionCtx, bool recordStack = true)
{
    auto *ctx = GetInteropScopeCtx(executionCtx);
    if (UNLIKELY(ctx == nullptr)) {
        return false;
    }
    if constexpr (KIND == InteropScopeKind::JS_TO_ETS) {
        UpdateStackInfoAndPopRecord(ctx, recordStack);
    }
    return true;
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_INL_H
