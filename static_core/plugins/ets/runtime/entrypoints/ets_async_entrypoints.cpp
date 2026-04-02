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

#include "libarkbase/mem/mem.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "runtime/include/cframe.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/ets_execution_context_wrapper.h"

namespace ark::ets {

#if defined(PANDA_TARGET_AMD64) || defined(PANDA_TARGET_ARM64)
namespace {
const void *GetCodeEntryPoint(CFrame cframe)
{
    if (cframe.ShouldDeoptimize()) {
        // When method was deoptimized due to speculation failure, regular code entry become invalid,
        // so we read entry from special backup field in the frame.
        return cframe.GetDeoptCodeEntry();
    }
    return cframe.GetMethod()->GetCompiledEntryPoint();
}

template <class VRegList>
std::pair<size_t, size_t> CountLiveVRegs(const VRegList &vregList)
{
    size_t primCount = 0;
    size_t refCount = 0;
    for (auto vregInfo : vregList) {
        if (!vregInfo.IsLive() || vregInfo.GetLocation() == compiler::VRegInfo::Location::CONSTANT) {
            continue;
        }
        if (vregInfo.IsObject()) {
            refCount++;
        } else {
            primCount++;
        }
    }
    return {refCount, primCount};
}

void SaveLiveVRegs(EtsExecutionContext *executionCtx, EtsAsyncContext *asyncContext, CFrame cframe,
                   uintptr_t suspendRetAddr)
{
    auto refValues = asyncContext->GetRefValues(executionCtx);
    auto primValues = asyncContext->GetPrimValues(executionCtx);
    auto frameOffsets = asyncContext->GetFrameOffsets(executionCtx);

    LOG(DEBUG, COROUTINES) << "Saving compiled async context for method " << cframe.GetMethod()->GetFullName();
    const void *compiledCode = GetCodeEntryPoint(cframe);
    compiler::CodeInfo codeInfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(compiledCode));
    auto npc = down_cast<uint32_t>(suspendRetAddr - bit_cast<uintptr_t>(compiledCode));
    auto stackMap = codeInfo.FindStackMapForNativePc(npc);
    ASSERT(stackMap.IsValid());

    asyncContext->SetCompiledCode(bit_cast<uintptr_t>(compiledCode));
    asyncContext->SetPc(npc);

    auto *internalAllocator = mem::InternalAllocator<>::GetInternalAllocatorFromRuntime();
    ASSERT(internalAllocator != nullptr);
    auto vregList = codeInfo.GetVRegList(stackMap, internalAllocator);
    const auto [totalRefCount, totalPrimCount] = CountLiveVRegs(vregList);

    uint32_t curRefCount = 0;
    uint32_t curPrimCount = 0;
    for (auto vregInfo : vregList) {
        if (!vregInfo.IsLive() || vregInfo.GetLocation() == compiler::VRegInfo::Location::CONSTANT) {
            continue;
        }
        ASSERT(vregInfo.GetLocation() == compiler::VRegInfo::Location::SLOT);
        auto locationValue = static_cast<EtsShort>(vregInfo.GetValue());
        auto slotValue = cframe.GetValueFromSlot(locationValue);
        if (vregInfo.IsObject()) {
            // remove possible garbage from high part?
            void *refValue = ToVoidPtr(ToObjPtr(bit_cast<void *>(slotValue)));
            frameOffsets->Set(curRefCount, locationValue);
            refValues->Set(curRefCount, static_cast<EtsObject *>(refValue));
            LOG(DEBUG, COROUTINES) << "Saved compiled ref #" << curRefCount << "/" << totalRefCount << " with value "
                                   << refValue << " from CFrame slot " << locationValue << " to async context ("
                                   << vregInfo << ")";
            ++curRefCount;
        } else {
            auto primValue = slotValue;
            frameOffsets->Set(totalRefCount + curPrimCount, static_cast<EtsShort>(locationValue));
            primValues->Set(curPrimCount, primValue);
            LOG(DEBUG, COROUTINES) << "Saved compiled prim #" << curPrimCount << "/" << totalPrimCount << " with value "
                                   << primValue << " from CFrame slot " << locationValue << " to async context ("
                                   << vregInfo << ")";
            ++curPrimCount;
        }
    }

