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
    TupleT,
    tsNumber,
    tsString,
    tsBool,
    tsArr,
    tsObj,
    tsUnion,
    tsTuple,
    tsLiteral,
    anyTypeParameter,
    anyTypeParameterExplicitCallFromTsInt,
    anyTypeParameterExplicitCallFromTsString,
    anyTypeParameterExplicitCallFromTsBool,
    anyTypeParameterExplicitCallFromTsArr,
    anyTypeParameterExplicitCallFromTsObj,
    anyTypeParameterExplicitCallFromTsUnion,
    anyTypeParameterExplicitCallFromTsTuple,
    anyTypeParameterExplicitCallFromTsLiteral,
    //
    genericTypeFunctionReturnAny,
    genericTypeFunctionExplicitCallFromTsInt,
    genericTypeFunctionExplicitCallFromTsString,
    genericTypeFunctionExplicitCallFromTsBool,
    genericTypeFunctionExplicitCallFromTsArr,
    genericTypeFunctionExplicitCallFromTsObj,
    genericTypeFunctionExplicitCallFromTsUnion,
    genericTypeFunctionExplicitCallFromTsTuple,
    genericTypeFunctionExplicitCallFromTsLiteral,
    //
    extendGenericNumber,
    extendGenericString,
    extendGenericBool,
    extendGenericArr,
    extendGenericObj,
    extendGenericUnion,
    extendGenericTuple,
    extendGenericLiteral,
    //
    GInterfaceClass,
    GTypeClass,
    genericExtendInterface,
    genericExtendType,
    //
    tupleGeneric,
    //
    collectGeneric,
    //
    UserClass,
    createClassFromTs,
    userClassInstance,
    //
    genericDefaultInt,
    genericDefaultString,
    genericDefaultBool,
    genericDefaultArr,
    genericDefaultObj,
    genericDefaultUnion,
    genericDefaultTuple,
    genericDefaultLiteral,
    genericDefaultIntCallFromTs,
    genericDefaultStringCallFromTs,
    genericDefaultBoolCallFromTs,
    genericDefaultArrCallFromTs,
    genericDefaultObjCallFromTs,
    genericDefaultUnionCallFromTs,
    genericDefaultTupleCallFromTs,
    genericDefaultLiteralCallFromTs,
    //
    DataClass,
    genericExtendClass,
} from 'main_js';

type TupleT = [int, boolean];
type Union = string | number;

const etsNumber = 1;
const etsString = 'string';
const etsBool = true;
const etsArr: FixedArray<int> = [etsNumber];
const etsObj: object = {};
const etsUnion: Union = etsNumber;
const etsTuple: TupleT = [etsNumber, etsBool];

function checkAnyTypeParameterInt(): boolean {
    return anyTypeParameter(etsNumber) as int == etsNumber as int;
}

function checkAnyTypeParameterString(): boolean {
    return anyTypeParameter(etsString) as string == etsString as string;
}

function checkAnyTypeParameterBool(): boolean {
    return anyTypeParameter(etsBool) as boolean == etsBool as boolean;
}

function checkAnyTypeParameterArr(): boolean {
    const res = anyTypeParameter(etsArr);

    return res as Array<int> instanceof Array
        && res[0] as int == etsArr[0] as int;
}

function checkAnyTypeParameterObj(): boolean {
    return typeof anyTypeParameter(etsObj) == 'object';
}

function checkAnyTypeParameterUnion(): boolean {
    return anyTypeParameter(etsUnion) as int == etsNumber as int;
}

function checkAnyTypeParameterTuple(): boolean {
    const res = anyTypeParameter(etsTuple);

    return res as Array<TupleT> instanceof Array
        && res[0] as int == etsTuple[0] as int
        && res[1] as boolean == etsTuple[1] as boolean;
}
// any type parameter explicit

function checkAnyTypeParameterExplicitCallInt(): boolean {
    return anyTypeParameterExplicitCallFromTsInt() as int == tsNumber as int;
}

function checkAnyTypeParameterExplicitCallString(): boolean {
    return anyTypeParameterExplicitCallFromTsString() as string == tsString as string;
}

function checkAnyTypeParameterExplicitCallBool(): boolean {
    return anyTypeParameterExplicitCallFromTsBool() as boolean == tsBool as boolean;
}

function checkAnyTypeParameterExplicitCallArr(): boolean {
    const res = anyTypeParameterExplicitCallFromTsArr();
    return res as Array<int> instanceof Array
        && res[0] as int == tsArr[0] as int;
}

function checkAnyTypeParameterExplicitCallObj(): boolean {
    return typeof anyTypeParameterExplicitCallFromTsObj() == 'object';
}

