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
void LookUpException(panda::Class *klass, Field *rawField)
{
    {
        auto type = IS_STORE ? "setter" : "getter";
        auto errorMsg = "Class " + panda::ConvertToString(klass->GetName()) + " does not have field and " +
                        panda::ConvertToString(type) + " with name " + utf::Mutf8AsCString(rawField->GetName().data);
        ThrowEtsException(
            EtsCoroutine::GetCurrent(),
            utf::Mutf8AsCString(
                Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS).GetNoSuchFieldErrorDescriptor()),
            errorMsg);
    }
    HandlePendingException();
}

template <panda_file::Type::TypeId FIELD_TYPE>
Field *TryGetField(panda::Method *method, Field *rawField, uint32_t pc, panda::Class *klass)
{
    auto cache = EtsCoroutine::GetCurrent()->GetInterpreterCache();
    bool useIc = pc != panda::compiler::INVALID_PC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto address = method->GetInstructions() + (useIc ? pc : 0);
    if (useIc) {
        auto *res = cache->template Get<Field>(address, method);
        auto resUint = reinterpret_cast<uint64_t>(res);
        if (res != nullptr && ((resUint & METHOD_FLAG_MASK) != 1) && (res->GetClass() == klass)) {
            return res;
        }
    }
    auto field = klass->LookupFieldByName(rawField->GetName());
    if (field != nullptr && useIc) {
        cache->template Set(address, field, method);
    }
    return field;
}

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
panda::Method *TryGetCallee(panda::Method *method, Field *rawField, uint32_t pc, panda::Class *klass)
{
    auto cache = EtsCoroutine::GetCurrent()->GetInterpreterCache();
    bool useIc = pc != panda::compiler::INVALID_PC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto address = method->GetInstructions() + (useIc ? pc : 0);
    if (useIc) {
        auto *res = cache->template Get<Method>(address, method);
        auto resUint = reinterpret_cast<uint64_t>(res);
        auto methodPtr = reinterpret_cast<Method *>(resUint & ~METHOD_FLAG_MASK);
        if (res != nullptr && ((resUint & METHOD_FLAG_MASK) == 1) && (methodPtr->GetClass() == klass)) {
            return methodPtr;
        }
    }
    panda::Method *callee;
    if constexpr (IS_GETTER) {
        callee = klass->LookupGetterByName<FIELD_TYPE>(rawField->GetName());
    } else {
        callee = klass->LookupSetterByName<FIELD_TYPE>(rawField->GetName());
    }
    if (callee != nullptr && useIc) {
        auto mUint = reinterpret_cast<uint64_t>(callee);
        ASSERT((mUint & METHOD_FLAG_MASK) == 0);
        cache->template Set(address, reinterpret_cast<Method *>(mUint | METHOD_FLAG_MASK), method);
    }
    return callee;
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
T CompilerEtsLdObjByName(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj)
{
    ASSERT(method != nullptr);
    panda::Class *klass;
    panda::Field *rawField;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, obj);
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<panda::Class *>(handleObj.GetPtr()->ClassAddr<panda::BaseClass>());
        rawField = classLinker->GetField(*method, panda_file::File::EntityId(id));

        auto field = TryGetField<FIELD_TYPE>(method, rawField, pc, klass);
        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                return handleObj.GetPtr()->GetFieldObject(*field);
            } else {
                switch (field->GetTypeId()) {
                    case panda_file::Type::TypeId::U1:
                    case panda_file::Type::TypeId::U8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<uint8_t>(*field);
                    }
                    case panda_file::Type::TypeId::I8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<int8_t>(*field);
                    }
                    case panda_file::Type::TypeId::I16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<int16_t>(*field);
                    }
                    case panda_file::Type::TypeId::U16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<uint16_t>(*field);
                    }
                    case panda_file::Type::TypeId::I32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<int32_t>(*field);
                    }
                    case panda_file::Type::TypeId::U32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<uint32_t>(*field);
                    }
                    case panda_file::Type::TypeId::I64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        return handleObj.GetPtr()->template GetFieldPrimitive<int64_t>(*field);
                    }
                    case panda_file::Type::TypeId::U64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        return handleObj.GetPtr()->template GetFieldPrimitive<uint64_t>(*field);
                    }
                    case panda_file::Type::TypeId::F32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F32);
                        return handleObj.GetPtr()->template GetFieldPrimitive<float>(*field);
                    }
                    case panda_file::Type::TypeId::F64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F64);
                        return handleObj.GetPtr()->template GetFieldPrimitive<double>(*field);
                    }
                    default:
                        UNREACHABLE();
                        break;
                }
                return handleObj.GetPtr()->template GetFieldPrimitive<T>(*field);
            }
        }

        auto callee = TryGetCallee<FIELD_TYPE, true>(method, rawField, pc, klass);
        if (callee != nullptr) {
            // PandaVector<Value> args;
            Value val(handleObj.GetPtr());
            Value result = callee->Invoke(Coroutine::GetCurrent(), &val);
            return result.GetAs<T>();
        }
    }
    LookUpException<false>(klass, rawField);
    UNREACHABLE();
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
void CompilerEtsStObjByName(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj, T storeValue)
{
    ASSERT(method != nullptr);
    panda::Class *klass;
    panda::Field *rawField;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, obj);
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
        rawField = classLinker->GetField(*method, panda_file::File::EntityId(id));

        auto field = TryGetField<FIELD_TYPE>(method, rawField, pc, klass);
        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                UNREACHABLE();
            } else {
                switch (field->GetTypeId()) {
                    case panda_file::Type::TypeId::U1:
                    case panda_file::Type::TypeId::U8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint8_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::I8: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int8_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::I16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int16_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::U16: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint16_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::I32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int32_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::U32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint32_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::I64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int64_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::U64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint64_t>(storeValue));
                        return;
                    }
                    case panda_file::Type::TypeId::F32: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F32);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, storeValue);
                        return;
                    }
                    case panda_file::Type::TypeId::F64: {
                        ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F64);
                        handleObj.GetPtr()->SetFieldPrimitive(*field, storeValue);
                        return;
                    }
                    default: {
                        UNREACHABLE();
                        return;
                    }
                }
            }
        }

        auto callee = TryGetCallee<FIELD_TYPE, false>(method, rawField, pc, klass);
        if (callee != nullptr) {
            PandaVector<Value> args {Value(handleObj.GetPtr()), Value(storeValue)};
            callee->Invoke(Coroutine::GetCurrent(), args.data());
            return;
        }
    }
    LookUpException<true>(klass, rawField);
    UNREACHABLE();
}

