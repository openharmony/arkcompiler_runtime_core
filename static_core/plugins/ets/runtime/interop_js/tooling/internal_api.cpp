/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/tooling/internal_api.h"

namespace ark::ets::interop::js {

bool ForEachFrameInUnionStack(const std::function<void(const void *frame, bool isStaticFrame)> &callback)
{
    auto *mutator = Mutator::GetCurrent();
    if (UNLIKELY(mutator == nullptr)) {
        return false;
    }
    auto *execCtx = EtsExecutionContext::GetCurrent();
    if (UNLIKELY(execCtx == nullptr)) {
        return false;
    }
    auto *ctx = interop::js::InteropCtx::Current(execCtx);
    if (UNLIKELY(ctx == nullptr)) {
        return false;
    }
    return ctx->GetOrCreateCallStack().ForEachInteropFrame(callback);
}

bool UnionStackIsEmpty(bool *isEmpty)
{
    auto *mutator = Mutator::GetCurrent();
    if (UNLIKELY(mutator == nullptr)) {
        return false;
    }
    auto *execCtx = EtsExecutionContext::GetCurrent();
    if (UNLIKELY(execCtx == nullptr)) {
        return false;
    }
    auto *ctx = interop::js::InteropCtx::Current(execCtx);
    if (UNLIKELY(ctx == nullptr)) {
        return false;
    }
    *isEmpty = (!ctx->GetInteropHybridStackEnabled()) || (ctx->GetOrCreateCallStack().GetRecords().Size() == 0);
    return true;
}

}  // namespace ark::ets::interop::js
