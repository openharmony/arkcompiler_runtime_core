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

#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/stackless/suspendable_job.h"
#include "plugins/ets/runtime/ets_execution_context_wrapper.h"

namespace ark::ets {

namespace {

template <typename ObjType, auto SETTER, typename... ConstructorArgs>
bool CreateAndSetCtxObject(EtsExecutionContext *const executionCtx, const EtsHandle<EtsAsyncContext> &asyncCtx,
                           ConstructorArgs... constructorArgs)
{
    auto *obj = ObjType::Create(constructorArgs...);
    if (UNLIKELY(obj == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return false;
    }
    (asyncCtx.GetPtr()->*SETTER)(executionCtx, obj);
    return true;
}

}  // namespace

// CC-OFFNXT(G.FUN.01-CPP) solid logic
EtsAsyncContext *EtsAsyncContext::Create(EtsExecutionContext *executionCtx, int32_t refSize, int32_t primSize,
                                         int32_t pc)
{
    EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->arkruntimeAsyncContext;
    auto *ctx = EtsAsyncContext::FromEtsObject(EtsObject::Create(klass));
    EtsHandle<EtsAsyncContext> asyncCtx(executionCtx, ctx);
    if (UNLIKELY(asyncCtx.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    auto *mt = executionCtx->GetMT();
    Method *method = nullptr;
    uint32_t frameSize = 0;
    if (mt->IsCurrentFrameCompiled()) {
        auto stackWalker = StackWalker::Create(mt);
        method = stackWalker.GetMethod();
        ASSERT(method != nullptr);
    } else {
        auto *frame = executionCtx->GetMT()->GetCurrentFrame();
        ASSERT(frame != nullptr);
        method = frame->GetMethod();
        ASSERT(method != nullptr);
        frameSize = method->GetNumVregs() + method->GetNumArgs();
        ASSERT(frameSize <= frame->GetSize());
        ASSERT(frameSize <= static_cast<uint32_t>(std::numeric_limits<EtsShort>::max()));
    }

    if (!CreateAndSetCtxObject<EtsPromise, &EtsAsyncContext::SetReturnValue>(executionCtx, asyncCtx, executionCtx) ||
        !CreateAndSetCtxObject<EtsObjectArray, &EtsAsyncContext::SetRefValues>(
            executionCtx, asyncCtx, PlatformTypes(executionCtx)->coreObject, refSize >= 0 ? refSize : frameSize) ||
        !CreateAndSetCtxObject<EtsLongArray, &EtsAsyncContext::SetPrimValues>(executionCtx, asyncCtx,
                                                                              primSize >= 0 ? primSize : frameSize)) {
        return nullptr;
    }

    asyncCtx->SetRefCount(0);
    asyncCtx->SetPrimCount(0);
    // Default value is ok, since it is ignored by interpreter,
    // and should be right value in compiler
    asyncCtx->SetPc(pc);
    if (pc >= 0) {
        asyncCtx->SetCompiledCode(bit_cast<uintptr_t>(method->GetCompiledEntryPoint()));
    } else {
        asyncCtx->SetCompiledCode(0);
    }

    if (!CreateAndSetCtxObject<EtsShortArray, &EtsAsyncContext::SetFrameOffsets>(
            executionCtx, asyncCtx, (primSize >= 0 || refSize >= 0) ? refSize + primSize : frameSize)) {
        return nullptr;
    }

    return asyncCtx.GetPtr();
}

/* static */
EtsAsyncContext *EtsAsyncContext::GetCurrent(EtsExecutionContext *executionCtx)
{
    if LIKELY (!StackWalker::Create(executionCtx->GetMT()).IsResumed()) {
        return nullptr;
    }
    auto *asyncContext = EtsAsyncContext::FromCoreType(
        EtsExecutionContextWrapper::CastFromMutator(executionCtx->GetMT())->GetSuspensionContext());
    ASSERT_PRINT(asyncContext == EtsAsyncContext::GetFromRefStorage(executionCtx),
                 "Cached asyncContext = " << asyncContext << ", asyncContext from ref storage = "
                                          << EtsAsyncContext::GetFromRefStorage(executionCtx));
    return asyncContext;
}

void EtsAsyncContext::AddReference(EtsExecutionContext *executionCtx, uint32_t idx, EtsObject *ref)
{
    GetRefValues(executionCtx)->Set(idx, ref);
}

void EtsAsyncContext::AddPrimitive(EtsExecutionContext *executionCtx, uint32_t idx, EtsLong primitive)
{
    GetPrimValues(executionCtx)->Set(idx, primitive);
}

EtsShort EtsAsyncContext::GetVregOffset(EtsExecutionContext *executionCtx, uint32_t idx) const
{
    ASSERT(idx < 0U + GetRefCount() + GetPrimCount());
    return GetFrameOffsets(executionCtx)->Get(idx);
}

EtsAsyncContext *EtsAsyncContext::EnsureCapacityForInterpreterFrame(EtsAsyncContext *compiledAsyncCtx,
                                                                    EtsExecutionContext *executionCtx,
                                                                    uint32_t frameSize)
{
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsAsyncContext> asyncCtx(executionCtx, compiledAsyncCtx);

    auto *refValues = asyncCtx->GetRefValues(executionCtx);
    if (refValues->GetLength() < frameSize) {
        auto *newRefValues = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, frameSize);
        if (UNLIKELY(newRefValues == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return nullptr;
        }
        asyncCtx->SetRefValues(executionCtx, newRefValues);
    }

    auto *primValues = asyncCtx->GetPrimValues(executionCtx);
    if (primValues->GetLength() < frameSize) {
        auto *newPrimValues = EtsLongArray::Create(frameSize);
        if (UNLIKELY(newPrimValues == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return nullptr;
        }
        asyncCtx->SetPrimValues(executionCtx, newPrimValues);
    }

    auto *frameOffsets = asyncCtx->GetFrameOffsets(executionCtx);
    if (frameOffsets->GetLength() < frameSize) {
        auto *newFrameOffsets = EtsShortArray::Create(frameSize);
        if (UNLIKELY(newFrameOffsets == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return nullptr;
        }
        asyncCtx->SetFrameOffsets(executionCtx, newFrameOffsets);
    }

    return asyncCtx.GetPtr();
}

namespace {
uint32_t CountRefVRegsForDebug([[maybe_unused]] StaticFrameHandler frameHandler, [[maybe_unused]] uint32_t frameSize)
{
    uint32_t refCount = 0;
#ifndef NDEBUG
    for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
        if (frameHandler.GetVReg(frameIdx).HasObject()) {
            refCount++;
        }
    }
#endif
    return refCount;
}
}  // namespace

void EtsAsyncContext::SaveInterpreterContext(ark::Frame *frame, EtsExecutionContext *executionCtx)
{
    auto *method = frame->GetMethod();
    ASSERT(method != nullptr);
    uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
    ASSERT(frameSize <= frame->GetSize());
    ASSERT(refValues_->GetLength() >= frameSize);
    ASSERT(primValues_->GetLength() >= frameSize);
    ASSERT(frameOffsets_->GetLength() >= frameSize);

    auto frameHandler = StaticFrameHandler(frame);
    LOG(DEBUG, COROUTINES) << "Saving interpreter async context for method " << method->GetFullName();

    auto *frameOffsets = GetFrameOffsets(executionCtx);

    [[maybe_unused]] const uint32_t totalRefCountForDebug = CountRefVRegsForDebug(frameHandler, frameSize);
    [[maybe_unused]] const uint32_t totalPrimCountForDebug = frameSize - totalRefCountForDebug;

    uint32_t idx = 0;
    for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
        auto vreg = frameHandler.GetVReg(frameIdx);
        if (vreg.HasObject()) {
            auto *ref = EtsObject::FromCoreType(vreg.GetReference());
            frameOffsets->Set(idx, frameIdx);
            AddReference(executionCtx, idx, ref);
            LOG(DEBUG, COROUTINES) << "Saved interpreter ref #" << idx << "/" << totalRefCountForDebug << " with value "
                                   << ref << " from frame slot " << frameIdx << " to async context";
            idx++;
        }
    }

    uint32_t refCount = std::exchange(idx, 0);
    for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
        auto vreg = frameHandler.GetVReg(frameIdx);
        if (!vreg.HasObject()) {
            auto prim = vreg.GetValue();
            frameOffsets->Set(refCount + idx, frameIdx);
            AddPrimitive(executionCtx, idx, prim);
            LOG(DEBUG, COROUTINES) << "Saved interpreter prim #" << idx << "/" << totalPrimCountForDebug
                                   << " with value " << prim << " from frame slot " << frameIdx << " to async context";
            idx++;
        }
    }
    SetRefCount(refCount);
    SetPrimCount(idx);
}

EtsLong EtsAsyncContext::RestoreInterpreterContext(ark::Frame *frame, EtsExecutionContext *executionCtx)
{
    ASSERT(GetCompiledCode() == 0);
    auto *method = frame->GetMethod();
    ASSERT(method != nullptr);
    [[maybe_unused]] uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
    ASSERT(frameSize <= frame->GetSize());
    auto frameHandler = StaticFrameHandler(frame);
    LOG(DEBUG, COROUTINES) << "Restoring interpreter async context for method " << method->GetFullName();
    auto *refValues = GetRefValues(executionCtx);

    auto *primValues = GetPrimValues(executionCtx);
    auto *frameOffsets = GetFrameOffsets(executionCtx);
    uint32_t refCount = GetRefCount();
    uint32_t primCount = GetPrimCount();

    ASSERT(refCount + primCount == frameSize);

    for (uint32_t idx = 0; idx < refCount; idx++) {
        auto offset = frameOffsets->Get(idx);
        auto vreg = frameHandler.GetVReg(offset);
        auto *ref = refValues->Get(idx);
        vreg.SetReference(EtsObject::ToCoreType(ref));
        refValues->Set(idx, nullptr);
        LOG(DEBUG, COROUTINES) << "Restored interpreter ref #" << idx << "/" << refCount << " with value " << ref
                               << " into frame slot " << offset << " from async context";
    }

    for (uint32_t idx = 0; idx < primCount; idx++) {
        auto offset = frameOffsets->Get(refCount + idx);
        auto vreg = frameHandler.GetVReg(offset);
        auto prim = primValues->Get(idx);
        vreg.SetPrimitive(prim);
        LOG(DEBUG, COROUTINES) << "Restored interpreter prim #" << idx << "/" << primCount << " with value " << prim
                               << " into frame slot " << offset << " from async context";
    }
    SetRefCount(0);
    SetPrimCount(0);
    return GetAwaitId();
}

namespace {
template <class VRegList, class Predicate, class RestoreValue>
void RestoreMatchingVRegs(const VRegList &vregList, StaticFrameHandler frameHandler, uint32_t frameSize,
                          Predicate &&pred, RestoreValue &&restoreValue)
{
    for (const auto &vregInfo : vregList) {
        if (!vregInfo.IsLive() || (vregInfo.IsSpecialVReg() && !vregInfo.IsAccumulator()) || !pred(vregInfo)) {
            continue;
        }
        if (vregInfo.IsAccumulator()) {
            restoreValue(frameHandler.GetAccAsVReg(), vregInfo);
            continue;
        }
        auto vregIndex = static_cast<uint32_t>(vregInfo.GetIndex());
        if (vregIndex >= frameSize) {
            // skip SaveStateSuspend bridges
            continue;
        }
        restoreValue(frameHandler.GetVReg(vregIndex), vregInfo);
    }
}

auto SlotOffsetVRegMatcher(uint32_t offset)
{
    return [offset](const compiler::VRegInfo &vregInfo) {
        return vregInfo.GetLocation() == compiler::VRegInfo::Location::SLOT && vregInfo.GetValue() == offset;
    };
}

auto ConstVRegMatcher()
{
    return [](const compiler::VRegInfo &vregInfo) {
        return vregInfo.GetLocation() == compiler::VRegInfo::Location::CONSTANT;
    };
}

auto ConstVRegRestorer(const compiler::CodeInfo &codeInfo)
{
    return [&codeInfo](auto vreg, const compiler::VRegInfo &vregInfo) {
        auto val = codeInfo.GetConstant(vregInfo);
        if (vregInfo.IsObject()) {
            auto *ref = reinterpret_cast<ObjectHeader *>(static_cast<ObjectPointerType>(val));
            vreg.SetReference(ref);
            LOG(DEBUG, COROUTINES) << "Restored compiled ref constant " << ref << std::dec << " into interpreter "
                                   << vregInfo;
        } else {
            vreg.SetPrimitive(val);
            LOG(DEBUG, COROUTINES) << "Restored compiled prim constant " << val << " into interpreter " << vregInfo;
        }
    };
}
}  // namespace

uint32_t EtsAsyncContext::RestoreCompiledContext(ark::Frame *frame, EtsExecutionContext *executionCtx)
{
    ASSERT(GetCompiledCode() != 0);
    auto *method = frame->GetMethod();
    ASSERT(method != nullptr);
    uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
    ASSERT(frameSize <= frame->GetSize());
    auto frameHandler = StaticFrameHandler(frame);
    LOG(DEBUG, COROUTINES) << "Restoring compiled async context for method " << method->GetFullName();

    auto *compiledCode = bit_cast<void *>(static_cast<uintptr_t>(GetCompiledCode()));
    compiler::CodeInfo codeInfo(compiler::CodeInfo::GetCodeOriginFromEntryPoint(compiledCode));
    auto stackMap = codeInfo.FindStackMapForNativePc(static_cast<uint32_t>(GetPc()));
    ASSERT(stackMap.IsValid());
    auto *internalAllocator = mem::InternalAllocator<>::GetInternalAllocatorFromRuntime();
    ASSERT(internalAllocator != nullptr);
    auto vregList = codeInfo.GetVRegList(stackMap, internalAllocator);

    auto *refValues = GetRefValues(executionCtx);
    auto *primValues = GetPrimValues(executionCtx);
    auto *frameOffsets = GetFrameOffsets(executionCtx);
    uint32_t refCount = GetRefCount();
    uint32_t primCount = GetPrimCount();

    // NOTE(compiler_team, #34469): can improve algorithm efficiency
    for (uint32_t idx = 0; idx < refCount; idx++) {
        auto offset = frameOffsets->Get(idx);
        auto *ref = refValues->Get(idx);
        auto restorer = [ref, idx, offset, refCount](auto vreg, const compiler::VRegInfo &vregInfo) {
            vreg.SetReference(EtsObject::ToCoreType(ref));
            LOG(DEBUG, COROUTINES) << "Restored compiled ref #" << idx << "/" << refCount << " with value " << ref
                                   << " matched by compiled slot " << offset << " into interpreter " << vregInfo;
        };
        RestoreMatchingVRegs(vregList, frameHandler, frameSize, SlotOffsetVRegMatcher(offset), restorer);
        refValues->Set(idx, nullptr);
    }

    for (uint32_t idx = 0; idx < primCount; idx++) {
        auto offset = frameOffsets->Get(refCount + idx);
        auto prim = primValues->Get(idx);
        auto restorer = [prim, idx, offset, primCount](auto vreg, const compiler::VRegInfo &vregInfo) {
            vreg.SetPrimitive(prim);
            LOG(DEBUG, COROUTINES) << "Restored compiled prim #" << idx << "/" << primCount << " with value " << prim
                                   << " matched by compiled slot " << offset << " into interpreter " << vregInfo;
        };
        RestoreMatchingVRegs(vregList, frameHandler, frameSize, SlotOffsetVRegMatcher(offset), restorer);
    }

    RestoreMatchingVRegs(vregList, frameHandler, frameSize, ConstVRegMatcher(), ConstVRegRestorer(codeInfo));

    SetRefCount(0);
    SetPrimCount(0);
    LOG(DEBUG, COROUTINES) << "Restored compiled native pc to awaitId: " << stackMap.GetBytecodePc();
    EVENT_CONVERT_ASYNC_CTX(std::string(frame->GetMethod()->GetFullName()), stackMap.GetBytecodePc());
    return stackMap.GetBytecodePc();
}

EtsAsyncContext *EtsAsyncContext::GetFromRefStorage(EtsExecutionContext *executionCtx)
{
    auto *job = SuspendableJob::FromExecutionContext(JobExecutionContext::CastFromMutator(executionCtx->GetMT()));
    auto *asyncCtxRef = EtsReference::CastFromReference(job->GetSuspensionContext());
    auto *refStorage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();
    return asyncCtxRef == nullptr ? nullptr : EtsAsyncContext::FromEtsObject(refStorage->GetEtsObject(asyncCtxRef));
}

}  // namespace ark::ets