void CompilerEtsStObjByNameRef(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                               panda::ObjectHeader *storeValue)
{
    ASSERT(method != nullptr);
    panda::Class *klass;
    panda::Field *rawField;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, obj);
        VMHandle<ObjectHeader> handleStore(thread, storeValue);
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<panda::Class *>(obj->ClassAddr<panda::BaseClass>());
        rawField = classLinker->GetField(*method, panda_file::File::EntityId(id));

        auto field = TryGetField<panda_file::Type::TypeId::REFERENCE>(method, rawField, pc, klass);
        if (field != nullptr) {
            return handleObj.GetPtr()->SetFieldObject(*field, handleStore.GetPtr());
        }

        auto callee = TryGetCallee<panda_file::Type::TypeId::REFERENCE, false>(method, rawField, pc, klass);
        if (callee != nullptr) {
            PandaVector<Value> args {Value(handleObj.GetPtr()), Value(handleStore.GetPtr())};
            callee->Invoke(Coroutine::GetCurrent(), args.data());
            return;
        }
    }
    LookUpException<true>(klass, rawField);
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
                                          int32_t storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameI64(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          int64_t storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I64, int64_t>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameF32(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          float storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::F32, float>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameF64(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          double storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::F64, double>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameObj(panda::Method *method, int32_t id, uint32_t pc, panda::ObjectHeader *obj,
                                          panda::ObjectHeader *storeValue)
{
    CompilerEtsStObjByNameRef(method, id, pc, obj, storeValue);
}

extern "C" panda::ObjectHeader *CompilerEtsLdundefined()
{
    return panda::ets::EtsCoroutine::GetCurrent()->GetUndefinedObject();
}

}  // namespace panda::ets::intrinsics
