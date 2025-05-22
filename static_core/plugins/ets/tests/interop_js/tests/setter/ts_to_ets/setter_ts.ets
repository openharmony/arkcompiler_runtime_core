/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
} from 'js/setter';

type TupleT = [int, string];

const strContent = 'ets_test_string';
const numContent = 1;
const tupleContent: TupleT = [numContent, strContent];

function errLog(op: String) {
    console.println("Bug operation: " + op);
}

function checkAbstractClassSetter(): boolean {
    const abstractClass = new AbstractClass();
    abstractClass.value = strContent;

    return strContent == abstractClass.value;
}

function checkUnionSetter(): boolean {
    const unionSetter = new UnionSetter();
    unionSetter.value = numContent;

    return numContent == unionSetter.value;
}

function checkInterfaceSetter(): boolean {
        const interfaceSetter = new InterfaceSetter();
        interfaceSetter.value = strContent;

        return interfaceSetter.value == strContent;
}

function checkUserClassSetter() {
    const base = new BaseClass();
    base.value = strContent;

    return strContent == base.value;
}

function checkExtendsClass() {
    const subsetRefSet = new SubsetRefSet();
    subsetRefSet.value = strContent;

    return strContent == subsetRefSet.value;
}

function checkExtendsClassWithValue() {
    const subsetValueSet = new SubsetValueSet();
    subsetValueSet.value = strContent;

    return strContent == subsetValueSet.value;
}

function checkTupleTypeObjectFormTs() {
    TupleSetObject.value = tupleContent;

    if (TupleSetObject.value[0] == tupleContent[0] && TupleSetObject.value[1] == tupleContent[1]) {
        return true;
    } else {
        errLog('tuple type object setter not work.');

        return false;
    }
}

function checkSetterAnyTypeObjectFormTs() {
    SetterAnyTypeObject.value = strContent;

    return check(SetterAnyTypeObject.value as string, strContent, 'setter any type object not work.');
}

function checkSetterUnionTypeObjectFormTs() {
    UnionSetterObject.value = numContent;

    return check(UnionSetterObject.value as int, numContent, 'setter union type object not work.');
}

function checkSetterInterfaceObjectFormTs() {
    InterfaceSetterObject.value = strContent;

    return check(InterfaceSetterObject.value as string, strContent, 'setter interface object not work.');
}

function checkSetterUserObjectFormTs() {
    BaseClassObject.value = strContent;

    return check(BaseClassObject.value as string, strContent, 'setter user object not work.');
}

function checkSetterSubsetRefSetObjectFormTs() {
    SubsetRefSetObject.value = strContent;

    return check(SubsetRefSetObject.value as string, strContent, 'setter subset ref set object not work.');
}

function checkSetterSubsetValueSetObjectFormTs() {
    if (SubsetValueSetObject.value !== tsTestString) {
        errLog('incorect value in object.');

        return false;
    }

    SubsetValueSetObject.value = strContent;

    return check(SubsetValueSetObject.value as string, strContent, 'setter subset value set object not work.');
}

function checkSetterAbstractClassObjectFormTs() {
    AbstractClassObject.value = strContent;

    return check(AbstractClassObject.value as string, strContent, 'setter abstract class object not work.');
}

function check(value: string | int, content: string | int, message: string): boolean {
    if (value == content) {
        return true;
    } else {
        errLog(message);

        return false;
    }
}
