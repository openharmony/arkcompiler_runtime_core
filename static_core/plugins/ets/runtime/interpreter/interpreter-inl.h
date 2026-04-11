/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGINS_ETS_INTERPRETER_INTERPRETER_INL_H
#define PLUGINS_ETS_INTERPRETER_INTERPRETER_INL_H

#include <cstdint>
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/interpreter/interpreter-inl.h"
#include "runtime/mem/internal_allocator.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/class_linker-inl.h"

namespace ark::ets {
template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_DEBUG, bool IS_PROFILE_ENABLED>
class InstructionHandler : public interpreter::InstructionHandler<RuntimeIfaceT, IS_DYNAMIC, IS_DEBUG> {
public:
    ALWAYS_INLINE inline explicit InstructionHandler(interpreter::InstructionHandlerState *state)
        : interpreter::InstructionHandler<RuntimeIfaceT, IS_DYNAMIC, IS_DEBUG>(state)
    {
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLdnullvalue()
    {
        LOG_INST() << "ets.ldnullvalue";

        this->GetAccAsVReg().SetReference(GetExecutionContext()->GetNullValue());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsMovnullvalue()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "ets.movnullvalue v" << vd;

        this->GetFrameHandler().GetVReg(vd).SetReference(GetExecutionContext()->GetNullValue());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsIsnullvalue()
    {
        LOG_INST() << "ets.isnullvalue";

        ObjectHeader *obj = this->GetAcc().GetReference();
        this->GetAccAsVReg().SetPrimitive(obj == GetExecutionContext()->GetNullValue());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsEquals()
    {
        uint16_t v1 = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t v2 = this->GetInst().template GetVReg<FORMAT, 1>();

        LOG_INST() << "ets.equals v" << v1 << ", v" << v2;

        ObjectHeader *obj1 = this->GetFrame()->GetVReg(v1).GetReference();
        ObjectHeader *obj2 = this->GetFrame()->GetVReg(v2).GetReference();

        bool res =
            EtsReferenceEquals(GetExecutionContext(), EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2));
        this->GetAccAsVReg().SetPrimitive(res);
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsStrictequals()
    {
        uint16_t v1 = this->GetInst().template GetVReg<FORMAT, 0>();
        uint16_t v2 = this->GetInst().template GetVReg<FORMAT, 1>();

        LOG_INST() << "ets.strictequals v" << v1 << ", v" << v2;

        ObjectHeader *obj1 = this->GetFrame()->GetVReg(v1).GetReference();
        ObjectHeader *obj2 = this->GetFrame()->GetVReg(v2).GetReference();

        bool res = EtsReferenceEquals<true>(GetExecutionContext(), EtsObject::FromCoreType(obj1),
                                            EtsObject::FromCoreType(obj2));
        this->GetAccAsVReg().SetPrimitive(res);
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsTypeof()
    {
        uint16_t v1 = this->GetInst().template GetVReg<FORMAT, 0>();

        LOG_INST() << "ets.typeof v" << v1;

        ObjectHeader *obj = this->GetFrame()->GetVReg(v1).GetReference();

        EtsString *res = EtsReferenceTypeof(GetExecutionContext(), EtsObject::FromCoreType(obj));
        this->GetAccAsVReg().SetReference(res->AsObjectHeader());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsIstrue()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT, 0>();

        LOG_INST() << "ets.istrue v" << v;

        ObjectHeader *obj = this->GetFrame()->GetVReg(v).GetReference();

        bool res = EtsIstrue(GetExecutionContext(), EtsObject::FromCoreType(obj));
        this->GetAccAsVReg().SetPrimitive(res);
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsNullcheck()
    {
        LOG_INST() << "ets.nullcheck";

        if (UNLIKELY(this->GetAcc().GetReference() == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return;
        }
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleSuspendStackless()
    {
        ASSERT(this->GetFrame()->IsStackless());

        uint16_t v = this->GetInst().template GetVReg<FORMAT>();

        auto *executionCtx = this->GetExecutionContext();
        auto *frame = this->GetFrame();
        auto *asyncCtx = EtsAsyncContext::FromCoreType(frame->GetVReg(v).GetReference());

        // NOTE(panferovi): need to handle compiler -> interpreter call
        auto *prev = frame->GetPrevFrame();

        // return to the caller frame
        frame->GetAcc().Set(asyncCtx->GetReturnValue(executionCtx));

        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(executionCtx->GetMT(), frame->GetMethod());

        this->GetInstructionHandlerState()->UpdateInstructionHandlerState(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            prev->GetInstruction() + prev->GetBytecodeOffset(), prev);

        this->SetDispatchTable(this->GetThread()->template GetCurrentDispatchTable<IS_DEBUG>());
        RuntimeIfaceT::SetCurrentFrame(executionCtx->GetMT(), prev);

        this->GetAcc() = frame->GetAcc();
        this->SetInst(prev->GetNextInstruction());

        RuntimeIfaceT::FreeFrame(this->GetThread(), frame);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsAsyncSuspend()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "ets.async.suspend v" << v;

        auto *frame = this->GetFrame();
        ASSERT(frame != nullptr);

        auto *asyncCtx = EtsAsyncContext::FromCoreType(frame->GetVReg(v).GetReference());
        if (UNLIKELY(asyncCtx == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return;
        }

        // store resume point id
        auto resumePointId = reinterpret_cast<int64_t>(this->GetInst().GetNext().GetAddress());
        asyncCtx->SetAwaitId(resumePointId);

        // save vregs to context
        auto frameHandler = GetFrameHandler(frame);
        auto *executionCtx = this->GetExecutionContext();
        auto *frameOffsets = asyncCtx->GetFrameOffsets(executionCtx);
        auto *method = frame->GetMethod();
        ASSERT(method != nullptr);
        uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
        ASSERT(frameSize <= frame->GetSize());
        uint32_t idx = 0;
        for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
            auto vreg = frameHandler.GetVReg(frameIdx);
            if (vreg.HasObject()) {
                frameOffsets->Set(idx, frameIdx);
                asyncCtx->AddReference(executionCtx, idx, EtsObject::FromCoreType(vreg.GetReference()));
                idx++;
            }
        }

        uint32_t refCount = std::exchange(idx, 0);
        for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
            auto vreg = frameHandler.GetVReg(frameIdx);
            if (!vreg.HasObject()) {
                frameOffsets->Set(refCount + idx, frameIdx);
                asyncCtx->AddPrimitive(executionCtx, idx, vreg.GetValue());
                idx++;
            }
        }
        uint32_t primCount = idx;
        asyncCtx->SetRefCount(refCount);
        asyncCtx->SetPrimCount(primCount);
        asyncCtx->SetCompiledCode(0);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsAsyncDispatch()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "ets.async.dispatch v" << v;

        auto *executionCtx = this->GetExecutionContext();
        auto *frame = this->GetFrame();
        auto *asyncCtx = EtsAsyncContext::FromCoreType(frame->GetVReg(v).GetReference());

        if (asyncCtx == nullptr) {
            this->template MoveToNextInst<FORMAT, true>();
            return;
        }
        ASSERT(asyncCtx->GetCompiledCode() == 0);

        // load vregs from context
        auto frameHandler = GetFrameHandler(frame);

        auto *refValues = asyncCtx->GetRefValues(executionCtx);
        auto *primValues = asyncCtx->GetPrimValues(executionCtx);
        auto *frameOffsets = asyncCtx->GetFrameOffsets(executionCtx);
        uint32_t refCount = asyncCtx->GetRefCount();
        uint32_t primCount = asyncCtx->GetPrimCount();

        [[maybe_unused]] auto *method = frame->GetMethod();
        ASSERT(method != nullptr);
        [[maybe_unused]] uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
        ASSERT(frameSize <= frame->GetSize());
        ASSERT(refCount + primCount == frameSize);

        for (uint32_t idx = 0; idx < refCount; idx++) {
            auto offset = frameOffsets->Get(idx);
            auto vreg = frameHandler.GetVReg(offset);
            auto *ref = refValues->Get(idx);
            vreg.SetReference(EtsObject::ToCoreType(ref));
            refValues->Set(idx, nullptr);
        }

        for (uint32_t idx = 0; idx < primCount; idx++) {
            auto offset = frameOffsets->Get(refCount + idx);
            auto vreg = frameHandler.GetVReg(offset);
            auto prim = primValues->Get(idx);
            vreg.SetPrimitive(prim);
        }

        asyncCtx->SetRefCount(0);
        asyncCtx->SetPrimCount(0);

        // load resume point id
        auto resumePointId = reinterpret_cast<const uint8_t *>(asyncCtx->GetAwaitId());
        this->template JumpTo<true>(resumePointId);
    }

private:
    ALWAYS_INLINE bool IsNull(ObjectHeader *obj)
    {
        return obj == nullptr;
    }

    ALWAYS_INLINE bool IsUndefined(ObjectHeader *obj)
    {
        return obj == GetExecutionContext()->GetUndefinedObject();
    }

    ALWAYS_INLINE bool IsNullish(ObjectHeader *obj)
    {
        return IsNull(obj) || IsUndefined(obj);
    }

    ALWAYS_INLINE ManagedThread *GetManagedThread() const
    {
        return this->GetThread();
    }

    ALWAYS_INLINE EtsExecutionContext *GetExecutionContext() const
    {
        return EtsExecutionContext::FromMT(GetManagedThread());
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE ObjectHeader *GetObjHelper()
    {
        uint16_t objVreg = this->GetInst().template GetVReg<FORMAT, 0>();
        return this->GetFrame()->GetVReg(objVreg).GetReference();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE ObjectHeader *GetCallerObject()
    {
        ObjectHeader *obj = GetObjHelper<FORMAT>();

        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return nullptr;
        }
        return obj;
    }

    ALWAYS_INLINE Method *ResolveMethod(BytecodeId id)
    {
        this->UpdateBytecodeOffset();

        InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Method>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        if (res != nullptr) {
            return res;
        }

        this->GetFrame()->SetAcc(this->GetAcc());
        auto *method = RuntimeIfaceT::ResolveMethod(this->GetThread(), *this->GetFrame()->GetMethod(), id);
        this->GetAcc() = this->GetFrame()->GetAcc();
        if (UNLIKELY(method == nullptr)) {
            ASSERT(this->GetThread()->HasPendingException());
            return nullptr;
        }

        cache->Set(this->GetInst().GetAddress(), method, this->GetFrame()->GetMethod());
        return method;
    }
};
}  // namespace ark::ets
#endif  // PLUGINS_ETS_INTERPRETER_INTERPRETER_INL_H
