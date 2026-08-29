/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/stack_info.h"

#include "ets_coroutine.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "runtime/execution/coroutines/stackful/stackful_coroutine.h"
#include "interop_js/napi_impl/napi_impl.h"

#include <cstddef>
#include <memory>

// NOLINTNEXTLINE(readability-identifier-naming)
napi_status __attribute__((weak)) napi_set_stackinfo(napi_env env, NapiStackInfo *info);
// NOLINTNEXTLINE(readability-identifier-naming)
napi_status __attribute__((weak)) napi_get_stackinfo(napi_env env, NapiStackInfo *result);

namespace ark::ets::interop::js {

namespace {

bool HasActiveInteropStackInfo(EtsExecutionContext *executionCtx)
{
    if (executionCtx == nullptr) {
        return false;
    }
    auto *state = executionCtx->GetLocalStorage()
                      .Get<EtsExecutionContext::DataIdx::INTEROP_STACK_INFO_STATE_PTR, InteropStackInfoState *>();
    return state != nullptr && !state->savedStackInfos.empty();
}

}  // namespace

void DeactivateInteropStackInfoForExecutionContext(EtsExecutionContext *executionCtx)
{
    if (!HasActiveInteropStackInfo(executionCtx)) {
        return;
    }
    auto *interopCtx = InteropCtx::Current(executionCtx);
    if (interopCtx != nullptr) {
        interopCtx->DeactivateInteropStackInfoIfNeeded(executionCtx);
    }
}

void ActivateInteropStackInfoForExecutionContext(EtsExecutionContext *executionCtx)
{
    if (!HasActiveInteropStackInfo(executionCtx)) {
        return;
    }
    auto *interopCtx = InteropCtx::Current(executionCtx);
    if (interopCtx != nullptr) {
        interopCtx->ActivateInteropStackInfoIfNeeded(executionCtx);
    }
}

StackInfoManagerOhos::StackInfoManagerOhos(InteropCtx *ctx, EtsExecutionContext *executionCtx)
    : StackInfoManagerBase {ctx, executionCtx}
{
}

// NOLINTNEXTLINE(modernize-use-equals-default)
StackInfoManagerOhos::~StackInfoManagerOhos() {}

void StackInfoManagerOhos::InitStackInfoIfNeeded()
{
    ASSERT(EtsExecutionContext::GetCurrent() == mainExecCtx_);
    if (LIKELY(!mainStackInfo_)) {
        mainStackInfo_ = std::make_unique<NapiStackInfo>();
        auto env = ctx_->GetJSEnv();
        auto status = napi_get_stackinfo(env, mainStackInfo_.get());
        if (UNLIKELY(status != napi_ok)) {
            INTEROP_LOG(ERROR) << "Failed to initialize interop stack info, status=" << status;
            mainStackInfo_.reset();
        }
    }
}

InteropStackInfoState *StackInfoManagerOhos::GetStackInfoState(EtsExecutionContext *executionCtx) const
{
    return executionCtx->GetLocalStorage()
        .Get<EtsExecutionContext::DataIdx::INTEROP_STACK_INFO_STATE_PTR, InteropStackInfoState *>();
}

InteropStackInfoState *StackInfoManagerOhos::GetOrCreateStackInfoState(EtsExecutionContext *executionCtx) const
{
    auto *state = GetStackInfoState(executionCtx);
    if (state == nullptr) {
        state = Runtime::GetCurrent()->GetInternalAllocator()->New<InteropStackInfoState>();
        if (UNLIKELY(state == nullptr)) {
            INTEROP_LOG(ERROR) << "Failed to allocate interop stack info state";
            return nullptr;
        }
        executionCtx->GetLocalStorage().Set<EtsExecutionContext::DataIdx::INTEROP_STACK_INFO_STATE_PTR>(
            state, [](void *param) {
                Runtime::GetCurrent()->GetInternalAllocator()->Delete(static_cast<InteropStackInfoState *>(param));
            });
    }
    return state;
}

bool StackInfoManagerOhos::SetStackInfo(NapiStackInfo &stackInfo) const
{
    auto status = napi_set_stackinfo(ctx_->GetJSEnv(), &stackInfo);
    if (UNLIKELY(status != napi_ok)) {
        INTEROP_LOG(ERROR) << "Failed to set interop stack info, status=" << status;
        return false;
    }
    return true;
}

// Preserve the void UpdateStackInfoIfNeeded() API while allowing snapshot operations to handle failures.
bool StackInfoManagerOhos::SetCurrentStackInfo(EtsExecutionContext *executionCtx)
{
    if (UNLIKELY(!mainStackInfo_)) {
        return true;
    }

    NapiStackInfo currentStackInfo {};
    if (executionCtx == mainExecCtx_) {
        currentStackInfo = *mainStackInfo_;
    } else {
        void *stackAddr = nullptr;
        size_t stackSize {};
        size_t guardSize {};
        ASSERT(executionCtx != nullptr);
        auto *coro = EtsCoroutine::CastFromThread(executionCtx->GetMT());
        coro->GetContext<StackfulCoroutineContext>()->RetrieveStackInfo(stackAddr, stackSize, guardSize);
        currentStackInfo = {reinterpret_cast<size_t>(stackAddr), stackSize};
    }

    return SetStackInfo(currentStackInfo);
}

void StackInfoManagerOhos::UpdateStackInfoIfNeeded()
{
    SetCurrentStackInfo(EtsExecutionContext::GetCurrent());
}

bool StackInfoManagerOhos::PushAndUpdateStackInfoIfNeeded(EtsExecutionContext *executionCtx)
{
    if (UNLIKELY(!mainStackInfo_)) {
        return true;
    }

    NapiStackInfo previousStackInfo {};
    auto status = napi_get_stackinfo(ctx_->GetJSEnv(), &previousStackInfo);
    if (UNLIKELY(status != napi_ok)) {
        INTEROP_LOG(ERROR) << "Failed to save interop stack info, status=" << status;
        return false;
    }

    auto *state = GetOrCreateStackInfoState(executionCtx);
    if (UNLIKELY(state == nullptr)) {
        return false;
    }
    state->savedStackInfos.push_back(previousStackInfo);
    if (UNLIKELY(!SetCurrentStackInfo(executionCtx))) {
        state->savedStackInfos.pop_back();
        return false;
    }
    return true;
}

bool StackInfoManagerOhos::RestoreStackInfoIfNeeded(EtsExecutionContext *executionCtx)
{
    if (UNLIKELY(!mainStackInfo_)) {
        return true;
    }
    auto *state = GetStackInfoState(executionCtx);
    if (UNLIKELY(state == nullptr || state->savedStackInfos.empty())) {
        INTEROP_LOG(ERROR) << "Cannot restore interop stack info: scope stack is empty";
        return false;
    }

    auto previousStackInfo = state->savedStackInfos.back();
    state->savedStackInfos.pop_back();
    return SetStackInfo(previousStackInfo);
}

void StackInfoManagerOhos::DeactivateStackInfoIfNeeded(EtsExecutionContext *executionCtx)
{
    if (UNLIKELY(!mainStackInfo_)) {
        return;
    }
    auto *state = GetStackInfoState(executionCtx);
    if (state == nullptr || state->savedStackInfos.empty()) {
        return;
    }

    if (UNLIKELY(!SetStackInfo(state->savedStackInfos.front()))) {
        INTEROP_LOG(ERROR) << "Failed to deactivate interop stack info";
    }
}

void StackInfoManagerOhos::ActivateStackInfoIfNeeded(EtsExecutionContext *executionCtx)
{
    if (UNLIKELY(!mainStackInfo_)) {
        return;
    }
    auto *state = GetStackInfoState(executionCtx);
    if (state == nullptr || state->savedStackInfos.empty()) {
        return;
    }

    if (UNLIKELY(!SetCurrentStackInfo(executionCtx))) {
        INTEROP_LOG(ERROR) << "Failed to activate interop stack info";
    }
}

}  // namespace ark::ets::interop::js
