/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include "file.h"
#include "handle_scope.h"
#include "include/coretypes/class.h"
#include "include/mem/panda_string.h"
#include "include/mtmanaged_thread.h"
#include "intrinsics.h"
#include "macros.h"
#include "mem/mem.h"
#include "mem/vm_handle.h"
#include "modifiers.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "types/ets_array.h"
#include "types/ets_box_primitive-inl.h"
#include "types/ets_class.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_type.h"
#include "types/ets_type_comptime_traits.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_field.h"
#include "types/ets_typeapi_method.h"
#include "types/ets_typeapi_parameter.h"

namespace ark::ets::intrinsics {

// General
EtsString *TypeAPIGetTypeDescriptor(EtsObject *object)
{
    if (object == nullptr) {
        return EtsString::CreateFromMUtf8(NULL_TYPE_DESC);
    }
    return EtsString::CreateFromMUtf8(object->GetClass()->GetDescriptor());
}

static EtsByte GetRefTypeKind(const PandaString &td, const EtsClass *refType)
{
    EtsByte result;
    if (td == ark::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::BOOLEAN);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_BYTE) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::BYTE);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_CHAR) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::CHAR);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_SHORT) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::SHORT);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_INT) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::INT);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_LONG) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::LONG);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::FLOAT);
    } else if (td == ark::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::DOUBLE);
    } else if (refType->IsFunctionalClass()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::FUNCTIONAL);
    } else if (refType->IsInterface()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::INTERFACE);
    } else if (refType->IsArrayClass()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::ARRAY);
    } else if (refType->IsTupleClass()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::TUPLE);
    } else if (refType->IsStringClass()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::STRING);
    } else if (refType->IsUnionClass()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::UNION);
    } else if (refType->IsUndefined()) {
        result = static_cast<EtsByte>(EtsTypeAPIKind::UNDEFINED);
    } else {
        result = static_cast<EtsByte>(EtsTypeAPIKind::CLASS);
    }
    return result;
}

static EtsByte GetValTypeKind(EtsValueTypeDesc td)
{
    EtsByte kind;
    switch (td) {
        case EtsValueTypeDesc::BOOLEAN:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::BOOLEAN) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::BYTE:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::BYTE) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::CHAR:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::CHAR) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::SHORT:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::SHORT) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::INT:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::INT) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::LONG:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::LONG) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::FLOAT:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::FLOAT) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case EtsValueTypeDesc::DOUBLE:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::DOUBLE) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        default:
            kind = static_cast<EtsByte>(EtsTypeAPIKind::NONE);
    }
    return kind;
}

EtsByte TypeAPIGetTypeKind(EtsString *td)
{
    auto typeDesc = td->GetMutf8();
    // Is Null?
    if (typeDesc == NULL_TYPE_DESC) {
        return static_cast<EtsByte>(EtsTypeAPIKind::NUL);
    }
    // Is Function for methods, because currently there is no representation of them in runtime
    if (typeDesc[0] == METHOD_PREFIX) {
        return static_cast<EtsByte>(EtsTypeAPIKind::METHOD);
    }
    // Is machine-level void
    if (typeDesc[0] == VOID_PRIMITIVE_TYPE_DESC) {
        return static_cast<EtsByte>(EtsTypeAPIKind::VOID);
    }
    // Is RefType?
    if (typeDesc[0] == CLASS_TYPE_PREFIX || typeDesc[0] == ARRAY_TYPE_PREFIX) {
        auto refType = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClass(typeDesc.c_str());
        if (refType == nullptr) {
            return static_cast<EtsByte>(EtsTypeAPIKind::NONE);
        }
        PandaEtsVM::GetCurrent()->GetClassLinker()->InitializeClass(EtsCoroutine::GetCurrent(), refType);
        return GetRefTypeKind(typeDesc, refType);
    }
    // Is ValueType?
    return GetValTypeKind(static_cast<EtsValueTypeDesc>(typeDesc[0]));
}