    // See EtsAsyncSuspendImpl note
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    asyncContext->SetRefCount(totalRefCount);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    asyncContext->SetPrimCount(totalPrimCount);
}

void RestoreLiveVRegs(EtsExecutionContext *executionCtx, EtsAsyncContext *asyncContext, CFrame cframe)
{
    auto refValues = asyncContext->GetRefValues(executionCtx);
    auto primValues = asyncContext->GetPrimValues(executionCtx);
    auto frameOffsets = asyncContext->GetFrameOffsets(executionCtx);
    const uint32_t refCount = asyncContext->GetRefCount();
    const uint32_t primCount = asyncContext->GetPrimCount();

    LOG(DEBUG, COROUTINES) << "Restoring compiled async context for method " << cframe.GetMethod()->GetFullName();
    for (uint32_t idx = 0; idx < refCount; idx++) {
        auto locationValue = frameOffsets->Get(idx);
        auto *value = EtsObject::ToCoreType(refValues->Get(idx));
        cframe.SetValueToSlot(locationValue, bit_cast<CFrame::SlotType>(value));
        refValues->Set(idx, nullptr);
        LOG(DEBUG, COROUTINES) << "Restored compiled ref #" << idx << "/" << refCount << " with value " << value
                               << " into CFrame slot " << locationValue << " from async context";
    }

    for (uint32_t idx = 0; idx < primCount; idx++) {
        auto locationValue = frameOffsets->Get(refCount + idx);
        auto value = primValues->Get(idx);
        cframe.SetValueToSlot(locationValue, bit_cast<CFrame::SlotType>(value));
        LOG(DEBUG, COROUTINES) << "Restored compiled prim #" << idx << "/" << primCount << " with value " << value
                               << " into CFrame slot " << locationValue << " from async context";
    }

    asyncContext->SetRefCount(0);
    asyncContext->SetPrimCount(0);
}
}  // namespace

extern "C" ObjectHeader *EtsAsyncSuspendImpl(EtsExecutionContextWrapper *executionCtxWrapper,
                                             ObjectHeader *asyncContextObj, void *fp, uintptr_t suspendRetAddr)
{
    auto *asyncContext = EtsAsyncContext::FromCoreType(asyncContextObj);
    if (UNLIKELY(asyncContext == nullptr)) {
        // NOTE(compiler_team, #34466) consider deoptimizing from IR code instead
        ThrowNullPointerException();
        return nullptr;
    }

    auto *executionCtx = EtsExecutionContext::FromMT(executionCtxWrapper);
    SaveLiveVRegs(executionCtx, asyncContext, CFrame(fp), suspendRetAddr);

    // Note: We don't inline async functions, so they are always on the top frame and,
    // therefore we don't call GetMethod and GC won't start.
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return asyncContext->GetReturnValue(executionCtx)->GetCoreType();
}

extern "C" uintptr_t EtsAsyncDispatchImpl(EtsExecutionContextWrapper *executionCtxWrapper,
                                          ObjectHeader *asyncContextObj, void *fp)
{
    if (asyncContextObj == nullptr) {
        return 0U;
    }

    auto *asyncContext = EtsAsyncContext::FromCoreType(asyncContextObj);
    CFrame cframe(fp);
    auto resumeAddr = ToUintPtr(GetCodeEntryPoint(cframe)) + asyncContext->GetPc();
    auto *executionCtx = EtsExecutionContext::FromMT(executionCtxWrapper);
    RestoreLiveVRegs(executionCtx, asyncContext, cframe);
    return resumeAddr;
}
#endif

#if !defined(PANDA_TARGET_AMD64) && !defined(PANDA_TARGET_ARM64)
extern "C" void EtsAsyncSuspendStub([[maybe_unused]] ObjectHeader *asyncContext) {}

extern "C" void EtsAsyncDispatchStub([[maybe_unused]] ObjectHeader *asyncContext) {}
#endif

}  // namespace ark::ets
