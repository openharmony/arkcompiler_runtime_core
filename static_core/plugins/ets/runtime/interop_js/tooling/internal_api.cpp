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
#include "runtime/tooling/debugger.h"

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

bool IsHybridStackEnabled()
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
    bool isEnabled = ctx->GetInteropHybridStackEnabled();
    if (isEnabled) {
        ctx->GetOrCreateCallStack();
    }
    return isEnabled;
}

bool GetStaticFrameInfo(const void *frame, arkplatform::HybridFrameInfo &frameInfo)
{
    auto *debugFrame = static_cast<const tooling::PtDebugFrame *>(frame);
    if (debugFrame == nullptr || debugFrame->GetMethod() == nullptr) {
        return false;
    }

    frameInfo = {};
    Method *method = debugFrame->GetMethod();
    frameInfo.SetFunctionName(utf::Mutf8AsCString(method->GetName().data));
    frameInfo.isStaticFrame = true;
    frameInfo.nativePtr = method;
    frameInfo.lineNumber = method->GetLineNumFromBytecodeOffset(debugFrame->GetBytecodeOffset());
    frameInfo.SetUrl(utf::Mutf8AsCString(method->GetClassSourceFile().data));

    return true;
}

bool GetDynamicFrameInfo(const void *frame, arkplatform::HybridFrameInfo &frameInfo)
{
    auto *ctx = InteropCtx::Current();
    if (ctx == nullptr) {
        return false;
    }
    auto *ecmaInterface = ctx->GetECMAInterface();
    if (ecmaInterface == nullptr) {
        return false;
    }
    return ecmaInterface->GetDynamicFrameInfo(frame, frameInfo);
}

const void *GetEcmaVM()
{
    auto *mutator = Mutator::GetCurrent();
    if (UNLIKELY(mutator == nullptr)) {
        return nullptr;
    }
    auto *execCtx = EtsExecutionContext::GetCurrent();
    if (UNLIKELY(execCtx == nullptr)) {
        return nullptr;
    }
    auto *ctx = interop::js::InteropCtx::Current(execCtx);
    if (UNLIKELY(ctx == nullptr)) {
        return nullptr;
    }
    auto *ecmaInterface = ctx->GetECMAInterface();
    if (UNLIKELY(ecmaInterface == nullptr)) {
        return nullptr;
    }
    return ecmaInterface->GetEcmaVM();
}

}  // namespace ark::ets::interop::js
