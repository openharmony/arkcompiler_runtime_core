/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace panda::ets {
template <class RuntimeIfaceT, bool IS_DYNAMIC, bool IS_DEBUG, bool IS_PROFILE_ENABLED>
class InstructionHandler : public interpreter::InstructionHandler<RuntimeIfaceT, IS_DYNAMIC, IS_DEBUG> {
public:
    ALWAYS_INLINE inline explicit InstructionHandler(interpreter::InstructionHandlerState *state)
        : interpreter::InstructionHandler<RuntimeIfaceT, IS_DYNAMIC, IS_DEBUG>(state)
    {
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLaunchShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "launch.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", " << std::hex << "0x" << id;
        HandleLaunch<FORMAT, false>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLaunch()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "launch v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 3>() << ", " << std::hex << "0x" << id;
        HandleLaunch<FORMAT, false>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLaunchRange()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "launch.range v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", " << std::hex << "0x"
                   << id;
        HandleLaunch<FORMAT, true>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLaunchVirtShort()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "launch.virt.short v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", " << std::hex << "0x" << id;
        HandleLaunchVirt<FORMAT, false>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLaunchVirt()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "launch.virt v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 1>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 2>() << ", v"
                   << this->GetInst().template GetVReg<FORMAT, 3>() << ", " << std::hex << "0x" << id;
        HandleLaunchVirt<FORMAT, false>(id);
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLaunchVirtRange()
    {
        auto id = this->GetInst().template GetId<FORMAT>();
        LOG_INST() << "launch.virt.range v" << this->GetInst().template GetVReg<FORMAT, 0>() << ", " << std::hex << "0x"
                   << id;
        HandleLaunchVirt<FORMAT, true>(id);
    }

    constexpr static uint64_t METHOD_FLAG_MASK = 0x00000001;

    template <panda_file::Type::TypeId FIELD_TYPE, bool IS_LOAD>
    ALWAYS_INLINE Field *LookupFieldByName(Class *klass, Field *raw_field)
    {
        auto cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Field>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        auto res_uint = reinterpret_cast<uint64_t>(res);
        if (res != nullptr && ((res_uint & METHOD_FLAG_MASK) != 1) && (res->GetClass() == klass)) {
            return res;
        }

        auto field = klass->LookupFieldByName(raw_field->GetName());

        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                if constexpr (IS_LOAD) {
                    ASSERT(raw_field->ResolveTypeClass()->IsAssignableFrom(field->ResolveTypeClass()));
                } else {
                    ASSERT(field->ResolveTypeClass()->IsAssignableFrom(raw_field->ResolveTypeClass()));
                }
            }
            ASSERT((reinterpret_cast<uint64_t>(field) & METHOD_FLAG_MASK) == 0);
            cache->template Set(this->GetInst().GetAddress(), field, this->GetFrame()->GetMethod());
        }
        return field;
    }

    template <panda_file::Type::TypeId FIELD_TYPE>
    ALWAYS_INLINE Method *LookupGetterByName(Class *klass, Field *raw_field)
    {
        auto cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Method>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        auto res_uint = reinterpret_cast<uint64_t>(res);
        auto method_ptr = reinterpret_cast<Method *>(res_uint & ~METHOD_FLAG_MASK);
        if (res != nullptr && ((res_uint & METHOD_FLAG_MASK) == 1) && (method_ptr->GetClass() == klass)) {
            return method_ptr;
        }

        auto method = klass->LookupGetterByName<FIELD_TYPE>(raw_field->GetName());

        if (method != nullptr) {
#ifndef NDEBUG
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                auto pf = method->GetPandaFile();
                panda_file::ProtoDataAccessor pda(*pf,
                                                  panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
                auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
                auto arg_class = class_linker->GetClass(*pf, pda.GetReferenceType(0), klass->GetLoadContext());
                ASSERT(raw_field->ResolveTypeClass()->IsAssignableFrom(arg_class));
            }
#endif  // !NDEBUG
            auto m_uint = reinterpret_cast<uint64_t>(method);
            ASSERT((m_uint & METHOD_FLAG_MASK) == 0);
            cache->template Set(this->GetInst().GetAddress(), reinterpret_cast<Method *>(m_uint | METHOD_FLAG_MASK),
                                this->GetFrame()->GetMethod());
        }
        return method;
    }

    template <panda_file::Type::TypeId FIELD_TYPE>
    ALWAYS_INLINE Method *LookupSetterByName(Class *klass, Field *raw_field)
    {
        auto cache = this->GetThread()->GetInterpreterCache();
        auto *res = cache->template Get<Method>(this->GetInst().GetAddress(), this->GetFrame()->GetMethod());
        auto res_uint = reinterpret_cast<uint64_t>(res);
        auto method_ptr = reinterpret_cast<Method *>(res_uint & ~METHOD_FLAG_MASK);
        if (res != nullptr && ((res_uint & METHOD_FLAG_MASK) == 1) && (method_ptr->GetClass() == klass)) {
            return method_ptr;
        }

        auto method = klass->LookupSetterByName<FIELD_TYPE>(raw_field->GetName());
        if (method != nullptr) {
#ifndef NDEBUG
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                auto pf = method->GetPandaFile();
                panda_file::ProtoDataAccessor pda(*pf,
                                                  panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
                auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
                auto arg_class = class_linker->GetClass(*pf, pda.GetReferenceType(0), klass->GetLoadContext());
                ASSERT(arg_class->IsAssignableFrom(raw_field->ResolveTypeClass()));
            }
#endif  // !NDEBUG
            auto m_uint = reinterpret_cast<uint64_t>(method);
            ASSERT((m_uint & METHOD_FLAG_MASK) == 0);
            cache->template Set(this->GetInst().GetAddress(), reinterpret_cast<Method *>(m_uint | METHOD_FLAG_MASK),
                                this->GetFrame()->GetMethod());
        }
        return method;
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
            auto klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
            auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto raw_field = class_linker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()));
            auto field = LookupFieldByName<panda_file::Type::TypeId::I32, true>(klass, raw_field);
            if (field != nullptr) {
                this->LoadPrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }

            auto method = LookupGetterByName<panda_file::Type::TypeId::I32>(klass, raw_field);
            if (method != nullptr) {
                this->template HandleCall<panda::interpreter::FrameHelperDefault, FORMAT,
                                          /* is_dynamic = */ false,
                                          /* is_range= */ false, /* accept_acc= */ false,
                                          /* initobj= */ false, /* call = */ false>(method);
                return;
            }
            auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) +
                             " does not have field and getter with name " +
                             utf::Mutf8AsCString(raw_field->GetName().data);
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              utf::Mutf8AsCString(Runtime::GetCurrent()
                                                      ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                      .GetNoSuchFieldErrorDescriptor()),
                              error_msg);
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
            auto klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
            auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto raw_field = class_linker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()));
            auto field = LookupFieldByName<panda_file::Type::TypeId::I64, true>(klass, raw_field);
            if (field != nullptr) {
                this->LoadPrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }

            auto method = LookupGetterByName<panda_file::Type::TypeId::I64>(klass, raw_field);
            if (method != nullptr) {
                this->template HandleCall<panda::interpreter::FrameHelperDefault, FORMAT,
                                          /* is_dynamic = */ false,
                                          /* is_range= */ false, /* accept_acc= */ false,
                                          /* initobj= */ false, /* call = */ false>(method);
                return;
            }
            auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) +
                             " does not have field and getter with name " +
                             utf::Mutf8AsCString(raw_field->GetName().data);
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              utf::Mutf8AsCString(Runtime::GetCurrent()
                                                      ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                      .GetNoSuchFieldErrorDescriptor()),
                              error_msg);
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
            auto klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
            auto class_linker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto raw_field = class_linker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()));
            auto field = LookupFieldByName<panda_file::Type::TypeId::REFERENCE, true>(klass, raw_field);
            if (field != nullptr) {
                ASSERT(field->GetType().IsReference());
                this->GetAccAsVReg().SetReference(
                    obj->GetFieldObject<RuntimeIfaceT::NEED_READ_BARRIER>(this->GetThread(), *field));
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }

            auto method = LookupGetterByName<panda_file::Type::TypeId::REFERENCE>(klass, raw_field);
            if (method != nullptr) {
                this->template HandleCall<panda::interpreter::FrameHelperDefault, FORMAT,
                                          /* is_dynamic = */ false,
                                          /* is_range= */ false, /* accept_acc= */ false,
                                          /* initobj= */ false, /* call = */ false>(method);
                return;
            }
            auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) +
                             " does not have field and getter with name " +
                             utf::Mutf8AsCString(raw_field->GetName().data);
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              utf::Mutf8AsCString(Runtime::GetCurrent()
                                                      ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                      .GetNoSuchFieldErrorDescriptor()),
                              error_msg);
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
            auto klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
            auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto raw_field = class_linker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()));
            auto field = LookupFieldByName<panda_file::Type::TypeId::I32, false>(klass, raw_field);
            if (field != nullptr) {
                this->StorePrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }

            auto method = LookupSetterByName<panda_file::Type::TypeId::I32>(klass, raw_field);
            if (method != nullptr) {
                this->template HandleCall<panda::interpreter::FrameHelperDefault, FORMAT,
                                          /* is_dynamic = */ false,
                                          /* is_range= */ false, /* accept_acc= */ false,
                                          /* initobj= */ false, /* call = */ false>(method);
                return;
            }
            auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) +
                             " does not have field and setter with name " +
                             utf::Mutf8AsCString(raw_field->GetName().data);
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              utf::Mutf8AsCString(Runtime::GetCurrent()
                                                      ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                      .GetNoSuchFieldErrorDescriptor()),
                              error_msg);
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
            auto klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
            auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto raw_field = class_linker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()));
            auto field = LookupFieldByName<panda_file::Type::TypeId::I64, false>(klass, raw_field);
            if (field != nullptr) {
                this->StorePrimitiveField(obj, field);
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }

            auto method = LookupSetterByName<panda_file::Type::TypeId::I64>(klass, raw_field);
            if (method != nullptr) {
                this->template HandleCall<panda::interpreter::FrameHelperDefault, FORMAT,
                                          /* is_dynamic = */ false,
                                          /* is_range= */ false, /* accept_acc= */ false,
                                          /* initobj= */ false, /* call = */ false>(method);
                return;
            }
            auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) +
                             " does not have field and setter with name " +
                             utf::Mutf8AsCString(raw_field->GetName().data);
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              utf::Mutf8AsCString(Runtime::GetCurrent()
                                                      ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                      .GetNoSuchFieldErrorDescriptor()),
                              error_msg);
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
            auto klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
            auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
            auto caller = this->GetFrame()->GetMethod();
            auto raw_field = class_linker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id.AsIndex()));
            auto field = LookupFieldByName<panda_file::Type::TypeId::REFERENCE, false>(klass, raw_field);
            if (field != nullptr) {
                ASSERT(field->GetType().IsReference());
                obj->SetFieldObject<RuntimeIfaceT::NEED_WRITE_BARRIER>(this->GetThread(), *field,
                                                                       this->GetAcc().GetReference());
                this->template MoveToNextInst<FORMAT, true>();
                return;
            }

            auto method = LookupSetterByName<panda_file::Type::TypeId::REFERENCE>(klass, raw_field);
            if (method != nullptr) {
                this->template HandleCall<panda::interpreter::FrameHelperDefault, FORMAT,
                                          /* is_dynamic = */ false,
                                          /* is_range= */ false, /* accept_acc= */ false,
                                          /* initobj= */ false, /* call = */ false>(method);
                return;
            }
            auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) +
                             " does not have field and setter with name " +
                             utf::Mutf8AsCString(raw_field->GetName().data);
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              utf::Mutf8AsCString(Runtime::GetCurrent()
                                                      ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                      .GetNoSuchFieldErrorDescriptor()),
                              error_msg);
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsLdundefined()
    {
        LOG_INST() << "ets.ldundefined";

        this->GetAccAsVReg().SetReference(GetCoro()->GetUndefinedObject());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsMovundefined()
    {
        uint16_t vd = this->GetInst().template GetVReg<FORMAT>();
        LOG_INST() << "ets.movundefined v" << vd;

        this->GetFrameHandler().GetVReg(vd).SetReference(GetCoro()->GetUndefinedObject());
        this->template MoveToNextInst<FORMAT, true>();
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE void HandleEtsIsundefined()
    {
        LOG_INST() << "ets.isundefined";

        ObjectHeader *obj = this->GetAcc().GetReference();
        this->GetAcc().Set(obj == GetCoro()->GetUndefinedObject() ? 1 : 0);
        this->template MoveToNextInst<FORMAT, true>();
    }

private:
    ALWAYS_INLINE EtsCoroutine *GetCoro() const
    {
        return EtsCoroutine::CastFromThread(this->GetThread());
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_RANGE>
    ALWAYS_INLINE void HandleLaunchVirt(BytecodeId method_id)
    {
        auto *method = ResolveMethod(method_id);
        if (LIKELY(method != nullptr)) {
            ObjectHeader *obj = this->GetCallerObject<FORMAT>();
            if (UNLIKELY(obj == nullptr)) {
                return;
            }
            auto *cls = obj->ClassAddr<Class>();
            ASSERT(cls != nullptr);
            method = cls->ResolveVirtualMethod(method);
            ASSERT(method != nullptr);
            HandleLaunch<FORMAT, IS_RANGE>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_RANGE>
    ALWAYS_INLINE void HandleLaunch(BytecodeId method_id)
    {
        auto *method = ResolveMethod(method_id);
        if (LIKELY(method != nullptr)) {
            HandleLaunch<FORMAT, IS_RANGE>(method);
        } else {
            this->MoveToExceptionHandler();
        }
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_RANGE>
    ALWAYS_INLINE void HandleLaunch(Method *method)
    {
        if (UNLIKELY(!method->Verify())) {
            RuntimeIfaceT::ThrowVerificationException(method->GetFullName());
            this->MoveToExceptionHandler();
            return;
        }

        // this promise is going to be resolved on coro completion
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        EtsPromise *promise = EtsPromise::Create(coroutine);
        if (UNLIKELY(promise == nullptr)) {
            this->MoveToExceptionHandler();
            return;
        }
        PandaEtsVM *ets_vm = coroutine->GetPandaVM();
        auto promise_ref = ets_vm->GetGlobalObjectStorage()->Add(promise, mem::Reference::ObjectType::WEAK);
        auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promise_ref);
        promise->SetEventPtr(evt);

        // create the coro and put it to the ready queue
        [[maybe_unused]] EtsHandleScope scope(coroutine);
        EtsHandle<EtsPromise> promise_handle(coroutine, promise);

        // since transferring arguments from frame registers (which are local roots for GC) to a C++ vector
        // introduces the potential risk of pointer invalidation in case GC moves the referenced objects,
        // we would like to do this transfer below all potential GC invocation points (e.g. Promise::Create)
        size_t num_args = method->GetNumArgs();
        PandaVector<Value> args(num_args);
        FillArgs<FORMAT, IS_RANGE>(args);

        auto *coro = coroutine->GetCoroutineManager()->Launch(evt, method, std::move(args));
        if (UNLIKELY(coro == nullptr)) {
            // OOM
            promise_handle.GetPtr()->SetEventPtr(nullptr);
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
            this->MoveToExceptionHandler();
            return;
        }

        this->GetAccAsVReg().SetReference(promise_handle.GetPtr());
        this->GetFrame()->SetAcc(this->GetAcc());
        this->template MoveToNextInst<FORMAT, false>();
    }

    template <BytecodeInstruction::Format FORMAT, bool IS_RANGE>
    ALWAYS_INLINE void FillArgs(PandaVector<Value> &args)
    {
        if (args.empty()) {
            return;
        }

        auto cur_frame_handler = this->template GetFrameHandler<false>();
        if constexpr (IS_RANGE) {
            uint16_t start_reg = this->GetInst().template GetVReg<FORMAT, 0>();
            for (size_t i = 0; i < args.size(); ++i) {
                args[i] = Value::FromVReg(cur_frame_handler.GetVReg(start_reg + i));
            }
        } else {
            // launch.short of launch
            args[0] = Value::FromVReg(cur_frame_handler.GetVReg(this->GetInst().template GetVReg<FORMAT, 0U>()));
            if (args.size() > 1U) {
                args[1] = Value::FromVReg(cur_frame_handler.GetVReg(this->GetInst().template GetVReg<FORMAT, 1U>()));
            }
            if constexpr (FORMAT == BytecodeInstruction::Format::PREF_V4_V4_V4_V4_ID16) {
                if (args.size() > 2U) {
                    args[2] =
                        Value::FromVReg(cur_frame_handler.GetVReg(this->GetInst().template GetVReg<FORMAT, 2U>()));
                }
                if (args.size() > 3U) {
                    args[3] =
                        Value::FromVReg(cur_frame_handler.GetVReg(this->GetInst().template GetVReg<FORMAT, 3U>()));
                }
            }
        }
    }

    template <BytecodeInstruction::Format FORMAT>
    ALWAYS_INLINE ObjectHeader *GetObjHelper()
    {
        uint16_t obj_vreg = this->GetInst().template GetVReg<FORMAT, 0>();
        return this->GetFrame()->GetVReg(obj_vreg).GetReference();
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

        auto cache = this->GetThread()->GetInterpreterCache();
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
}  // namespace panda::ets
#endif  // PLUGINS_ETS_INTERPRETER_INTERPRETER_INL_H
