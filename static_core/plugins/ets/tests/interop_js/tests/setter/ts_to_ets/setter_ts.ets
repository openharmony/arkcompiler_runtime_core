/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import {
    UnionSetter,
    InterfaceSetter,
    BaseClass,
    AbstractClass,
    SubsetRefSet,
    SubsetValueSet,
    UnionSetterObject,
    InterfaceSetterObject,
    TupleSetObject,
    SetterAnyTypeObject,
    BaseClassObject,
    SubsetRefSetObject,
    SubsetValueSetObject,
    tsTestString,
    AbstractClassObject
} from '/plugins/ets/tests/interop_js/tests/setter/ts_to_ets/setter.js';

type Tuple = [int, string];

const strContent = 'ets_test_string';
const numContent = 1;
const tupleContent: Tuple = [numContent, strContent];

function err_log(op: String) {
    console.println("Bug operation: " + op);
}

function check_abstract_сlass_setter(): boolean {
    const abstractClass = new AbstractClass();
    abstractClass.value = strContent;

    return strContent == abstractClass.value;
}

function check_union_setter(): boolean {
    const unionSetter = new UnionSetter();
    unionSetter.value = numContent;

    return numContent == unionSetter.value;
}

function check_interface_setter(): boolean {
        const interfaceSetter = new InterfaceSetter();
        interfaceSetter.value = strContent;

        return interfaceSetter.value == strContent;
}

function check_user_class_setter() {
    const base = new BaseClass();
    base.value = strContent;

    return strContent == base.value;
}

function check_extends_class() {
    const subsetRefSet = new SubsetRefSet();
    subsetRefSet.value = strContent;

    return strContent == subsetRefSet.value;
}

function check_extends_class_with_value() {
    const subsetValueSet = new SubsetValueSet();
    subsetValueSet.value = strContent;

    return strContent == subsetValueSet.value;
}

function check_tuple_type_object_form_ts() {
    TupleSetObject.value = tupleContent;

    if (TupleSetObject.value[0] == tupleContent[0] && TupleSetObject.value[1] == tupleContent[1]) {
        return true;
    } else {
        err_log('tuple type object setter not work.');

        return false;
    }
}

function check_setter_any_type_object_form_ts() {
    SetterAnyTypeObject.value = strContent;

    return check(SetterAnyTypeObject.value as string, strContent, 'setter any type object not work.');
}

function check_setter_union_type_object_form_ts() {
    UnionSetterObject.value = numContent;

    return check(UnionSetterObject.value as int, numContent, 'setter union type object not work.');
}

function check_setter_interface_object_form_ts() {
    InterfaceSetterObject.value = strContent;

    return check(InterfaceSetterObject.value as string, strContent, 'setter interface object not work.');
}

function check_setter_user_object_form_ts() {
    BaseClassObject.value = strContent;

    return check(BaseClassObject.value as string, strContent, 'setter user object not work.');
}

function check_setter_subset_ref_set_object_form_ts() {
    SubsetRefSetObject.value = strContent;

    return check(SubsetRefSetObject.value as string, strContent, 'setter subset ref set object not work.');
}

function check_setter_subset_value_set_object_form_ts() {
    if (SubsetValueSetObject.value !== tsTestString) {
        err_log('incorect value in object.');

        return false;
    }

    SubsetValueSetObject.value = strContent;

    return check(SubsetValueSetObject.value as string, strContent, 'setter subset value set object not work.');
}

function check_setter_abstract_class_object_form_ts() {
    AbstractClassObject.value = strContent;

    return check(AbstractClassObject.value as string, strContent, 'setter abstract class object not work.');
}

function check(value: string | int, content: string | int, message: string): boolean {
    if (value == content) {
        return true;
    } else {
        err_log(message);

        return false;
    }
}