EtsBoolean TypeAPIIsValueType(EtsString *td)
{
    // NOTE(shumilov-petr): Add td is valid check
    return static_cast<EtsBoolean>(
        !((static_cast<uint8_t>(TypeAPIGetTypeKind(td)) & static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK)) == 0));
}

EtsString *TypeAPIGetTypeName(EtsString *td)
{
    // NOTE(shumilov-petr): Add td is valid check
    auto className = td->GetMutf8();
    auto type = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClass(className.c_str());
    return EtsClass::CreateEtsClassName(type->GetDescriptor());
}

EtsInt TypeAPIGetClassAttributes(EtsString *td)
{
    // NOTE(shumilov-petr): Add td is valid check
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());

    uint32_t attrs = 0;
    attrs |= (type->IsFinal()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::FINAL) : 0U;

    return static_cast<EtsInt>(attrs);
}

// Features
EtsLong TypeAPIGetFieldsNum(EtsString *td)
{
    // NOTE(shumilov-petr): Add td is valid check
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    return type->GetFieldsNumber();
}

EtsLong TypeAPIGetOwnFieldsNum(EtsString *td)
{
    // NOTE(shumilov-petr): Add td is valid check
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    return type->GetOwnFieldsNumber();
}

EtsTypeAPIField *CreateField(EtsField *field, EtsClass *type)
{
    // NOTE(shumilov-petr): Add td is valid check

    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    // Make the instance of Type API Field
    VMHandle<EtsTypeAPIField> typeapiField(coroutine, EtsTypeAPIField::Create(coroutine));

    auto td = EtsString::CreateFromMUtf8(field->GetTypeDescriptor());
    VMHandle<EtsString> tdHandle(coroutine, td->GetCoreType());
    auto ownerTd = EtsString::CreateFromMUtf8(field->GetDeclaringClass()->GetDescriptor());
    VMHandle<EtsString> ownerTdHandle(coroutine, ownerTd->GetCoreType());
    auto name = field->GetNameString();
    // Set field's type, field's owner type and name
    typeapiField->SetTypeDesc(tdHandle.GetPtr());
    typeapiField->SetOwnerTypeDesc(ownerTdHandle.GetPtr());
    typeapiField->SetName(name);

    // Set Access Modifier
    EtsTypeAPIAccessModifier accessMod;
    if (field->IsPublic()) {
        accessMod = EtsTypeAPIAccessModifier::PUBLIC;
    } else if (field->IsPrivate()) {
        accessMod = EtsTypeAPIAccessModifier::PRIVATE;
    } else {
        accessMod = EtsTypeAPIAccessModifier::PROTECTED;
    }
    typeapiField->SetAccessMod(accessMod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (field->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (!field->IsDeclaredIn(type)) ? static_cast<uint32_t>(EtsTypeAPIAttributes::INHERITED) : 0U;
    attr |= (field->IsReadonly()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::READONLY) : 0U;

    typeapiField->SetAttributes(attr);

    return typeapiField.GetPtr();
}

EtsTypeAPIField *TypeAPIGetField(EtsString *td, EtsLong idx)
{
    // Resolve type by td
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    EtsField *field = type->GetFieldByIndex(idx);
    ASSERT(field != nullptr);
    return CreateField(field, type);
}

EtsTypeAPIField *TypeAPIGetOwnField(EtsString *td, EtsLong idx)
{
    // Resolve type by td
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    EtsField *field = type->GetOwnFieldByIndex(idx);
    ASSERT(field != nullptr);
    return CreateField(field, type);
}

EtsTypeAPIField *TypeAPIGetFieldByName(EtsString *td, EtsString *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> fnamePtr(coroutine, name->GetCoreType());

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());

    auto instanceField = type->GetFieldIDByName(fnamePtr.GetPtr()->GetMutf8().c_str());
    if (instanceField != nullptr) {
        return CreateField(instanceField, type);
    }
    auto staticField = type->GetStaticFieldIDByName(fnamePtr.GetPtr()->GetMutf8().c_str());
    return CreateField(staticField, type);
}

EtsObject *TypeAPIGetStaticFieldValue(EtsString *ownerTd, EtsString *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> fnamePtr(coroutine, name->GetCoreType());

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto ownerType = classLinker->GetClass(ownerTd->GetMutf8().c_str());

    auto field = ownerType->GetFieldIDByName(fnamePtr.GetPtr()->GetMutf8().c_str());
    if (field == nullptr) {
        field = ownerType->GetStaticFieldIDByName(fnamePtr.GetPtr()->GetMutf8().c_str());
    }
    ASSERT(field != nullptr && field->IsStatic());

    if (!field->IsStatic()) {
        UNREACHABLE();
    }

    if (field->GetType()->IsPrimitive()) {
        return EtsPrimitiveTypeEnumToComptimeConstant(
            // CC-OFFNXT(G.FMT.14-CPP) project code style
            ConvertPandaTypeToEtsType(field->GetCoreType()->GetType()), [&](auto type) -> EtsObject * {
                using T = EtsTypeEnumToCppType<decltype(type)::value>;
                auto val = ownerType->GetRuntimeClass()->GetFieldPrimitive<T>(*field->GetCoreType());
                // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                return EtsBoxPrimitive<T>::Create(coroutine, val);
            });
    }

    return EtsObject::FromCoreType(ownerType->GetRuntimeClass()->GetFieldObject(*field->GetCoreType()));
}

void TypeAPISetStaticFieldValue(EtsString *ownerTd, EtsString *name, EtsObject *v)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsString> fnamePtr(coroutine, name->GetCoreType());
    VMHandle<EtsObject> valuePtr(coroutine, v->GetCoreType());

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto ownerType = classLinker->GetClass(ownerTd->GetMutf8().c_str());

    auto field = ownerType->GetFieldIDByName(fnamePtr.GetPtr()->GetMutf8().c_str());
    if (field == nullptr) {
        field = ownerType->GetStaticFieldIDByName(fnamePtr.GetPtr()->GetMutf8().c_str());
    }
    ASSERT(field != nullptr && field->IsStatic());

    if (!field->IsStatic()) {
        UNREACHABLE();
    }

    if (field->GetType()->IsPrimitive()) {
        EtsPrimitiveTypeEnumToComptimeConstant(
            ConvertPandaTypeToEtsType(field->GetCoreType()->GetType()), [&](auto type) {
                using T = EtsTypeEnumToCppType<decltype(type)::value>;
                // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                ownerType->GetRuntimeClass()->SetFieldPrimitive<T>(
                    *field->GetCoreType(), EtsBoxPrimitive<T>::FromCoreType(valuePtr.GetPtr())->GetValue());
            });
    } else {
        ownerType->GetRuntimeClass()->SetFieldObject(*field->GetCoreType(), valuePtr.GetPtr()->GetCoreType());
    }
}

