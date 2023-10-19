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

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include "file.h"
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
#include "types/ets_void.h"
#include "types/ets_type.h"
#include "types/ets_type_comptime_traits.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_field.h"
#include "types/ets_typeapi_method.h"
#include "types/ets_typeapi_parameter.h"

namespace panda::ets::intrinsics {

// General
EtsString *TypeAPIGetTypeDescriptor(EtsObject *object)
{
    if (object == nullptr) {
        return EtsString::CreateFromMUtf8(NULL_TYPE_DESC);
    }
    return EtsString::CreateFromMUtf8(object->GetClass()->GetDescriptor());
}

EtsByte TypeAPIGetTypeKind(EtsString *td)
{
    auto type_desc = td->GetMutf8();

    auto kind = static_cast<EtsByte>(EtsTypeAPIKind::NONE);

    // Is Null?
    if (type_desc == NULL_TYPE_DESC) {
        return static_cast<EtsByte>(EtsTypeAPIKind::NUL);
    }
    // Is Function for methods, because currently there is no representation of them in runtime
    if (type_desc[0] == METHOD_PREFIX) {
        return static_cast<EtsByte>(EtsTypeAPIKind::METHOD);
    }

    // Is RefType?
    if (type_desc[0] == CLASS_TYPE_PREFIX || type_desc[0] == ARRAY_TYPE_PREFIX) {
        auto ref_type = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClass(type_desc.c_str());
        ASSERT(ref_type != nullptr);
        PandaEtsVM::GetCurrent()->GetClassLinker()->InitializeClass(EtsCoroutine::GetCurrent(), ref_type);

        if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::BOOLEAN);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BYTE) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::BYTE);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_CHAR) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::CHAR);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_SHORT) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::SHORT);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_INT) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::INT);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_LONG) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::LONG);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::FLOAT);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::DOUBLE);
        } else if (ref_type->IsInterface()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::INTERFACE);
        } else if (ref_type->IsArrayClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::ARRAY);
        } else if (ref_type->IsTupleClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::TUPLE);
        } else if (ref_type->IsStringClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::STRING);
        } else if (ref_type->IsLambdaClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::LAMBDA);
        } else if (ref_type->IsUnionClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::UNION);
        } else if (ref_type->IsUndefined()) {  // TODO(shumilov-petr): think about it
            kind = static_cast<EtsByte>(EtsTypeAPIKind::UNDEFINED);
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::VOID) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::VOID);
        } else {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::CLASS);
        }

        return kind;
    }

    // Is ValueType?
    auto val_type = panda::panda_file::Type::GetTypeIdBySignature(type_desc[0]);
    switch (val_type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            kind = static_cast<EtsByte>(EtsTypeAPIKind::VOID);
            break;
        case panda_file::Type::TypeId::U1:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::BOOLEAN) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::I8:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::BYTE) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::U16:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::CHAR) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::I16:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::SHORT) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::I32:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::INT) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::I64:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::LONG) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::F32:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::FLOAT) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        case panda_file::Type::TypeId::F64:
            kind = static_cast<uint8_t>(EtsTypeAPIKind::DOUBLE) | static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK);
            break;
        default:
            kind = static_cast<EtsByte>(EtsTypeAPIKind::NONE);
    }
    return kind;
}

EtsBoolean TypeAPIIsValueType(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check
    return static_cast<EtsBoolean>(
        !((static_cast<uint8_t>(TypeAPIGetTypeKind(td)) & static_cast<uint8_t>(ETS_TYPE_KIND_VALUE_MASK)) == 0));
}

EtsString *TypeAPIGetTypeName(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check
    auto class_name = td->GetMutf8();
    auto type = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClass(class_name.c_str());
    return EtsClass::CreateEtsClassName(type->GetDescriptor());
}

EtsInt TypeAPIGetClassAttributes(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());

    uint32_t attrs = 0;
    attrs |= (type->IsFinal()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::FINAL) : 0U;

    return static_cast<EtsInt>(attrs);
}

// Features
EtsLong TypeAPIGetFieldsNum(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    return type->GetFieldsNumber();
}

EtsLong TypeAPIGetOwnFieldsNum(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    return type->GetOwnFieldsNumber();
}

