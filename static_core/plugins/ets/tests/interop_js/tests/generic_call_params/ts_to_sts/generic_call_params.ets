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
    applyFunctionGeneric,
    applyFunctionGenericTuple,
    genericExtendsClass,
    Vector,
    genericExtendsClassCallFromTs,
    applyFunctionGenericArray,
    applyFunctionGenericArrayCallFromTs,
    applyFunWithConstraints,
    genericKeyof,
    ClassSubset,
    subsetClassCallFromTs,
    applyFunctionGenericUnion,
} from 'main_js';

type Union = string | int;
type TupleT = [string, int];
const etsInt = 1;
const etsUnion: Union = etsInt;
const etsString = 'string';
const arrForLoop: FixedArray<int> = [0, 1, 2]
const etsTuple: TupleT = [etsString, etsInt];
class EtsUser {
    age = etsInt;
    data = etsString;
}

const etsUser = new EtsUser();

const unionFoo = (a: Union, b: Union): FixedArray<Union> => [a, b];
const add = (a: int, b: int): int => a + b;
const concat = (a: string, b: string): string => a + ' ' + b;
const push = (a: string, b: int): TupleT => [a, b];

function checkApplyFunctionGenericString(): boolean {
    return applyFunctionGeneric(etsString, etsString, concat) as string == etsString + ' ' + etsString as string;
}

function checkApplyFunctionGenericInt(): boolean {
    return applyFunctionGeneric(etsInt, etsInt, add) as int == etsInt + etsInt as int;
}

function checkApplyFunctionGenericTuple(): boolean {
    const res: TupleT = applyFunctionGenericTuple(etsTuple[0], etsTuple[1], push);

    return res[0] as string == etsTuple[0] as string
        && res[1] as int == etsTuple[1] as int;
}

function checkGenericExtendsClass(): boolean {
    const res = genericExtendsClass(new Vector(etsInt));

    return res.get() as int > etsInt as int;
}

function checkGenericExtendsClassCallFromTs(): boolean {
    const res = genericExtendsClassCallFromTs(new Vector(etsInt));

    return res.get() as int > etsInt as int;
}

function checkApplyFunctionGenericArrayCallFromTs() {
    const res = arrSum(arrForLoop);

    return applyFunctionGenericArrayCallFromTs(arrForLoop) as int == res as int;
}

function checkApplyFunctionGenericArray(): boolean {
    const res = arrSum(arrForLoop);

    return applyFunctionGenericArray(arrForLoop, add) as int == res as int;
}

function arrSum(arr: FixedArray<int>): int {
    let res = 0
    for (let i = 0; i < arrForLoop.length; ++i) {
        res += arrForLoop[i];
    }

    return res;
}

const toStringArrowFu = (a: int, b: string): string => a + ' ' + b;

function toStringFu(a: int, b: string): string {
    return a + ' ' + b;
}

//NOTE: issue (18005) fix function as param
// function checkApplyFunWithConstraintsFun() {
//     const res = applyFunWithConstraints(etsInt, etsString, toStringFu);

//     return res as string == etsInt + ' ' + etsString as string;
// }

function checkApplyFunWithConstraintsArrowFun() {
    const res = applyFunWithConstraints(etsInt, etsString, toStringArrowFu);

    return res as string == etsInt + ' ' + etsString as string;
}

function checkGenericKeyof(): boolean {
    const keys: FixedArray<string> = ['age', 'data'];
    const res = genericKeyof(etsUser, keys);

    const checkKeys = res.keys as Array<string> instanceof Array
        && res.keys[0] as string == keys[0] as string
        && res.keys[1] as string == keys[1] as string;

    const checkObj = res.obj.age as int == etsInt as int
        && res.obj.data as string == etsString as string

    return checkKeys && checkObj;
}

function checkClassSubset(): boolean {
    const GClass = ClassSubset();

    return GClass.get(etsInt, etsInt, add) as int > etsInt as int;
}

function checkSubsetClassCallFromTs(): boolean {
    return subsetClassCallFromTs() as int > etsInt as int;
}

function checkApplyFunctionGenericUnion(): boolean {
    const res = applyFunctionGenericUnion(etsUnion, etsUnion, unionFoo);
    return res as Array<Union> instanceof Array
        && res[0] as Union == etsUnion as Union
        && res[0] as Union == etsUnion as Union;
}
