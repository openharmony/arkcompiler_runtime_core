/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "intrinsics.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "runtime/include/exceptions.h"
#include "compiler/optimizer/ir/constants.h"

namespace panda::ets::intrinsics {

constexpr static uint64_t METHOD_FLAG_MASK = 0x00000001;

template <bool IS_STORE>
void LookUpException(panda::Class *klass, Field *raw_field)
{
    {
        auto type = IS_STORE ? "setter" : "getter";
        auto error_msg = "Class " + panda::ConvertToString(klass->GetName()) + " does not have field and " +
                         panda::ConvertToString(type) + " with name " + utf::Mutf8AsCString(raw_field->GetName().data);
        ThrowEtsException(
            EtsCoroutine::GetCurrent(),
            utf::Mutf8AsCString(
                Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS).GetNoSuchFieldErrorDescriptor()),
            error_msg);
    }
    HandlePendingException();
}

template <panda_file::Type::TypeId FIELD_TYPE>
Field *TryGetField(panda::Method *method, Field *raw_field, uint32_t pc, panda::Class *klass)
{
    auto cache = EtsCoroutine::GetCurrent()->GetInterpreterCache();
    bool use_ic = pc != panda::compiler::INVALID_PC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto address = method->GetInstructions() + (use_ic ? pc : 0);
    if (use_ic) {
        auto *res = cache->template Get<Field>(address, method);
        auto res_uint = reinterpret_cast<uint64_t>(res);
        if (res != nullptr && ((res_uint & METHOD_FLAG_MASK) != 1) && (res->GetClass() == klass)) {
            return res;
        }
    }
    auto field = klass->LookupFieldByName(raw_field->GetName());
    if (field != nullptr && use_ic) {
        cache->template Set(address, field, method);
    }
    return field;
}

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
panda::Method *TryGetCallee(panda::Method *method, Field *raw_field, uint32_t pc, panda::Class *klass)
{
    auto cache = EtsCoroutine::GetCurrent()->GetInterpreterCache();
    bool use_ic = pc != panda::compiler::INVALID_PC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto address = method->GetInstructions() + (use_ic ? pc : 0);
    if (use_ic) {
        auto *res = cache->template Get<Method>(address, method);
        auto res_uint = reinterpret_cast<uint64_t>(res);
        auto method_ptr = reinterpret_cast<Method *>(res_uint & ~METHOD_FLAG_MASK);
        if (res != nullptr && ((res_uint & METHOD_FLAG_MASK) == 1) && (method_ptr->GetClass() == klass)) {
            return method_ptr;
        }
    }
    panda::Method *callee;
    if constexpr (IS_GETTER) {
        callee = klass->LookupGetterByName<FIELD_TYPE>(raw_field->GetName());
    } else {
        callee = klass->LookupSetterByName<FIELD_TYPE>(raw_field->GetName());
    }
    if (callee != nullptr && use_ic) {
        auto m_uint = reinterpret_cast<uint64_t>(callee);
        ASSERT((m_uint & METHOD_FLAG_MASK) == 0);
        cache->template Set(address, reinterpret_cast<Method *>(m_uint | METHOD_FLAG_MASK), method);
    }
    return callee;
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
T CompilerEtsLdObjByName(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj)
{
    ASSERT(method != nullptr);
    panda::Class *klass;
    panda::Field *raw_field;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handle_obj(thread, obj);
        auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<panda::Class *>(handle_obj.GetPtr()->ClassAddr<panda::BaseClass>());
        raw_field = class_linker->GetField(*method, panda_file::File::EntityId(id));

        auto field = TryGetField<FIELD_TYPE>(method, raw_field, pc, klass);
        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                return handle_obj.GetPtr()->GetFieldObject(*field);
            } else {
                switch (field->GetTypeId()) {
                    case panda_file::Type::TypeId::U1:
                    case panda_file::Type::TypeId::U8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<uint8_t>(*field);
                    }
                    case panda_file::Type::TypeId::I8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<int8_t>(*field);
                    }
                    case panda_file::Type::TypeId::I16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<int16_t>(*field);
                    }
                    case panda_file::Type::TypeId::U16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<uint16_t>(*field);
                    }
                    case panda_file::Type::TypeId::I32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<int32_t>(*field);
                    }
                    case panda_file::Type::TypeId::U32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<uint32_t>(*field);
                    }
                    case panda_file::Type::TypeId::I64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<int64_t>(*field);
                    }
                    case panda_file::Type::TypeId::U64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<uint64_t>(*field);
                    }
                    case panda_file::Type::TypeId::F32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F32);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<float>(*field);
                    }
                    case panda_file::Type::TypeId::F64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F64);
                        return handle_obj.GetPtr()->template GetFieldPrimitive<double>(*field);
                    }
                    default:
                        UNREACHABLE();
                        break;
                }
                return handle_obj.GetPtr()->template GetFieldPrimitive<T>(*field);
            }
        }

        auto callee = TryGetCallee<FIELD_TYPE, true>(method, raw_field, pc, klass);
        if (callee != nullptr) {
            // PandaVector<Value> args;
            Value val(handle_obj.GetPtr());
            Value result = callee->Invoke(Coroutine::GetCurrent(), &val);
            return result.GetAs<T>();
        }
    }
    LookUpException<false>(klass, raw_field);
    UNREACHABLE();
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
void CompilerEtsStObjByName(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj, T store_value)
{
    ASSERT(method != nullptr);
    panda::Class *klass;
    panda::Field *raw_field;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handle_obj(thread, obj);
        auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
        raw_field = class_linker->GetField(*method, panda_file::File::EntityId(id));

        auto field = TryGetField<FIELD_TYPE>(method, raw_field, pc, klass);
        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                UNREACHABLE();
            } else {
                switch (field->GetTypeId()) {
                    case panda_file::Type::TypeId::U1:
                    case panda_file::Type::TypeId::U8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint8_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::I8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<int8_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::I16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<int16_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::U16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint16_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::I32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<int32_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::U32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint32_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::I64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<int64_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::U64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint64_t>(store_value));
                        return;
                    }
                    case panda_file::Type::TypeId::F32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F32);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, store_value);
                        return;
                    }
                    case panda_file::Type::TypeId::F64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F64);
                        handle_obj.GetPtr()->SetFieldPrimitive(*field, store_value);
                        return;
                    }
                    default: {
                        UNREACHABLE();
                        return;
                    }
                }
            }
        }

        auto callee = TryGetCallee<FIELD_TYPE, false>(method, raw_field, pc, klass);
        if (callee != nullptr) {
            PandaVector<Value> args {Value(handle_obj.GetPtr()), Value(store_value)};
            callee->Invoke(Coroutine::GetCurrent(), args.data());
            return;
        }
    }
    LookUpException<true>(klass, raw_field);
    UNREACHABLE();
}