EtsLong TypeAPIGetMethodsNum(EtsString *td)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    return type->GetMethodsNum();
}

EtsLong TypeAPIGetConstructorsNum(EtsString *td)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    auto methods = type->GetConstructors();
    return methods.size();
}

EtsTypeAPIMethod *CreateMethod(EtsMethod *method, EtsClass *type)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsTypeAPIMethod> typeapiMethod(coroutine, EtsTypeAPIMethod::Create(coroutine));

    // Set Type Descriptor
    typeapiMethod.GetPtr()->SetTypeDesc(method->GetDescriptor().c_str());
    EtsString *name;
    if (method->IsInstanceConstructor()) {
        name = EtsString::CreateFromMUtf8(CONSTRUCTOR_NAME);
    } else {
        name = method->GetNameString();
    }
    typeapiMethod.GetPtr()->SetName(name);

    // Set Access Modifier
    EtsTypeAPIAccessModifier accessMod;
    if (method->IsPublic()) {
        accessMod = EtsTypeAPIAccessModifier::PUBLIC;
    } else if (method->IsPrivate()) {
        accessMod = EtsTypeAPIAccessModifier::PRIVATE;
    } else {
        accessMod = EtsTypeAPIAccessModifier::PROTECTED;
    }
    typeapiMethod.GetPtr()->SetAccessMod(accessMod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (method->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (method->IsConstructor()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::CONSTRUCTOR) : 0U;
    attr |= (method->IsAbstract()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ABSTRACT) : 0U;
    attr |= (!method->IsDeclaredIn(type)) ? static_cast<uint32_t>(EtsTypeAPIAttributes::INHERITED) : 0U;
    attr |= (method->IsGetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::GETTER) : 0U;
    attr |= (method->IsSetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::SETTER) : 0U;

    typeapiMethod.GetPtr()->SetAttributes(attr);
    return typeapiMethod.GetPtr();
}

EtsTypeAPIMethod *TypeAPIGetMethod(EtsString *td, EtsLong i)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    return CreateMethod(type->GetMethodByIndex(i), type);
}