EtsTypeAPIField *CreateField(EtsField *field, EtsClass *type)
{
    // TODO(shumilov-petr): Add td is valid check

    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    // Make the instance of Type API Field
    VMHandle<EtsTypeAPIField> typeapi_field(coroutine, EtsTypeAPIField::Create(coroutine));

    auto td = EtsString::CreateFromMUtf8(field->GetTypeDescriptor());
    VMHandle<EtsString> td_handle(coroutine, td->GetCoreType());
    auto owner_td = EtsString::CreateFromMUtf8(field->GetDeclaringClass()->GetDescriptor());
    VMHandle<EtsString> owner_td_handle(coroutine, owner_td->GetCoreType());
    auto name = field->GetNameString();
    // Set field's type, field's owner type and name
    typeapi_field->SetTypeDesc(td_handle.GetPtr());
    typeapi_field->SetOwnerTypeDesc(owner_td_handle.GetPtr());
    typeapi_field->SetName(name);

    // Set Access Modifier
    EtsTypeAPIAccessModifier access_mod;
    if (field->IsPublic()) {
        access_mod = EtsTypeAPIAccessModifier::PUBLIC;
    } else if (field->IsPrivate()) {
        access_mod = EtsTypeAPIAccessModifier::PRIVATE;
    } else {
        access_mod = EtsTypeAPIAccessModifier::PROTECTED;
    }
    typeapi_field->SetAccessMod(access_mod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (field->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (!field->IsDeclaredIn(type)) ? static_cast<uint32_t>(EtsTypeAPIAttributes::INHERITED) : 0U;
    attr |= (field->IsReadonly()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::READONLY) : 0U;

    typeapi_field->SetAttributes(attr);

    return typeapi_field.GetPtr();
}

EtsTypeAPIField *TypeAPIGetField(EtsString *td, EtsLong idx)
{
    // Resolve type by td
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    EtsField *field = type->GetFieldByIndex(idx);
    ASSERT(field != nullptr);
    return CreateField(field, type);
}

EtsTypeAPIField *TypeAPIGetOwnField(EtsString *td, EtsLong idx)
{
    // Resolve type by td
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    EtsField *field = type->GetOwnFieldByIndex(idx);
    ASSERT(field != nullptr);
    return CreateField(field, type);
}

EtsTypeAPIField *TypeAPIGetFieldByName(EtsString *td, EtsString *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> fname_ptr(coroutine, name->GetCoreType());

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());

    auto instance_field = type->GetFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str());
    if (instance_field != nullptr) {
        return CreateField(instance_field, type);
    }
    auto static_field = type->GetStaticFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str());
    return CreateField(static_field, type);
}

EtsObject *TypeAPIGetStaticFieldValue(EtsString *owner_td, EtsString *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> fname_ptr(coroutine, name->GetCoreType());

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto owner_type = class_linker->GetClass(owner_td->GetMutf8().c_str());

    auto field = owner_type->GetFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str());
    if (field == nullptr) {
        field = owner_type->GetStaticFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str());
    }
    ASSERT(field != nullptr && field->IsStatic());

    if (!field->IsStatic()) {
        UNREACHABLE();
    }

    if (field->GetType()->IsPrimitive()) {
        return EtsPrimitiveTypeEnumToComptimeConstant(
            ConvertPandaTypeToEtsType(field->GetCoreType()->GetType()), [&](auto type) -> EtsObject * {
                using T = EtsTypeEnumToCppType<decltype(type)::value>;
                auto val = owner_type->GetRuntimeClass()->GetFieldPrimitive<T>(*field->GetCoreType());
                // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                return EtsBoxPrimitive<T>::Create(EtsCoroutine::GetCurrent(), val);
            });
    }

    return EtsObject::FromCoreType(owner_type->GetRuntimeClass()->GetFieldObject(*field->GetCoreType()));
}

EtsVoid *TypeAPISetStaticFieldValue(EtsString *owner_td, EtsString *name, EtsObject *v)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> fname_ptr(coroutine, name->GetCoreType());
    VMHandle<EtsObject> value_ptr(coroutine, v->GetCoreType());

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto owner_type = class_linker->GetClass(owner_td->GetMutf8().c_str());

    auto field = owner_type->GetFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str());
    if (field == nullptr) {
        field = owner_type->GetStaticFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str());
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
                owner_type->GetRuntimeClass()->SetFieldPrimitive<T>(
                    *field->GetCoreType(), EtsBoxPrimitive<T>::FromCoreType(value_ptr.GetPtr())->GetValue());
            });
    } else {
        owner_type->GetRuntimeClass()->SetFieldObject(*field->GetCoreType(), value_ptr.GetPtr()->GetCoreType());
    }
    return EtsVoid::GetInstance();
}

EtsLong TypeAPIGetMethodsNum(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    return type->GetMethodsNum();
}

EtsLong TypeAPIGetConstructorsNum(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    auto methods = type->GetConstructors();
    return methods.size();
}

EtsTypeAPIMethod *CreateMethod(EtsMethod *method, EtsClass *type)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsTypeAPIMethod> typeapi_method(coroutine, EtsTypeAPIMethod::Create(coroutine));

    // Set Type Descriptor
    typeapi_method.GetPtr()->SetTypeDesc(method->GetDescriptor().c_str());
    EtsString *name;
    if (method->IsConstructor()) {
        name = EtsString::CreateFromMUtf8("constructor");
    } else {
        name = method->GetNameString();
    }
    typeapi_method.GetPtr()->SetName(name);

    // Set Access Modifier
    EtsTypeAPIAccessModifier access_mod;
    if (method->IsPublic()) {
        access_mod = EtsTypeAPIAccessModifier::PUBLIC;
    } else if (method->IsPrivate()) {
        access_mod = EtsTypeAPIAccessModifier::PRIVATE;
    } else {
        access_mod = EtsTypeAPIAccessModifier::PROTECTED;
    }
    typeapi_method.GetPtr()->SetAccessMod(access_mod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (method->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (method->IsConstructor()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::CONSTRUCTOR) : 0U;
    attr |= (method->IsAbstract()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ABSTRACT) : 0U;
    attr |= (method->IsDeclaredIn(type)) ? static_cast<uint32_t>(EtsTypeAPIAttributes::INHERITED) : 0U;
    attr |= (method->IsGetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::GETTER) : 0U;
    attr |= (method->IsSetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::SETTER) : 0U;

    typeapi_method.GetPtr()->SetAttributes(attr);
    return typeapi_method.GetPtr();
}

EtsTypeAPIMethod *TypeAPIGetMethod(EtsString *td, EtsLong i)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    return CreateMethod(type->GetMethodByIndex(i), type);
}

