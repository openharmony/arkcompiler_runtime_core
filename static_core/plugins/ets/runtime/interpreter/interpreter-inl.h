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
#include "runtime/coroutines/coroutine_manager.h"
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
    ALWAYS_INLINE void HandleEtsLdobjName()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ets.ldobj.name v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
            auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()), false);
            InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
            Field *field = GetFieldByName<true>(cache->GetEntry(this->GetInst().GetAddress()), caller, rawField,
                                                this->GetInst().GetAddress(), klass);
            if (field != nullptr) {
                // GetFieldByName can trigger GC hanse need to reread
                obj = this->GetFrame()->GetVReg(vs).GetReference();
                this->LoadPrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }
            ark::Method *method = GetAccessorByName<panda_file::Type::TypeId::I32, true>(
                cache->GetEntry(this->GetInst().GetAddress()), caller, rawField, this->GetInst().GetAddress(), klass);
            if (method != nullptr) {
                this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, false, false, false,
                                          false>(method);
                return;
            }
            LookUpException<true>(klass, rawField);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLdobjNameWide()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ets.ldobj.name.64 v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
            auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()), false);
            InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
            Field *field = GetFieldByName<true>(cache->GetEntry(this->GetInst().GetAddress()), caller, rawField,
                                                this->GetInst().GetAddress(), klass);
            if (field != nullptr) {
                // GetFieldByName can trigger GC hanse need to reread
                obj = this->GetFrame()->GetVReg(vs).GetReference();
                this->LoadPrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }
            ark::Method *method = GetAccessorByName<panda_file::Type::TypeId::I64, true>(
                cache->GetEntry(this->GetInst().GetAddress()), caller, rawField, this->GetInst().GetAddress(), klass);
            if (method != nullptr) {
                this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, false, false, false,
                                          false>(method);
                return;
            }
            LookUpException<true>(klass, rawField);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLdobjNameObj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ets.ldobj.name.obj v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
            auto classLinker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()), false);
            InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
            Field *field = GetFieldByName<true>(cache->GetEntry(this->GetInst().GetAddress()), caller, rawField,
                                                this->GetInst().GetAddress(), klass);
            if (field != nullptr) {
                // GetFieldByName can trigger GC hanse need to reread
                obj = this->GetFrame()->GetVReg(vs).GetReference();
                ASSERT(field->GetType().IsReference());
                this->GetAccAsVReg().SetReference(
                    obj->GetFieldObject<RuntimeIfaceT::NEED_READ_BARRIER>(this->GetThread(), *field));
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }
            ark::Method *method = GetAccessorByName<panda_file::Type::TypeId::REFERENCE, true>(
                cache->GetEntry(this->GetInst().GetAddress()), caller, rawField, this->GetInst().GetAddress(), klass);
            if (method != nullptr) {
                this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, false, false, false,
                                          false>(method);
                return;
            }
            LookUpException<true>(klass, rawField);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsStobjName()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ets.stobj.name v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
            auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()), false);
            InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
            Field *field = GetFieldByName<false>(cache->GetEntry(this->GetInst().GetAddress()), caller, rawField,
                                                 this->GetInst().GetAddress(), klass);
            if (field != nullptr) {
                // GetFieldByName can trigger GC hanse need to reread
                obj = this->GetFrame()->GetVReg(vs).GetReference();
                this->StorePrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }
            ark::Method *method = GetAccessorByName<panda_file::Type::TypeId::I32, false>(
                cache->GetEntry(this->GetInst().GetAddress()), caller, rawField, this->GetInst().GetAddress(), klass);
            if (method != nullptr) {
                this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, false, false, false,
                                          false>(method);
                return;
            }
            LookUpException<false>(klass, rawField);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsStobjNameWide()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ets.stobj.name.64 v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
            auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()), false);
            InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
            Field *field = GetFieldByName<false>(cache->GetEntry(this->GetInst().GetAddress()), caller, rawField,
                                                 this->GetInst().GetAddress(), klass);
            if (field != nullptr) {
                // GetFieldByName can trigger GC hanse need to reread
                obj = this->GetFrame()->GetVReg(vs).GetReference();
                this->StorePrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }
            ark::Method *method = GetAccessorByName<panda_file::Type::TypeId::I64, false>(
                cache->GetEntry(this->GetInst().GetAddress()), caller, rawField, this->GetInst().GetAddress(), klass);
            if (method != nullptr) {
                this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, false, false, false,
                                          false>(method);
                return;
            }
            LookUpException<false>(klass, rawField);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsStobjNameObj()
    {
        uint16_t vs = this->GetInst().template GetVReg<FORMAT>();
        auto id = this->GetInst().template GetId<FORMAT>();

        LOG_INST() << "ets.stobj.name v" << vs << ", " << std::hex << "0x" << id;

        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
        } else {
            auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
            auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()), false);
            InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
            Field *field = GetFieldByName<false>(cache->GetEntry(this->GetInst().GetAddress()), caller, rawField,
                                                 this->GetInst().GetAddress(), klass);
            if (field != nullptr) {
                // GetFieldByName can trigger GC hanse need to reread
                obj = this->GetFrame()->GetVReg(vs).GetReference();
                ASSERT(field->GetType().IsReference());
                obj->SetFieldObject<RuntimeIfaceT::NEED_WRITE_BARRIER>(this->GetThread(), *field,
                                                                       this->GetAcc().GetReference());
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }
            ark::Method *method = GetAccessorByName<panda_file::Type::TypeId::REFERENCE, false>(
                cache->GetEntry(this->GetInst().GetAddress()), caller, rawField, this->GetInst().GetAddress(), klass);
            if (method != nullptr) {
                this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, false, false, false,
                                          false>(method);
                return;
            }
            LookUpException<false>(klass, rawField);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_RANGE = false>
    ALWAYS_INLINE void HandleEtsMethodCallName()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        uint16_t vs = this->GetInst().template GetVReg<FORMAT, 0>();
        ObjectHeader *obj = this->GetFrame()->GetVReg(vs).GetReference();
        if (UNLIKELY(obj == nullptr)) {
            RuntimeIfaceT::ThrowNullPointerException();
            this->MoveToExceptionHandler();
            return;
        }
        auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto caller = this->GetFrame()->GetMethod();
        auto rawMethod = classLinker->GetMethod(*caller, caller->GetClass()->ResolveMethodIndex(id.AsIndex()));
        if (UNLIKELY(rawMethod == nullptr)) {
            HandlePendingException();
            this->MoveToExceptionHandler();
            return;
        }
        InterpreterCache *cache = this->GetThread()->GetInterpreterCache();
        ETSStubCacheInfo cacheInfo {cache->GetEntry(this->GetInst().GetAddress()), caller,
                                    this->GetInst().GetAddress()};
        ark::Method *method = GetMethodByName(GetCoro(), cacheInfo, rawMethod, klass);
        if (UNLIKELY(method == nullptr)) {
            LookUpException(klass, rawMethod);
            this->MoveToExceptionHandler();
            return;
        }
        if (method->IsAbstract()) {
            RuntimeIfaceT::ThrowAbstractMethodError(method);
            this->MoveToExceptionHandler();
            return;
        }
        this->template HandleCall<ark::interpreter::FrameHelperDefault, FORMAT, false, IS_RANGE, false, false, false>(
            method);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsCallNameShort()
    {
        LOG_INST() << "ets.call.name.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", " << std::hex << "0x"
                   << this->GetInst().template GetId<FORMAT>();

        HandleEtsMethodCallName<FORMAT>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsCallName()
    {
        LOG_INST() << "ets.call.name v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 3>() << ", " << std::hex << "0x"
                   << this->GetInst().template GetId<FORMAT>();

        HandleEtsMethodCallName<FORMAT>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsCallNameRange()
    {
        LOG_INST() << "ets.call.name.range v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", " << std::hex
                   << "0x" << this->GetInst().template GetId<FORMAT>();

        HandleEtsMethodCallName<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLdnullvalue()
    {
        LOG_INST() << "ets.ldnullvalue";

        this->GetAccAsVReg().SetReference(GetCoro()->GetNullValue());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsMovnullvalue()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "ets.movnullvalue v" << vd;

        this->GetFrameHandler().GetVReg(vd).SetReference(GetCoro()->GetNullValue());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsIsnullvalue()
    {
        LOG_INST() << "ets.isnullvalue";

        ObjectHeader *obj = this->GetAcc().GetReference();
        this->GetAccAsVReg().SetPrimitive(obj == GetCoro()->GetNullValue());
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

        bool res = EtsReferenceEquals(GetCoro(), EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2));
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

        bool res = EtsReferenceEquals<true>(GetCoro(), EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2));
        this->GetAccAsVReg().SetPrimitive(res);
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsTypeof()
    {
        uint16_t v1 = this->GetInst().template GetVReg<FORMAT, 0>();

        LOG_INST() << "ets.typeof v" << v1;

        ObjectHeader *obj = this->GetFrame()->GetVReg(v1).GetReference();

        EtsString *res = EtsReferenceTypeof(GetCoro(), EtsObject::FromCoreType(obj));
        this->GetAccAsVReg().SetReference(res->AsObjectHeader());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsIstrue()
    {
        uint16_t v = this->GetInst().template GetVReg<FORMAT, 0>();

        LOG_INST() << "ets.istrue v" << v;

        ObjectHeader *obj = this->GetFrame()->GetVReg(v).GetReference();

        bool res = EtsIstrue(GetCoro(), EtsObject::FromCoreType(obj));
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

        auto *coro = this->GetCoro();
        auto *frame = this->GetFrame();
        auto *asyncCtx = EtsAsyncContext::FromCoreType(frame->GetVReg(v).GetReference());

        // NOTE(panferovi): need to handle compiler -> interpreter call
        auto *prev = frame->GetPrevFrame();

        // return to the caller frame
        frame->GetAcc().Set(asyncCtx->GetReturnValue(coro));

        Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(coro, frame->GetMethod());

        this->GetInstructionHandlerState()->UpdateInstructionHandlerState(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            prev->GetInstruction() + prev->GetBytecodeOffset(), prev);

        this->SetDispatchTable(this->GetThread()->template GetCurrentDispatchTable<IS_DEBUG>());
        RuntimeIfaceT::SetCurrentFrame(coro, prev);

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
        auto *coro = this->GetCoro();
        auto *frameOffsets = asyncCtx->GetFrameOffsets(coro);
        auto *method = frame->GetMethod();
        ASSERT(method != nullptr);
        uint32_t frameSize = method->GetNumVregs() + method->GetNumArgs();
        ASSERT(frameSize <= frame->GetSize());
        uint32_t idx = 0;
        for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
            auto vreg = frameHandler.GetVReg(frameIdx);
            if (vreg.HasObject()) {
                frameOffsets->Set(idx, frameIdx);
                asyncCtx->AddReference(coro, idx, EtsObject::FromCoreType(vreg.GetReference()));
                idx++;
            }
        }

        uint32_t refCount = std::exchange(idx, 0);
        for (uint32_t frameIdx = 0; frameIdx < frameSize; frameIdx++) {
            auto vreg = frameHandler.GetVReg(frameIdx);
            if (!vreg.HasObject()) {
                frameOffsets->Set(refCount + idx, frameIdx);
                asyncCtx->AddPrimitive(coro, idx, vreg.GetValue());
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

        auto *coro = this->GetCoro();
        auto *frame = this->GetFrame();
        auto *asyncCtx = EtsAsyncContext::FromCoreType(frame->GetVReg(v).GetReference());

        if (asyncCtx == nullptr) {
            ObjectHeader *undefined = nullptr;
            this->GetAccAsVReg().SetReference(undefined);
            this->template MoveToNextInst<FORMAT, true>();
            return;
        }
        ASSERT(asyncCtx->GetCompiledCode() == 0);

        // load vregs from context
        auto frameHandler = GetFrameHandler(frame);

        auto *refValues = asyncCtx->GetRefValues(coro);
        auto *primValues = asyncCtx->GetPrimValues(coro);
        auto *frameOffsets = asyncCtx->GetFrameOffsets(coro);
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

        auto *awaiteePromise = asyncCtx->GetAwaitee(coro);
        ASSERT(!awaiteePromise->IsPending());
        this->GetAccAsVReg().SetReference(EtsObject::ToCoreType(awaiteePromise->GetValue(coro)));

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
        return obj == GetCoro()->GetUndefinedObject();
    }

    ALWAYS_INLINE bool IsNullish(ObjectHeader *obj)
    {
        return IsNull(obj) || IsUndefined(obj);
    }

    ALWAYS_INLINE EtsCoroutine *GetCoro() const
    {
        return EtsCoroutine::CastFromThread(this->GetThread());
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