EtsTypeAPIMethod *TypeAPIGetConstructor(EtsString *td, EtsLong i)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    auto constructors = type->GetConstructors();
    ASSERT(0 <= i && i < (EtsLong)constructors.size());
    return CreateMethod(constructors[i], type);
}

EtsLong TypeAPIGetInterfacesNum(EtsString *td)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    return type->GetRuntimeClass()->GetInterfaces().size();
}

EtsString *TypeAPIGetInterface(EtsString *td, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = classLinker->GetClass(td->GetMutf8().c_str());
    auto interfaces = type->GetRuntimeClass()->GetInterfaces();
    ASSERT(0 <= i && i <= (EtsLong)interfaces.size());
    auto ifaceType = EtsClass::FromRuntimeClass(interfaces[i]);
    return EtsString::CreateFromMUtf8(ifaceType->GetDescriptor());
}

EtsInt TypeAPIGetFunctionAttributes([[maybe_unused]] EtsString *td)
{
    // NOTE(shumilov-petr): Not implemented
    return 0;
}

EtsLong TypeAPIGetParametersNum(EtsString *td)
{
    auto function = EtsMethod::FromTypeDescriptor(td->GetMutf8());
    return function->GetParametersNum();
}

EtsTypeAPIParameter *CreateParameter(EtsClass *type, std::string_view name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsTypeAPIParameter> typeapiParameter(coroutine, EtsTypeAPIParameter::Create(coroutine));

    // Set Type Descriptor
    auto td = EtsString::CreateFromMUtf8(type->GetDescriptor());
    VMHandle<EtsString> tdHandle(coroutine, td->GetCoreType());
    typeapiParameter.GetPtr()->SetTypeDesc(tdHandle.GetPtr());

    // NOTE(shumilov-petr): It's a temporary solution, extra type info dumping required
    auto pname = EtsString::CreateFromUtf8(name.data(), name.size());
    VMHandle<EtsString> pnameHandle(coroutine, pname->GetCoreType());
    typeapiParameter.GetPtr()->SetName(pnameHandle.GetPtr());

    // Set specific attributes
    uint32_t attr = 0U;
    // NOTE(kirill-mitkin): Need to dump attributes of parameters from frontend to runtime
    typeapiParameter.GetPtr()->SetAttributes(attr);
    return typeapiParameter.GetPtr();
}

EtsTypeAPIParameter *TypeAPIGetParameter(EtsString *td, EtsLong i)
{
    auto function = EtsMethod::FromTypeDescriptor(td->GetMutf8());
    EtsClass *type = nullptr;
    if (function->IsStatic()) {
        type = function->ResolveArgType(i);
    } else {
        // 0 is recevier type
        type = function->ResolveArgType(i + 1);
    }
    return CreateParameter(type, std::to_string(i));
}

EtsString *TypeAPIGetResultType(EtsString *td)
{
    auto function = EtsMethod::FromTypeDescriptor(td->GetMutf8());
    auto type = function->GetReturnTypeDescriptor().data();
    return EtsString::CreateFromMUtf8(type);
}

EtsString *TypeAPIGetReceiverType(EtsString *td)
{
    auto function = EtsMethod::FromTypeDescriptor(td->GetMutf8());
    ASSERT(!function->IsStatic());
    auto type = function->ResolveArgType(0);
    return EtsString::CreateFromMUtf8(type->GetDescriptor());
}

