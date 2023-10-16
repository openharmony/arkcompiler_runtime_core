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
#include "file.h"
#include "include/coretypes/class.h"
#include "include/mem/panda_string.h"
#include "include/mtmanaged_thread.h"
#include "intrinsics.h"
#include "mem/mem.h"
#include "mem/vm_handle.h"
#include "modifiers.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_type.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_feature.h"
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
    if (type_desc[0] == NULL_TYPE_PREFIX) {
        return static_cast<EtsByte>(EtsTypeAPIKind::NUL);
    }
    // Is Function for methods, because currently there is no representation of them in runtime
    if (type_desc[0] == METHOD_PREFIX) {
        return static_cast<EtsByte>(EtsTypeAPIKind::FUNCTION);
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
        } else if (ref_type->IsStringClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::STRING);
        } else if (ref_type->IsFunctionClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::FUNCTION);
        } else if (ref_type->IsUnionClass()) {
            kind = static_cast<EtsByte>(EtsTypeAPIKind::UNION);
        } else if (ref_type->IsUndefined()) {  // TODO(shumilov-petr): think about it
            kind = static_cast<EtsByte>(EtsTypeAPIKind::UNDEFINED);
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

// Features
EtsLong TypeAPIGetFieldsNum(EtsString *td)
{
    // TODO(shumilov-petr): Add td is valid check

    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());

    return obj_type->GetFieldsNumber();
}

EtsTypeAPIField *CreateField(EtsField *field)
{
    // TODO(shumilov-petr): Add td is valid check

    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    // Make the instance of Type API Field
    VMHandle<EtsTypeAPIField> typeapi_field(coroutine, EtsTypeAPIField::Create(coroutine));

    auto td = EtsString::CreateFromMUtf8(field->GetTypeDescriptor());
    VMHandle<EtsString> td_handle(coroutine, td->GetCoreType());
    auto name = field->GetNameString();
    // Set values
    typeapi_field->SetTypeDesc(td_handle.GetPtr());
    typeapi_field->SetName(name);

    // Set Access Modifier
    EtsTypeAPIFeatureAccessModifier access_mod;
    if (field->IsPublic()) {
        access_mod = EtsTypeAPIFeatureAccessModifier::PUBLIC;
    } else if (field->IsPrivate()) {
        access_mod = EtsTypeAPIFeatureAccessModifier::PRIVATE;
    } else {
        access_mod = EtsTypeAPIFeatureAccessModifier::PROTECTED;
    }
    typeapi_field->SetAccessMod(access_mod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (field->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIFeatureAttributes::STATIC) : 0U;
    attr |= (field->IsInherited()) ? static_cast<uint32_t>(EtsTypeAPIFeatureAttributes::INHERITED) : 0U;
    attr |= (field->IsReadonly()) ? static_cast<uint32_t>(EtsTypeAPIFeatureAttributes::READONLY) : 0U;

    typeapi_field->SetAttributes(attr);

    return typeapi_field.GetPtr();
}

EtsTypeAPIField *TypeAPIGetField(EtsString *td, EtsLong idx)
{
    // Resolve type by td
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    EtsField *res = obj_type->GetFieldByIndex(idx);
    ASSERT(res != nullptr);
    return CreateField(res);
}

EtsTypeAPIField *TypeAPIGetFieldByName(EtsString *td, EtsString *str)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> fname_ptr(coroutine, str->GetCoreType());

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    return CreateField(obj_type->GetFieldIDByName(fname_ptr.GetPtr()->GetMutf8().c_str()));
}

EtsLong TypeAPIGetMethodsNum(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    return obj_type->GetMethodsNum();
}

EtsLong TypeAPIGetConstructorsNum(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    auto methods = obj_type->GetConstructors();
    return methods.size();
}

EtsTypeAPIMethod *CreateMethod(EtsMethod *method)
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
    EtsTypeAPIFeatureAccessModifier access_mod;
    if (method->IsPublic()) {
        access_mod = EtsTypeAPIFeatureAccessModifier::PUBLIC;
    } else if (method->IsPrivate()) {
        access_mod = EtsTypeAPIFeatureAccessModifier::PRIVATE;
    } else {
        access_mod = EtsTypeAPIFeatureAccessModifier::PROTECTED;
    }
    typeapi_method.GetPtr()->SetAccessMod(access_mod);

    // Set specific attributes
    uint32_t attr = 0;
    attr |= (method->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIFeatureAttributes::STATIC) : 0U;
    attr |= (method->IsConstructor()) ? static_cast<uint32_t>(EtsTypeAPIFeatureAttributes::CONSTRUCTOR) : 0U;

    typeapi_method.GetPtr()->SetAttributes(attr);
    return typeapi_method.GetPtr();
}

EtsTypeAPIMethod *TypeAPIGetMethod(EtsString *td, EtsLong i)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    return CreateMethod(obj_type->GetMethodByIndex(i));
}

EtsTypeAPIMethod *TypeAPIGetConstructor(EtsString *td, EtsLong i)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    auto constructors = obj_type->GetConstructors();
    ASSERT(0 <= i && i < (EtsLong)constructors.size());
    return CreateMethod(constructors[i]);
}

EtsLong TypeAPIGetInterfacesNum(EtsString *td)
{
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    return obj_type->GetRuntimeClass()->GetInterfaces().size();
}

EtsString *TypeAPIGetInterface(EtsString *td, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto obj_type = class_linker->GetClass(td->GetMutf8().c_str());
    auto interfaces = obj_type->GetRuntimeClass()->GetInterfaces();
    ASSERT(0 <= i && i <= (EtsLong)interfaces.size());
    auto inter_type = EtsClass::FromRuntimeClass(interfaces[i]);
    return EtsString::CreateFromMUtf8(inter_type->GetDescriptor());
}

EtsLong TypeAPIGetParametersNum(EtsString *td)
{
    auto type_desc = td->GetMutf8();
    auto constructor = EtsMethod::FromTypeDescriptor(type_desc);
    return constructor->GetParametersNum();
}

EtsTypeAPIParameter *CreateParameter(EtsClass *type)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsTypeAPIParameter> typeapi_parameter(coroutine, EtsTypeAPIParameter::Create(coroutine));

    // Set Type Descriptor
    auto td = EtsString::CreateFromMUtf8(type->GetDescriptor());
    VMHandle<EtsString> td_handle(coroutine, td->GetCoreType());
    typeapi_parameter.GetPtr()->SetTypeDesc(td_handle.GetPtr());
    auto name = EtsClass::CreateEtsClassName(type->GetDescriptor());
    typeapi_parameter.GetPtr()->SetName(name);

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
    return CreateParameter(type);
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

EtsBoolean TypeAPIIsMethod(EtsString *td)
{
    return static_cast<panda::ets::EtsBoolean>(EtsMethod::IsMethod(td->GetMutf8()));
}

EtsLong TypeAPIGetTypeId(EtsString *td)
{
    auto type_desc = td->GetMutf8();
    // Create Null class in runtime
    if (type_desc[0] == NULL_TYPE_PREFIX) {
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