EtsTypeAPIMethod *TypeAPIGetConstructor(EtsString *td, EtsLong i)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    auto constructors = type->GetConstructors();
    ASSERT(0 <= i && i < (EtsLong)constructors.size());
    return CreateMethod(constructors[i], type);
}

EtsLong TypeAPIGetInterfacesNum(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    return type->GetRuntimeClass()->GetInterfaces().size();
}

EtsString *TypeAPIGetInterface(EtsString *td, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = class_linker->GetClass(td->GetMutf8().c_str());
    auto interfaces = type->GetRuntimeClass()->GetInterfaces();
    ASSERT(0 <= i && i <= (EtsLong)interfaces.size());
    auto inter_type = EtsClass::FromRuntimeClass(interfaces[i]);
    return EtsString::CreateFromMUtf8(inter_type->GetDescriptor());
}

EtsInt TypeAPIGetFunctionAttributes([[maybe_unused]] EtsString *td)
{
    // TODO(shumilov-petr): Not implemented
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

    VMHandle<EtsTypeAPIParameter> typeapi_parameter(coroutine, EtsTypeAPIParameter::Create(coroutine));

    // Set Type Descriptor
    auto td = EtsString::CreateFromMUtf8(type->GetDescriptor());
    VMHandle<EtsString> td_handle(coroutine, td->GetCoreType());
    typeapi_parameter.GetPtr()->SetTypeDesc(td_handle.GetPtr());

    // TODO(shumilov-petr): It's a temporary solution, extra type info dumping required
    auto pname = EtsString::CreateFromUtf8(name.data(), name.size());
    VMHandle<EtsString> pname_handle(coroutine, pname->GetCoreType());
    typeapi_parameter.GetPtr()->SetName(pname_handle.GetPtr());

    // Set specific attributes
    uint32_t attr = 0U;
    // TODO(kirill-mitkin): Need to dump attributes of parameters from frontend to runtime
    typeapi_parameter.GetPtr()->SetAttributes(attr);
    return typeapi_parameter.GetPtr();
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
    auto type_desc = td->GetMutf8();
    // Create Null class in runtime
    if (type_desc == NULL_TYPE_DESC) {
        return 0;
    }
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(type_desc.c_str());
    return obj_type->GetRuntimeClass()->GetUniqId();
}

EtsString *TypeAPIGetArrayElementType(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check
    auto arr_class = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClass(td->GetMutf8().c_str());
    return EtsString::CreateFromMUtf8(arr_class->GetComponentType()->GetDescriptor());
}

EtsObject *TypeAPIMakeClassInstance(EtsString *td)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type_class = class_linker->GetClass(td->GetMutf8().c_str());
    VMHandle<EtsObject> obj_handle(coroutine, EtsObject::Create(type_class)->GetCoreType());
    auto has_default_constr = false;
    type_class->EnumerateMethods([&](EtsMethod *method) {
        if (method->IsConstructor() && method->GetParametersNum() == 0) {
            std::array<Value, 1> args {Value(obj_handle.GetPtr()->GetCoreType())};
            method->GetPandaMethod()->InvokeVoid(EtsCoroutine::GetCurrent(), args.data());
            has_default_constr = true;
            return true;
        }
        return false;
    });
    ASSERT(has_default_constr);
    return obj_handle.GetPtr();
}

EtsObject *TypeAPIMakeArrayInstance(EtsString *td, EtsLong len)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type_desc = td->GetMutf8();
    auto type_class = class_linker->GetClass(type_desc.c_str());

    auto val_type = panda::panda_file::Type::GetTypeIdBySignature(type_desc[0]);
    switch (val_type.GetId()) {
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
        case panda_file::Type::TypeId::REFERENCE:
            return EtsObjectArray::Create(type_class, len)->AsObject();
        default:
            return nullptr;
    }
}

EtsString *TypeAPIGetBaseType(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type_class = class_linker->GetClass(td->GetMutf8().c_str());
    auto base_class = type_class->GetBase();
    if (base_class == nullptr) {
        return EtsString::CreateFromMUtf8(class_linker->GetObjectClass()->GetDescriptor());
    }
    return EtsString::CreateFromMUtf8(base_class->GetDescriptor());
}
}  // namespace panda::ets::intrinsics