EtsLong TypeAPIGetTypeId(EtsString *td)
{
    auto typeDesc = td->GetMutf8();
    // Create Null class in runtime
    if (typeDesc == NULL_TYPE_DESC) {
        return 0;
    }
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto objType = classLinker->GetClass(typeDesc.c_str());
    return objType->GetRuntimeClass()->GetUniqId();
}

EtsString *TypeAPIGetArrayElementType(EtsString *td)
{
    // NOTE(shumilov-petr): Add td is valid check
    auto arrClass = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClass(td->GetMutf8().c_str());
    return EtsString::CreateFromMUtf8(arrClass->GetComponentType()->GetDescriptor());
}

EtsObject *MakeClassInstance(EtsString *td)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto typeClass = classLinker->GetClass(td->GetMutf8().c_str());

    ASSERT(!typeClass->IsArrayClass());

    if (typeClass->IsStringClass()) {
        return EtsString::CreateNewEmptyString()->AsObject();
    }

    VMHandle<EtsObject> objHandle(coroutine, EtsObject::Create(typeClass)->GetCoreType());
    auto hasDefaultConstr = false;
    typeClass->EnumerateMethods([&](EtsMethod *method) {
        if (method->IsConstructor() && method->GetParametersNum() == 0) {
            std::array<Value, 1> args {Value(objHandle.GetPtr()->GetCoreType())};
            method->GetPandaMethod()->InvokeVoid(coroutine, args.data());
            hasDefaultConstr = true;
            return true;
        }
        return false;
    });
    ASSERT(hasDefaultConstr);
    return objHandle.GetPtr();
}

EtsObject *TypeAPIMakeArrayInstance(EtsString *td, EtsLong len)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsString> tdHandle(coroutine, td->GetCoreType());

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto typeDesc = tdHandle->GetMutf8();
    auto typeClass = classLinker->GetClass(typeDesc.c_str());

    auto valType = ark::panda_file::Type::GetTypeIdBySignature(typeDesc[0]);
    switch (valType.GetId()) {
        case panda_file::Type::TypeId::U1:
            return EtsBooleanArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::I8:
            return EtsByteArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::U16:
            return EtsCharArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::I16:
            return EtsShortArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::I32:
            return EtsIntArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::I64:
            return EtsLongArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::F32:
            return EtsFloatArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::F64:
            return EtsDoubleArray::Create(len)->AsObject();
        case panda_file::Type::TypeId::REFERENCE: {
            VMHandle<EtsObjectArray> arrayHandle(coroutine, EtsObjectArray::Create(typeClass, len)->GetCoreType());
            for (size_t i = 0; i < arrayHandle->GetLength(); ++i) {
                VMHandle<EtsObject> elementHandle(coroutine, MakeClassInstance(tdHandle.GetPtr())->GetCoreType());
                arrayHandle->Set(i, elementHandle.GetPtr());
            }
            return arrayHandle->AsObject();
        }
        default:
            return nullptr;
    }
}

EtsString *TypeAPIGetBaseType(EtsString *td)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto typeClass = classLinker->GetClass(td->GetMutf8().c_str());
    auto baseClass = typeClass->GetBase();
    if (baseClass == nullptr) {
        return EtsString::CreateFromMUtf8(classLinker->GetObjectClass()->GetDescriptor());
    }
    return EtsString::CreateFromMUtf8(baseClass->GetDescriptor());
}

EtsBoolean TypeAPIIsInheritedFrom(EtsString *ltd, EtsString *rtd)
{
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope {coro};
    EtsHandle rtdHandle {coro, rtd};
    EtsHandle lHandle {coro, classLinker->GetClass(ltd->GetMutf8().c_str())};
    auto r = classLinker->GetClass(rtdHandle->GetMutf8().c_str());
    return static_cast<EtsBoolean>(lHandle->GetRuntimeClass()->Implements(r->GetRuntimeClass()));
}

}  // namespace ark::ets::intrinsics