function checkAnyTypeParameterExplicitCallUnion(): boolean {
    return anyTypeParameterExplicitCallFromTsUnion() as int == tsUnion as int;
}

function checkAnyTypeParameterExplicitCallTuple(): boolean {
    const res = anyTypeParameterExplicitCallFromTsTuple();
    return res as Array<TupleT> instanceof Array
        && res[0] as int == tsTuple[0] as int
        && res[1] as boolean == tsTuple[1] as boolean;
}

function checkAnyTypeParameterExplicitCallLiteral(): boolean {
    return anyTypeParameterExplicitCallFromTsLiteral() as int == tsLiteral as int;
}

// generic type function return any
function checkGenericTypeFunctionReturnAnyInt(): boolean {
    return genericTypeFunctionReturnAny(etsNumber) as int == etsNumber as int;
}

function checkGenericTypeFunctionReturnAnyString(): boolean {
    return genericTypeFunctionReturnAny(etsString) as string == etsString as string;
}

function checkGenericTypeFunctionReturnAnyBool(): boolean {
    return genericTypeFunctionReturnAny(etsBool) as boolean == etsBool as boolean;
}

function checkGenericTypeFunctionReturnAnyArr(): boolean {
    const res = genericTypeFunctionReturnAny(etsArr);

    return res as Array<int> instanceof Array
        && res[0] as int == etsArr[0] as int;
}

function checkGenericTypeFunctionReturnAnyObj(): boolean {
    return typeof genericTypeFunctionReturnAny(etsObj) == 'object';
}

function checkGenericTypeFunctionReturnAnyUnion(): boolean {
    return genericTypeFunctionReturnAny(etsUnion) as int == etsNumber as int;
}

function checkGenericTypeFunctionReturnAnyTuple(): boolean {
    const res = genericTypeFunctionReturnAny(etsTuple);

    return res as Array<TupleT> instanceof Array
        && res[0] as int == etsTuple[0] as int
        && res[1] as boolean == etsTuple[1] as boolean;
}
//
function checkGenericTypeFunctionExplicitCallInt(): boolean {
    return genericTypeFunctionExplicitCallFromTsInt() as int == tsNumber as int;
}

function checkGenericTypeFunctionExplicitCallString(): boolean {
    return genericTypeFunctionExplicitCallFromTsString() as string == tsString as string;
}

function checkGenericTypeFunctionExplicitCallBool(): boolean {
    return genericTypeFunctionExplicitCallFromTsBool() as boolean == tsBool as boolean;
}

function checkGenericTypeFunctionExplicitCallArr(): boolean {
    const res = genericTypeFunctionExplicitCallFromTsArr();
    return res as Array<int> instanceof Array
        && res[0] as int == tsArr[0] as int;
}

function checkGenericTypeFunctionExplicitCallObj(): boolean {
    return typeof genericTypeFunctionExplicitCallFromTsObj() == 'object';
}

function checkGenericTypeFunctionExplicitCallUnion(): boolean {
    return genericTypeFunctionExplicitCallFromTsUnion() as int == tsUnion as int;
}

function checkGenericTypeFunctionExplicitCallTuple(): boolean {
    const res = genericTypeFunctionExplicitCallFromTsTuple();
    return res as Array<TupleT> instanceof Array
        && res[0] as int == tsTuple[0] as int
        && res[1] as boolean == tsTuple[1] as boolean;
}

function checkGenericTypeFunctionExplicitCallLiteral(): boolean {
    return genericTypeFunctionExplicitCallFromTsLiteral() as int == tsLiteral as int;
}
// extends generic arg
function checkExtendGenericNumber(): boolean {
    return extendGenericNumber(etsNumber) as int == etsNumber as int;
}

function checkExtendGenericString(): boolean {
    return extendGenericString(etsString) as string == etsString as string;
}

function checkExtendGenericBool(): boolean {
    return extendGenericBool(etsBool) as boolean == etsBool as boolean;
}

function checkExtendGenericArr(): boolean {
    const res = extendGenericArr(etsArr);
    return res as Array<int> instanceof Array
        && res[0] as int == etsArr[0] as int;
}

function checkExtendGenericObj(): boolean {
    return typeof extendGenericObj(etsObj) == 'object';
}

function checkExtendGenericUnion(): boolean {
    return extendGenericUnion(etsUnion) as int == etsUnion as int;
}

function checkExtendGenericTuple(): boolean {
    const res = extendGenericTuple(etsTuple);

    return res as Array<TupleT> instanceof Array
        && res[0] as int == etsTuple[0] as int
        && res[1] as boolean == etsTuple[1] as boolean;
}