void CompilerEtsStObjByNameRef(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                               panda::ObjectHeader *store_value)
{
    ASSERT(method != nullptr);
    panda::Class *klass;
    panda::Field *raw_field;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handle_obj(thread, obj);
        VMHandle<ObjectHeader> handle_store(thread, store_value);
        auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
        raw_field = class_linker->GetField(*method, panda_file::File::EntityId(id));

        auto field = TryGetField<panda_file::Type::TypeId::REFERENCE>(method, raw_field, pc, klass);
        if (field != nullptr) {
            return handle_obj.GetPtr()->SetFieldObject(*field, handle_store.GetPtr());
        }

        auto callee = TryGetCallee<panda_file::Type::TypeId::REFERENCE, false>(method, raw_field, pc, klass);
        if (callee != nullptr) {
            PandaVector<Value> args {Value(handle_obj.GetPtr()), Value(handle_store.GetPtr())};
            callee->Invoke(Coroutine::GetCurrent(), args.data());
            return;
        }
    }
    LookUpException<true>(klass, raw_field);
    UNREACHABLE();
}

extern "C" int32_t CompilerEtsLdObjByNameI32(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj);
}

extern "C" int64_t CompilerEtsLdObjByNameI64(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::I64, int64_t>(method, id, pc, obj);
}

extern "C" float CompilerEtsLdObjByNameF32(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::F32, float>(method, id, pc, obj);
}

extern "C" double CompilerEtsLdObjByNameF64(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::F64, double>(method, id, pc, obj);
}

extern "C" panda::ObjectHeader *CompilerEtsLdObjByNameObj(panda::Method *method, int32_t id, uint32_t pc,
                                                          panda::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::REFERENCE, panda::ObjectHeader *>(method, id, pc, obj);
}

extern "C" void CompilerEtsStObjByNameI32(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          int32_t store_value)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj, store_value);
}

extern "C" void CompilerEtsStObjByNameI64(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          int64_t store_value)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I64, int64_t>(method, id, pc, obj, store_value);
}

extern "C" void CompilerEtsStObjByNameF32(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          float store_value)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::F32, float>(method, id, pc, obj, store_value);
}

extern "C" void CompilerEtsStObjByNameF64(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          double store_value)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::F64, double>(method, id, pc, obj, store_value);
}

extern "C" void CompilerEtsStObjByNameObj(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          panda::ObjectHeader *store_value)
{
    CompilerEtsStObjByNameRef(method, id, pc, obj, store_value);
}

extern "C" panda::ObjectHeader *CompilerEtsLdundefined()
{
    return panda::ets::EtsCoroutine::GetCurrent()->GetUndefinedObject();
}

}  // namespace panda::ets::intrinsics
