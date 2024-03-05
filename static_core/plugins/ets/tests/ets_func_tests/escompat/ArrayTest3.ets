/*
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

const success : int = 0;
const fail    : int = 1;

function main(): int {
    let failures: int = 0;
    failures += check((): int => { return everyFuncArg3ArrayNumber()}, "everyFuncArg3ArrayNumber");
    failures += check((): int => { return everyFuncArg2()}, "everyFuncArg2");
    failures += check((): int => { return everyFuncArg1()}, "everyFuncArg1");
    failures += check((): int => { return someFuncArg2()}, "someFuncArg2");
    failures += check((): int => { return someFuncArg1()}, "someFuncArg1");
    failures += check((): int => { return filterFuncArg3ArrayNumber()}, "filterFuncArg3ArrayNumber");
    failures += check((): int => { return filterFuncArg2()}, "filterFuncArg2");
    failures += check((): int => { return filterFuncArg1()}, "filterFuncArg1");
    failures += check((): int => { return reduceFuncArg4ArrayNumber()}, "reduceFuncArg4ArrayNumber");
    failures += check((): int => { return reduceFuncArg3()}, "reduceFuncArg3");
    failures += check((): int => { return reduceFuncArg2()}, "reduceFuncArg2");
    failures += check((): int => { return reduceFuncArg4ArrayNumberWithInitialValue()}, "reduceFuncArg4ArrayNumberWithInitialValue");
    failures += check((): int => { return reduceFuncArg3WithInitialValue()}, "reduceFuncArg3WithInitialValue");
    failures += check((): int => { return reduceFuncArg2WithInitialValue()}, "reduceFuncArg2WithInitialValue");
    return check(failures, "All tests run");
}


const d: Number[] = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];
const dd: Number[] = [1, 0, 3, 4, 0, 6];

function __asGenericBool1<T>(x: Object): (elem: T) => boolean {
    return x as (elem: T) => boolean
}

function __asGenericBool2<T>(x: Object): (elem: T, index: number) => boolean {
  return x as (elem: T, index: number) => boolean
}

function __asGenericBool3<T>(x: Object): (elem: T, index: number, obj: Array<T>) => boolean {
  return x as (elem: T, index: number, obj: Array<T>) => boolean
}

function __asGenericU1<T,U>(x: Object): (elem: T) => U {
    return x as (elem: T) => U
}

function __asGenericU2<T,U>(x: Object): (elem: T, index: number) => U {
    return x as (elem: T, index: number) => U
}

function __asGenericU3<T,U>(x: Object): (elem: T, index: number, obj: Array<T>) => U {
    return x as (elem: T, index: number, obj: Array<T>) => U
}

function __asGenericABnumber<T>(x: Object): (a: T, b: T) => number {
    return x as (a: T, b: T) => number
}

function __asGenericVoid1<T>(x: Object): (a: T) => void {
    return x as (a: T) => void
}

function __asGenericVoid2<T>(x: Object): (a: T, index: number) => void {
    return x as (a: T, index: number) => void
}

function __asGenericVoid3<T>(x: Object): (a: T, index: number, arr: Array<T>) => void {
    return x as (a: T, index: number, arr: Array<T>) => void
}

function __asGenericABT<T>(x: Object): (a: T, b: T) => T {
    return x as (a: T, b: T) => T
}

function __asGenericABindexT<T>(x: Object): (a: T, b: T, index: number) => T {
    return x as (a: T, b: T, index: number) => T
}

function __asGenericABindexArrT<T>(x: Object): (a: T, b: T, index: number, arr: Array<T>) => T {
    return x as (a: T, b: T, index: number, arr: Array<T>) => T
}

function everyFuncArg3ArrayNumber(): int {
    let fnTrue = (elem: Object, index: number, arr: Array<Object>): boolean => {
        return (arr.at(index) as Number).valueOf() == (elem as Number).valueOf() && (elem as Number).valueOf() < 238;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.every(__asGenericBool3<Number>(fnTrue)) == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    if (arr.every(__asGenericBool3<Number>(fnTrue)) == false) { return fail; }

    let fnFalse = (elem: Object, index: number, arr: Array<Object>): boolean => {
        return (arr.at(index) as Number).valueOf() == (elem as Number).valueOf() && (elem as Number).valueOf() < -238;
    }
    if (arr.every(__asGenericBool3<Number>(fnFalse)) == true) { return fail; }
    return success;
}

function everyFuncArg2(): int {
    let fnTrue = (elem: Object, index: number): boolean => {
        return index >= 0 && (elem as Number).valueOf() < 238;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.every(__asGenericBool2<Number>(fnTrue)) == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    if (arr.every(__asGenericBool2<Number>(fnTrue)) == false) { return fail; }

    let fnFalse = (elem: Object, index: number): boolean => {
        return index >= 0 && (elem as Number).valueOf() < -238;
    }
    if (arr.every(__asGenericBool2<Number>(fnFalse)) == true) { return fail; }
    return success;
}

function everyFuncArg1(): int {
    let fnTrue = (elem: Object): boolean => {
        return (elem as Number).valueOf() < 238;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.every(__asGenericBool1<Number>(fnTrue)) == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    if (arr.every(__asGenericBool1<Number>(fnTrue)) == false) { return fail; }

    let fnFalse = (elem: Object): boolean => {
        return (elem as Number).valueOf() < -238;
    }
    if (arr.every(__asGenericBool1<Number>(fnFalse)) == true) { return fail; }
    return success;
}

function someFuncArg2(): int {
    let fnTrue = (v: Object, index: number): boolean => {
        return index >= 0 && (v as Number).valueOf() < -4;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.some(__asGenericBool2<Number>(fnTrue)) == true) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    if (arr.some(__asGenericBool2<Number>(fnTrue)) == false) { return fail; }

    let fnFalse = (v: Object, index: number): boolean => {
        return index >= 0 && (v as Number).valueOf() < -238;
    }
    if (arr.some(__asGenericBool2<Number>(fnFalse)) == true) { return fail; }
    return success;
}

function someFuncArg1(): int {
    let fnTrue = (v: Object): boolean => {
        return (v as Number).valueOf() < -4;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.some(__asGenericBool1<Number>(fnTrue)) == true) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    if (arr.some(__asGenericBool1<Number>(fnTrue)) == false) { return fail; }

    let fnFalse = (v: Object): boolean => {
        return (v as Number).valueOf() < -238;
    }
    if (arr.some(__asGenericBool1<Number>(fnFalse)) == true) { return fail; }
    return success;
}

function filterFuncArg3ArrayNumber(): int {
    let fnTrue = (v: Object, index: number, arr: Array<Object>): boolean => {
        return ((arr.at(index) as Number).valueOf() == (v as Number).valueOf()) && (v as Number).valueOf() > -1000;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.filter(__asGenericBool3<Number>(fnTrue)).length != 0) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.filter(__asGenericBool3<Number>(fnTrue));
    if (res.length != arr.length) { return fail; }
    for (let i: int = 0; i < arr.length; ++i) {
        if (res.at(i) != arr.at(i)) { return fail; }
    }

    let fnFalse = (v: Object, index: number, arr: Array<Object>): boolean => {
        return arr.at(index) == v && v == undefined;
    }
    if (arr.filter(__asGenericBool3<Number>(fnFalse)).length != 0) { return fail; }
    return success;
}

function filterFuncArg2(): int {
    let fnTrue = (v: Object, index: number): boolean => {
        return index >= 0 && (v as Number).valueOf() > -1000;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.filter(__asGenericBool2<Number>(fnTrue)).length != 0) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.filter(__asGenericBool2<Number>(fnTrue));
    if (res.length != arr.length) { return fail; }
    for (let i: int = 0; i < arr.length; ++i) {
        if (res.at(i) != arr.at(i)) { return fail; }
    }

    let fnFalse = (v: Object, index: number): boolean => {
        return index >= 0 && v == undefined;
    }
    if (arr.filter(__asGenericBool2<Number>(fnFalse)).length != 0) { return fail; }
    return success;
}

function filterFuncArg1(): int {
    let fnTrue = (v: Object): boolean => {
        return (v as Number).valueOf() > -1000;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.filter(__asGenericBool1<Number>(fnTrue)).length != 0) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.filter(__asGenericBool1<Number>(fnTrue));
    if (res.length != arr.length) { return fail; }
    for (let i: int = 0; i < arr.length; ++i) {
        if (res.at(i) != arr.at(i)) { return fail; }
    }

    let fnFalse = (v: Object): boolean => {
        return v == undefined;
    }
    if (arr.filter(__asGenericBool1<Number>(fnFalse)).length != 0) { return fail; }
    return success;
}

function reduceFuncArg4ArrayNumber(): int {
    let fn = (a: Object, b: Object, index: number, arr: Array<Object>): Object => {
        if (index < 0 || index >= arr.length) {
            return new Number(-1);
        }
        return new Number((a as Number).valueOf() + (b as Number).valueOf() + (arr.at(index) as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduce(__asGenericABindexArrT<Number>(fn));
        return fail;
    } catch(e) {
        hasException = true;
    }
    if (hasException == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.reduce(__asGenericABindexArrT<Number>(fn));
    if (res.valueOf() != 996) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceFuncArg3(): int {
    let fn = (a: Object, b: Object, index: number): Object => {
        if (index < 0) {
            return new Number(-1);
        }
        return new Number((a as Number).valueOf() + (b as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduce(__asGenericABindexT<Number>(fn));
        return fail;
    } catch(e) {
        hasException = true;
    }
    if (hasException == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.reduce(__asGenericABindexT<Number>(fn));
    if (res.valueOf() != 503) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceFuncArg2(): int {
    let fn = (a: Object, b: Object): Object => {
        return new Number((a as Number).valueOf() + (b as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduce(__asGenericABT<Number>(fn));
        return fail;
    } catch(e) {
        hasException = true;
    }
    if (hasException == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.reduce(__asGenericABT<Number>(fn));
    if (res.valueOf() != 503) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceFuncArg4ArrayNumberWithInitialValue(): int {
    let fn = (a: Object, b: Object, index: number, arr: Array<Object>): Object => {
        if (index < 0 || index >= arr.length) {
            return new Number(-1);
        }
        return new Number((a as Number).valueOf() + (b as Number).valueOf() + (arr.at(index) as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let initialValue = new Number(0);
    let res = arrEmpty.reduce(__asGenericABindexArrT<Number>(fn), initialValue);
    if (res.valueOf() != initialValue.valueOf()) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    initialValue = -arr.at(0)!;
    res = arr.reduce(__asGenericABindexArrT<Number>(fn), initialValue);
    if (res.valueOf() != 996) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceFuncArg3WithInitialValue(): int {
    let fn = (a: Object, b: Object, index: number): Object => {
        if (index < 0) {
            return new Number(-1);
        }
        return new Number((a as Number).valueOf() + (b as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let initialValue = new Number(0);
    let res = arrEmpty.reduce(__asGenericABindexT<Number>(fn), initialValue);
    if (res.valueOf() != initialValue.valueOf()) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    res = arr.reduce(__asGenericABindexT<Number>(fn), initialValue);
    if (res.valueOf() != 503) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceFuncArg2WithInitialValue(): int {
    let fn = (a: Object, b: Object): Object => {
        return new Number((a as Number).valueOf() + (b as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let initialValue = new Number(0);
    let res = arrEmpty.reduce(__asGenericABT<Number>(fn), initialValue);
    if (res.valueOf() != initialValue.valueOf()) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    res = arr.reduce(__asGenericABT<Number>(fn), initialValue);
    if (res.valueOf() != 503) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function check(result: int, message: String): int {
    if(result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Exception catched: " + e);
        return 1;
    }
    if(result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