function checkExtendGenericLiteral(): boolean {
    return extendGenericLiteral(tsLiteral) as int == tsLiteral as int;
}
// generic extend type and interface
function checkGenericExtendInterface(): boolean {
    const obj = new GInterfaceClass();
    const res = genericExtendInterface(obj);

    return !!res.data && res.data as string == obj.data as string;
}

function checkGenericExtendType(): boolean {
    const obj = new GTypeClass();
    const res = genericExtendType(obj);

    return !!res.data && res.data as string == obj.data as string;
}

function checkTupleGeneric() {
    const res = tupleGeneric(etsTuple);

    return res as Array<TupleT> instanceof Array
        && res[0] as boolean == etsTuple[1] as boolean
        && res[1] as int == etsTuple[0] as int;
}
// collectGeneric

function checkCollectGenericInt(): boolean {
    const res = collectGeneric(etsNumber);

    return res as Array<int> instanceof Array && res[0] as int == etsNumber as int;
}

function checkCollectGenericString(): boolean {
    const res = collectGeneric(etsString);

    return res as Array<string> instanceof Array && res[0] as string == etsString as string;
}

function checkCollectGenericBool(): boolean {
    const res = collectGeneric(etsBool);

    return res as Array<boolean> instanceof Array && res[0] as boolean == etsBool as boolean;
}

function checkCollectGenericObj(): boolean {
    const res = collectGeneric(tsObj);

    return res as Array<object> instanceof Array;
}

//user class
function checkGenericUserClass(): boolean {
    const user = new UserClass(etsString, etsNumber);

    return user.name as string == etsString as string && user.age as int == etsNumber as int;
}

function checkCreateClassFromTs(): boolean {
    const user = createClassFromTs();

    return user.name as string == tsString as string && user.age as int == tsNumber as int;
}

function checkUserClassInstance(): boolean {
    const user = userClassInstance;

    return user.name as string == tsString as string && user.age as int == tsNumber as int;
}
// generic default type
function checkGenericDefaultInt(): boolean {
    return genericDefaultInt(etsNumber) as int == etsNumber as int;
}

function checkGenericDefaultString(): boolean {
    return genericDefaultString(etsString) as string == etsString as string;
}

function checkGenericDefaultBool(): boolean {
    return genericDefaultBool(etsBool) as boolean == etsBool as boolean;
}

function checkGenericDefaultArr(): boolean {
    const res = genericDefaultArr(etsArr);

    return res as Array<int> instanceof Array
        && res[0] as int == etsArr[0] as int;
}

function checkGenericDefaultObj(): boolean {
    return typeof genericDefaultObj(etsObj) == 'object';
}

function checkGenericDefaultUnion(): boolean {
    return genericDefaultUnion(etsUnion) as int == etsNumber as int;
}

function checkGenericDefaultTuple(): boolean {
    const res = genericDefaultTuple(etsTuple);

    return res as Array<TupleT> instanceof Array
        && res[0] as int == etsTuple[0] as int
        && res[1] as boolean == etsTuple[1] as boolean;
}

function checkGenericDefaultLiteral(): boolean {
    return genericDefaultLiteral(etsNumber) as int == etsNumber as int;
}

function checkGenericDefaultIntCallFromTs(): boolean {
    return genericDefaultIntCallFromTs() as int == etsNumber as int;
}

function checkGenericDefaultStringCallFromTs(): boolean {
    return genericDefaultStringCallFromTs() as string == etsString as string;
}

function checkGenericDefaultBoolCallFromTs(): boolean {
    return genericDefaultBoolCallFromTs() as boolean == etsBool as boolean;
}

function checkGenericDefaultArrCallFromTs(): boolean {
    const res = genericDefaultArrCallFromTs();

    return res as Array<int> instanceof Array
        && res[0] as int == etsArr[0] as int;
}

function checkGenericDefaultObjCallFromTs(): boolean {
    return typeof genericDefaultObjCallFromTs(etsObj) == 'object';
}

function checkGenericDefaultUnionCallFromTs(): boolean {
    return genericDefaultUnionCallFromTs() as int == etsNumber as int;
}

function checkGenericDefaultTupleCallFromTs(): boolean {
    const res = genericDefaultTupleCallFromTs();

    return res as Array<TupleT> instanceof Array
        && res[0] as int == etsTuple[0] as int
        && res[1] as boolean == etsTuple[1] as boolean;
}

function checkGenericDefaultLiteralCallFromTs(): boolean {
    return genericDefaultLiteralCallFromTs() as int == etsNumber as int;
}
//generic extend class
function checkGenericExtendClass(): boolean {
    const data = new DataClass();
    return genericExtendClass(data).data as string == data.data as string;
}
