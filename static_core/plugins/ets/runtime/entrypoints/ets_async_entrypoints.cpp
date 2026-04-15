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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "runtime/include/cframe.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/object_header.h"
#include "runtime/include/stack_walker-inl.h"

namespace ark::ets {

#if defined(PANDA_TARGET_AMD64) || defined(PANDA_TARGET_ARM64)
static void SaveLiveVRegs(EtsCoroutine *coro, EtsAsyncContext *asyncContext, CFrame cframe, uintptr_t suspendRetAddr)
{
    auto executionCtx = coro->GetExecutionCtx();
    auto refValues = asyncContext->GetRefValues(executionCtx);
    auto primValues = asyncContext->GetPrimValues(executionCtx);
    auto frameOffsets = asyncContext->GetFrameOffsets(executionCtx);
    uint32_t refCount = 0;
    uint32_t primCount = 0;

    auto *compiledCode = cframe.GetMethod()->GetCompiledEntryPoint();
    compiler::CodeInfo codeInfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(compiledCode));
    auto npc = down_cast<uint32_t>(suspendRetAddr - bit_cast<uintptr_t>(compiledCode));
    auto stackMap = codeInfo.FindStackMapForNativePc(npc);
    ASSERT(stackMap.IsValid());

    asyncContext->SetCompiledCode(bit_cast<uintptr_t>(compiledCode));
    asyncContext->SetPc(npc);

    auto *internalAllocator = mem::InternalAllocator<>::GetInternalAllocatorFromRuntime();
    ASSERT(internalAllocator != nullptr);
    auto vregList = codeInfo.GetVRegList(stackMap, internalAllocator);

    for (auto vregInfo : vregList) {
        if (!vregInfo.IsLive() || vregInfo.GetLocation() == compiler::VRegInfo::Location::CONSTANT ||
            !vregInfo.IsObject()) {
            continue;
        }

        ASSERT(vregInfo.GetLocation() == compiler::VRegInfo::Location::SLOT);
        auto locationValue = static_cast<int>(vregInfo.GetValue());
        auto *refValue = bit_cast<void *>(cframe.GetValueFromSlot(locationValue));
        // remove possible garbage from high part?
        refValue = ToVoidPtr(ToObjPtr(refValue));
        frameOffsets->Set(refCount, static_cast<EtsShort>(locationValue));
        refValues->Set(refCount, static_cast<EtsObject *>(refValue));
        ++refCount;
    }

    // NOTE: remove the second pass?
    for (auto vregInfo : vregList) {
        if (!vregInfo.IsLive() || vregInfo.GetLocation() == compiler::VRegInfo::Location::CONSTANT ||
            vregInfo.IsObject()) {
            continue;
        }

        ASSERT(vregInfo.GetLocation() == compiler::VRegInfo::Location::SLOT);
        auto locationValue = static_cast<int>(vregInfo.GetValue());
        auto primValue = cframe.GetValueFromSlot(locationValue);
        frameOffsets->Set(refCount + primCount, static_cast<EtsShort>(locationValue));
        primValues->Set(primCount, primValue);
        ++primCount;
    }

    // See EtsAsyncSuspendImpl note
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    asyncContext->SetRefCount(refCount);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    asyncContext->SetPrimCount(primCount);
}

static void RestoreLiveVRegs(EtsCoroutine *coro, EtsAsyncContext *asyncContext, CFrame cframe)
{
    auto executionCtx = coro->GetExecutionCtx();
    auto refValues = asyncContext->GetRefValues(executionCtx);
    auto primValues = asyncContext->GetPrimValues(executionCtx);
    auto frameOffsets = asyncContext->GetFrameOffsets(executionCtx);
    const uint32_t refCount = asyncContext->GetRefCount();
    const uint32_t primCount = asyncContext->GetPrimCount();

    for (uint32_t idx = 0; idx < refCount; idx++) {
        auto locationValue = frameOffsets->Get(idx);
        auto *value = EtsObject::ToCoreType(refValues->Get(idx));
        cframe.SetValueToSlot(locationValue, bit_cast<CFrame::SlotType>(value));
        refValues->Set(idx, nullptr);
    }

    for (uint32_t idx = 0; idx < primCount; idx++) {
        auto locationValue = frameOffsets->Get(refCount + idx);
        auto value = primValues->Get(idx);
        cframe.SetValueToSlot(locationValue, bit_cast<CFrame::SlotType>(value));
    }

    asyncContext->SetRefCount(0);
    asyncContext->SetPrimCount(0);
}

extern "C" ObjectHeader *EtsAsyncSuspendImpl(EtsCoroutine *coro, ObjectHeader *asyncContextObj, void *fp,
                                             uintptr_t suspendRetAddr)
{
    auto *asyncContext = EtsAsyncContext::FromCoreType(asyncContextObj);
    if (UNLIKELY(asyncContext == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    SaveLiveVRegs(coro, asyncContext, CFrame(fp), suspendRetAddr);

    auto *executionCtx = coro->GetExecutionCtx();
    // Note: We don't inline async functions, so they are always on the top frame and,
    // therefore we don't call GetMethod and GC won't start.
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return asyncContext->GetReturnValue(executionCtx)->GetCoreType();
}

extern "C" uintptr_t EtsAsyncDispatchImpl(EtsCoroutine *coro, ObjectHeader *asyncContextObj, void *fp)
{
    if (asyncContextObj == nullptr) {
        return 0U;
    }

    auto *asyncContext = EtsAsyncContext::FromCoreType(asyncContextObj);
    const auto resumeAddr = static_cast<uintptr_t>(asyncContext->GetCompiledCode() + asyncContext->GetPc());
    // resumeAddr can be 0 when the async function is resumed without awaiting, e.g. when the awaited promise is already
    // resolved at the moment of awaiting.
    if (resumeAddr == 0U) {
        return 0U;
    }

    RestoreLiveVRegs(coro, asyncContext, CFrame(fp));

    return resumeAddr;
}
#endif

#if !defined(PANDA_TARGET_AMD64) && !defined(PANDA_TARGET_ARM64)
extern "C" void EtsAsyncSuspendStub([[maybe_unused]] ObjectHeader *asyncContext) {}

extern "C" void EtsAsyncDispatchStub([[maybe_unused]] ObjectHeader *asyncContext) {}
#endif

}  // namespace ark::ets
