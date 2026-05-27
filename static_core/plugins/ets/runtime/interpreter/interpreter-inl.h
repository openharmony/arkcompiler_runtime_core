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
#include "plugins/ets/runtime/intrinsics/helpers/intrinsic_promise_impl.h"
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
    ALWAYS_INLINE void HandleAwaitStackless()
    {
        ASSERT(this->GetFrame()->IsStackless());

        auto *executionCtx = this->GetExecutionContext();
        auto *frame = this->GetFrame();

        // NOTE(panferovi): need to handle compiler -> interpreter call
        auto *prev = frame->GetPrevFrame();

        // return to the caller frame
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
    ALWAYS_INLINE void HandleEtsAsyncAwait()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT>();

        LOG_INST() << "ets.async.await v" << v;

        auto *frame = this->GetFrame();
        ASSERT(frame != nullptr);

        auto *awaitee = EtsPromise::FromCoreType(frame->GetVReg(v).GetReference());
        if (UNLIKELY(awaitee == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return;
        }

        auto aCtx = intrinsics::helpers::EtsAwaitPromiseImpl(awaitee);
        if (UNLIKELY(aCtx == nullptr)) {
            ASSERT(this->GetThread()->HasPendingException());
            this->MoveToExceptionHandler();
            return;
        }

        auto asyncCtx = EtsAsyncContext::FromEtsObject(aCtx);

        // store resume point id
        auto resumePointId = this->GetBytecodeOffset() + this->GetInst().GetSize();
        asyncCtx->SetAwaitId(resumePointId);

        // make sure we have enough space in potentially compiled async context
        auto *executionCtx = this->GetExecutionContext();
        if (UNLIKELY(asyncCtx->GetCompiledCode() != 0)) {
            asyncCtx = EtsAsyncContext::EnsureCapacityForInterpreterFrame(asyncCtx, executionCtx,
                                                                          frame->GetSize());  // may trigger GC
            if (UNLIKELY(asyncCtx == nullptr)) {
                this->MoveToExceptionHandler();
                return;
            }
            asyncCtx->SetCompiledCode(0);
        }

        // save vregs to context
        asyncCtx->SaveInterpreterContext(frame, executionCtx);
        frame->GetAccAsVReg().SetReference(asyncCtx->GetReturnValue(executionCtx)->GetCoreType());
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsAsyncDispatch()
    {
        LOG_INST() << "ets.async.dispatch";

        auto *executionCtx = this->GetExecutionContext();
        auto *asyncCtx = EtsAsyncContext::GetCurrent(executionCtx);

        if (asyncCtx == nullptr) {
            this->template MoveToNextInst<FORMAT, true>();
            return;
        }

        // load vregs & resume point id from context
        auto *frame = this->GetFrame();
        int32_t resumePoint = 0;
        if (asyncCtx->GetCompiledCode() == 0) {
            ASSERT(this->GetBytecodeOffset() < asyncCtx->GetAwaitId());
            resumePoint = asyncCtx->RestoreInterpreterContext(frame, executionCtx);
        } else {
            resumePoint = asyncCtx->RestoreCompiledContext(frame, executionCtx);
        }
        resumePoint -= this->GetBytecodeOffset();
        this->template JumpToInst<true>(resumePoint);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsAsyncUnpack()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "ets.async.unpack v" << v;

        auto *frame = this->GetFrame();
        auto *executionCtx = this->GetExecutionContext();
        auto *awaitee = EtsPromise::FromCoreType(frame->GetVReg(v).GetReference());
        if (UNLIKELY(awaitee == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return;
        }

        if (UNLIKELY(awaitee->IsPending())) {
            ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreInvalidAsyncOperationError,
                              "Cannot unpack pending promise");
            this->MoveToExceptionHandler();
            return;
        }

        if (awaitee->IsResolved()) {
            this->GetAccAsVReg().SetReference(awaitee->GetValue(executionCtx)->GetCoreType());
            this->template MoveToNextInst<FORMAT, true>();
            return;
        }

        ASSERT(awaitee->IsRejected());
        executionCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(awaitee, executionCtx);
        executionCtx->GetMT()->SetException(awaitee->GetValue(executionCtx)->GetCoreType());
        this->MoveToExceptionHandler();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsAsyncResolve()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "ets.async.resolve v" << v;

        auto *frame = this->GetFrame();
        auto *executionCtx = this->GetExecutionContext();
        auto *asyncCtx = EtsAsyncContext::GetCurrent(executionCtx);
        auto *promise = asyncCtx != nullptr ? asyncCtx->GetReturnValue(executionCtx) : EtsPromise::Create(executionCtx);
        if (UNLIKELY(promise == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            this->MoveToExceptionHandler();
            return;
        }

        this->GetAccAsVReg().SetReference(promise->GetCoreType());
        intrinsics::helpers::EtsPromiseResolveImpl(executionCtx, promise,
                                                   EtsObject::FromCoreType(frame->GetVReg(v).GetReference()));
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsAsyncReject()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "ets.async.reject v" << v;

        auto *frame = this->GetFrame();
        auto *executionCtx = this->GetExecutionContext();
        auto *asyncCtx = EtsAsyncContext::GetCurrent(executionCtx);
        auto *promise = asyncCtx != nullptr ? asyncCtx->GetReturnValue(executionCtx) : EtsPromise::Create(executionCtx);
        if (UNLIKELY(promise == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            this->MoveToExceptionHandler();
            return;
        }

        this->GetAccAsVReg().SetReference(promise->GetCoreType());
        intrinsics::helpers::EtsPromiseRejectImpl(executionCtx, promise,
                                                  EtsObject::FromCoreType(frame->GetVReg(v).GetReference()));
        this->template MoveToNextInst<FORMAT, true>();
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
