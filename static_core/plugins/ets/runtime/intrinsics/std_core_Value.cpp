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

#include "ets_coroutine.h"
#include "ets_vm.h"
#include "intrinsics.h"
#include "mem/vm_handle.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_void.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_type.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_field.h"

namespace panda::ets::intrinsics {

EtsVoid *ValueAPISetFieldObject(EtsObject *obj, EtsLong i, EtsObject *val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> obj_handle(coroutine, obj->GetCoreType());

    auto type_class = obj_handle.GetPtr()->GetClass();
    auto field_object = type_class->GetFieldByIndex(i);
    obj_handle.GetPtr()->SetFieldObject(field_object, val);
    return EtsVoid::GetInstance();
}

template <typename T>
void SetFieldValue(EtsObject *obj, EtsLong i, T val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> obj_handle(coroutine, obj->GetCoreType());

    auto type_class = obj_handle.GetPtr()->GetClass();
    auto field_object = type_class->GetFieldByIndex(i);
    if (field_object->GetType()->IsBoxedClass()) {
        obj_handle.GetPtr()->SetFieldObject(field_object, EtsBoxPrimitive<T>::Create(coroutine, val));
        return;
    }
    obj_handle.GetPtr()->SetFieldPrimitive<T>(field_object, val);
}

EtsVoid *ValueAPISetFieldBoolean(EtsObject *obj, EtsLong i, EtsBoolean val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByte(EtsObject *obj, EtsLong i, EtsByte val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldShort(EtsObject *obj, EtsLong i, EtsShort val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldChar(EtsObject *obj, EtsLong i, EtsChar val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldInt(EtsObject *obj, EtsLong i, EtsInt val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldLong(EtsObject *obj, EtsLong i, EtsLong val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldFloat(EtsObject *obj, EtsLong i, EtsFloat val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldDouble(EtsObject *obj, EtsLong i, EtsDouble val)
{
    SetFieldValue(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameObject(EtsObject *obj, EtsString *name, EtsObject *val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> obj_handle(coroutine, obj->GetCoreType());

    auto type_class = obj_handle.GetPtr()->GetClass();
    auto field_object = type_class->GetFieldIDByName(name->GetMutf8().c_str());
    obj_handle.GetPtr()->SetFieldObject(field_object, val);
    return EtsVoid::GetInstance();
}

template <typename T>
void SetFieldByNameValue(EtsObject *obj, EtsString *name, T val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> obj_handle(coroutine, obj->GetCoreType());

    auto type_class = obj_handle.GetPtr()->GetClass();
    auto field_object = type_class->GetFieldIDByName(name->GetMutf8().c_str());
    if (field_object->GetType()->IsBoxedClass()) {
        obj_handle.GetPtr()->SetFieldObject(field_object, EtsBoxPrimitive<T>::Create(coroutine, val));
        return;
    }
    obj_handle.GetPtr()->SetFieldPrimitive<T>(field_object, val);
}

EtsVoid *ValueAPISetFieldByNameBoolean(EtsObject *obj, EtsString *name, EtsBoolean val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameByte(EtsObject *obj, EtsString *name, EtsByte val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameShort(EtsObject *obj, EtsString *name, EtsShort val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameChar(EtsObject *obj, EtsString *name, EtsChar val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameInt(EtsObject *obj, EtsString *name, EtsInt val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameLong(EtsObject *obj, EtsString *name, EtsLong val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameFloat(EtsObject *obj, EtsString *name, EtsFloat val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetFieldByNameDouble(EtsObject *obj, EtsString *name, EtsDouble val)
{
    SetFieldByNameValue(obj, name, val);
    return EtsVoid::GetInstance();
}

EtsObject *ValueAPIGetFieldObject(EtsObject *obj, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> obj_handle(coroutine, obj->GetCoreType());

    auto type_class = obj_handle.GetPtr()->GetClass();
    auto field_object = type_class->GetFieldByIndex(i);
    return obj_handle.GetPtr()->GetFieldObject(field_object);
}

template <typename T>
T GetFieldValue(EtsObject *obj, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> obj_handle(coroutine, obj->GetCoreType());

    auto type_class = obj_handle.GetPtr()->GetClass();
    auto field_object = type_class->GetFieldByIndex(i);

    if (field_object->GetType()->IsBoxedClass()) {
        return EtsBoxPrimitive<T>::FromCoreType(obj_handle.GetPtr()->GetFieldObject(field_object))->GetValue();
    }
    return obj_handle.GetPtr()->GetFieldPrimitive<T>(field_object);
}

EtsBoolean ValueAPIGetFieldBoolean(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsBoolean>(obj, i);
}

EtsByte ValueAPIGetFieldByte(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsByte>(obj, i);
}

EtsShort ValueAPIGetFieldShort(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsShort>(obj, i);
}

EtsChar ValueAPIGetFieldChar(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsChar>(obj, i);
}

EtsInt ValueAPIGetFieldInt(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsInt>(obj, i);
}

EtsLong ValueAPIGetFieldLong(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsLong>(obj, i);
}

EtsFloat ValueAPIGetFieldFloat(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsFloat>(obj, i);
}

EtsDouble ValueAPIGetFieldDouble(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsDouble>(obj, i);
}

EtsVoid *ValueAPISetElementObject(EtsObject *obj, EtsLong i, EtsObject *val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObjectArray> arr_handle(coroutine, obj->GetCoreType());

    arr_handle.GetPtr()->Set(i, val);
    return EtsVoid::GetInstance();
}

template <typename P, typename T>
void SetElement(EtsObject *obj, EtsLong i, T val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    auto type_class = obj->GetClass();
    if (!type_class->GetComponentType()->IsBoxedClass()) {
        VMHandle<P> arr_handle(coroutine, obj->GetCoreType());
        arr_handle.GetPtr()->Set(i, val);
    } else {
        VMHandle<EtsObjectArray> arr_handle(coroutine, obj->GetCoreType());
        arr_handle.GetPtr()->Set(i, EtsBoxPrimitive<T>::Create(coroutine, val));
    }
}

EtsVoid *ValueAPISetElementBoolean(EtsObject *obj, EtsLong i, EtsBoolean val)
{
    SetElement<EtsBooleanArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementByte(EtsObject *obj, EtsLong i, EtsByte val)
{
    SetElement<EtsByteArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementShort(EtsObject *obj, EtsLong i, EtsShort val)
{
    SetElement<EtsShortArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementChar(EtsObject *obj, EtsLong i, EtsChar val)
{
    SetElement<EtsCharArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementInt(EtsObject *obj, EtsLong i, EtsInt val)
{
    SetElement<EtsIntArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementLong(EtsObject *obj, EtsLong i, EtsLong val)
{
    SetElement<EtsLongArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementFloat(EtsObject *obj, EtsLong i, EtsFloat val)
{
    SetElement<EtsFloatArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

EtsVoid *ValueAPISetElementDouble(EtsObject *obj, EtsLong i, EtsDouble val)
{
    SetElement<EtsDoubleArray>(obj, i, val);
    return EtsVoid::GetInstance();
}

}  // namespace panda::ets::intrinsics