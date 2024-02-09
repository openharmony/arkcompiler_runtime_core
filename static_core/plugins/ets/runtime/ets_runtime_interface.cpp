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

#include <string>

#include "ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"

namespace ark::ets {
compiler::RuntimeInterface::ClassPtr EtsRuntimeInterface::GetClass(MethodPtr method, IdType id) const
{
    if (id == RuntimeInterface::MEM_PROMISE_CLASS_ID) {
        ScopedMutatorLock lock;
        auto *caller = MethodCast(method);
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
        return static_cast<EtsClassLinkerExtension *>(Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx))
            ->GetPromiseClass();
    }
    return PandaRuntimeInterface::GetClass(method, id);
}

compiler::RuntimeInterface::FieldPtr EtsRuntimeInterface::ResolveLookUpField(FieldPtr rawField, ClassPtr klass)
{
    ScopedMutatorLock lock;
    ASSERT(rawField != nullptr);
    ASSERT(klass != nullptr);
    return ClassCast(klass)->LookupFieldByName(FieldCast(rawField)->GetName());
}

template <panda_file::Type::TypeId FIELD_TYPE>
compiler::RuntimeInterface::MethodPtr EtsRuntimeInterface::GetLookUpCall(FieldPtr rawField, ClassPtr klass,
                                                                         bool isSetter)
{
    if (isSetter) {
        return ClassCast(klass)->LookupSetterByName<FIELD_TYPE>(FieldCast(rawField)->GetName());
    }
    return ClassCast(klass)->LookupGetterByName<FIELD_TYPE>(FieldCast(rawField)->GetName());
}

compiler::RuntimeInterface::MethodPtr EtsRuntimeInterface::ResolveLookUpCall(FieldPtr rawField, ClassPtr klass,
                                                                             bool isSetter)
{
    ScopedMutatorLock lock;
    ASSERT(rawField != nullptr);
    ASSERT(klass != nullptr);
    switch (FieldCast(rawField)->GetTypeId()) {
        case panda_file::Type::TypeId::U1:
            return GetLookUpCall<panda_file::Type::TypeId::U1>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U8:
            return GetLookUpCall<panda_file::Type::TypeId::U8>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I8:
            return GetLookUpCall<panda_file::Type::TypeId::I8>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I16:
            return GetLookUpCall<panda_file::Type::TypeId::I16>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U16:
            return GetLookUpCall<panda_file::Type::TypeId::U16>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I32:
            return GetLookUpCall<panda_file::Type::TypeId::I32>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U32:
            return GetLookUpCall<panda_file::Type::TypeId::U32>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I64:
            return GetLookUpCall<panda_file::Type::TypeId::I64>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U64:
            return GetLookUpCall<panda_file::Type::TypeId::U64>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::F32:
            return GetLookUpCall<panda_file::Type::TypeId::F32>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::F64:
            return GetLookUpCall<panda_file::Type::TypeId::F64>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::REFERENCE:
            return GetLookUpCall<panda_file::Type::TypeId::REFERENCE>(rawField, klass, isSetter);
        default: {
            UNREACHABLE();
            break;
        }
    }
    return nullptr;
}

uint64_t EtsRuntimeInterface::GetUndefinedObject() const
{
    return ToUintPtr(PandaEtsVM::GetCurrent()->GetUndefinedObject());
}

compiler::RuntimeInterface::InteropCallKind EtsRuntimeInterface::GetInteropCallKind(MethodPtr methodPtr) const
{
    auto className = GetClassNameFromMethod(methodPtr);
    auto classNameSuffix = className.substr(className.find_last_of('.') + 1);
    if (classNameSuffix == "$jsnew") {
        return InteropCallKind::NEW_INSTANCE;
    }
    if (classNameSuffix != "$jscall") {
        return InteropCallKind::UNKNOWN;
    }

    auto method = MethodCast(methodPtr);
    auto pf = method->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*method);
    auto linkerCtx = static_cast<EtsClassLinkerExtension *>(classLinker->GetExtension(ctx))->GetBootContext();

    ScopedMutatorLock lock;

    ASSERT(method->GetArgType(0).IsReference());  // arg0 is always a reference
    ASSERT(method->GetArgType(1).IsReference());  // arg1 is always a reference
    uint32_t const argReftypeShift = method->GetReturnType().IsReference() ? 1 : 0;
    auto cls = classLinker->GetClass(*pf, pda.GetReferenceType(1 + argReftypeShift), linkerCtx);
    if (cls->IsStringClass()) {
        return InteropCallKind::CALL;
    }
    return InteropCallKind::CALL_BY_VALUE;
}

char *EtsRuntimeInterface::GetFuncPropName(MethodPtr methodPtr, uint32_t strId) const
{
    auto method = MethodCast(methodPtr);
    auto pf = method->GetPandaFile();
    auto str = reinterpret_cast<const char *>(pf->GetStringData(ark::panda_file::File::EntityId(strId)).data);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return const_cast<char *>(std::strrchr(str, '.') + 1);
}

uint64_t EtsRuntimeInterface::GetFuncPropNameOffset(MethodPtr methodPtr, uint32_t strId) const
{
    auto pf = MethodCast(methodPtr)->GetPandaFile();
    auto str = GetFuncPropName(methodPtr, strId);
    return reinterpret_cast<uint64_t>(str) - reinterpret_cast<uint64_t>(pf->GetBase());
}

bool EtsRuntimeInterface::IsMethodStringBuilderConstructorWithStringArg(MethodPtr method) const
{
    return MethodCast(method)->IsConstructor() && GetClassNameFromMethod(method) == "std.core.StringBuilder" &&
           MethodCast(method)->GetProto().GetSignature() == "(Lstd/core/String;)V";
}

bool EtsRuntimeInterface::IsMethodStringBuilderToString(MethodPtr method) const
{
    return GetMethodFullName(method, false) == "std.core.StringBuilder::toString" &&
           MethodCast(method)->GetProto().GetSignature() == "()Lstd/core/String;";
}

bool EtsRuntimeInterface::IsIntrinsicStringBuilderToString(IntrinsicId id) const
{
    return id == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_TO_STRING;
}

}  // namespace ark::ets
