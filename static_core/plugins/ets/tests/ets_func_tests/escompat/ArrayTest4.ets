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
    failures += check((): int => { return reduceRightFuncArg4ArrayNumber()}, "reduceRightFuncArg4ArrayNumber");
    failures += check((): int => { return reduceRightFuncArg3()}, "reduceRightFuncArg3");
    failures += check((): int => { return reduceRightFuncArg2()}, "reduceRightFuncArg2");
    failures += check((): int => { return reduceRightFuncArg4ArrayNumberWithInitialValue()}, "reduceRightFuncArg4ArrayNumberWithInitialValue");
    failures += check((): int => { return reduceRightFuncArg3WithInitialValue()}, "reduceRightFuncArg3WithInitialValue");
    failures += check((): int => { return reduceRightFuncArg2WithInitialValue()}, "reduceRightFuncArg2WithInitialValue");
    failures += check((): int => { return forEachFuncArg3ArrayNumber()}, "forEachFuncArg3ArrayNumber");
    failures += check((): int => { return forEachFuncArg2()}, "forEachFuncArg2");
    failures += check((): int => { return forEachFuncArg1()}, "forEachFuncArg1");
    failures += check((): int => { return sortFunc()}, "sortFunc");
    failures += check((): int => { return toSortedFunc()}, "toSortedFunc");
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

function reduceRightFuncArg4ArrayNumber(): int {
    let fn = (a: Object, b: Object, index: number, arr: Array<Object>): Object => {
        if (index < 0 || index >= arr.length) {
            return new Number(-1);
        }
        return new Number((a as Number).valueOf() + (b as Number).valueOf() + (arr.at(index) as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduceRight(__asGenericABindexArrT<Number>(fn));
        return fail;
    } catch(e) {
        hasException = true;
    }
    if (hasException == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.reduceRight(__asGenericABindexArrT<Number>(fn));
    if (res.valueOf() != 994) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceRightFuncArg3(): int {
    let fn = (a: Object, b: Object, index: number): Object => {
        if (index < 0) {
            return new Number(-1);
        }
        return new Number((a as Number).valueOf() + (b as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduceRight(__asGenericABindexT<Number>(fn));
        return fail;
    } catch(e) {
        hasException = true;
    }
    if (hasException == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.reduceRight(__asGenericABindexT<Number>(fn));
    if (res.valueOf() != 503) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceRightFuncArg2(): int {
    let fn = (a: Object, b: Object): Object => {
        return new Number((a as Number).valueOf() + (b as Number).valueOf());
    }
    let arrEmpty = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduceRight(__asGenericABT<Number>(fn));
        return fail;
    } catch(e) {
        hasException = true;
    }
    if (hasException == false) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.reduceRight(__asGenericABT<Number>(fn));
    if (res.valueOf() != 503) { return fail; }
    return success;
}

function reduceRightFuncArg4ArrayNumberWithInitialValue(): int {
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

function reduceRightFuncArg3WithInitialValue(): int {
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
    initialValue = -arr.at(0)!;
    res = arr.reduce(__asGenericABindexT<Number>(fn), initialValue);
    if (res.valueOf() != 493) { return fail; }
    if (res.valueOf() ==  -1) { return fail; }
    return success;
}

function reduceRightFuncArg2WithInitialValue(): int {
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
    initialValue = -arr.at(0)!;
    res = arr.reduce(__asGenericABT<Number>(fn), initialValue);
    if (res.valueOf() != 493) { return fail; }
    return success;
}

function forEachFuncArg3ArrayNumber(): int {
    let s = new String;
    let fn = (a: Object, index: number, arr: Array<Object>): void => {
        if (index < 0 && index >= arr.length) {
            s = "";
        } else {
            let tmp = new Number((arr.at(index) as Number).valueOf() + (a as Number).valueOf());
            s += tmp.toString() + ",";
        }
    }
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    arr.forEach(__asGenericVoid3<Number>(fn));
    if (s != "20,-2,474,-10,296,130,6,68,0,24,") { return fail; }
    return success;
}

function forEachFuncArg2(): int {
    let s = new String;
    let fn = (a: Object, index: number): void => {
        if (index < 0) {
            s = "";
        } else {
            s += (a as Number).toString() + ",";
        }
    }
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    arr.forEach(__asGenericVoid2<Number>(fn));
    if (s != "10,-1,237,-5,148,65,3,34,0,12,") { return fail; }
    return success;
}

function forEachFuncArg1(): int {
    let s = new String;
    let fn = (a: Object): void => {
        s += (a as Number).toString() + ",";
    }
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    arr.forEach(__asGenericVoid1<Number>(fn));
    if (s != "10,-1,237,-5,148,65,3,34,0,12,") { return fail; }
    return success;
}

function sortFunc(): int {
    let fn = (a: Object, b: Object): number => {
        let res: number = (a as Number).valueOf() - (b as Number).valueOf();
        return res;
    }
    let arrEmpty = new Array<Number>;
    let res0 = arrEmpty.sort(__asGenericABnumber<Number>(fn));
    if (res0.length > 0) { return fail; }
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.sort(__asGenericABnumber<Number>(fn));
    if (res.length != arr.length)   { return fail; }
    for (let i: int = 0; i < res.length; ++i) {
        if (res.at(i) == undefined) { return fail; }
    }
    if (res.at(0)!.valueOf() != -5)  { return fail; }
    if (res.at(1)!.valueOf() != -1)  { return fail; }
    if (res.at(2)!.valueOf() != -0)  { return fail; }
    if (res.at(3)!.valueOf() != 3)   { return fail; }
    if (res.at(4)!.valueOf() != 10)  { return fail; }
    if (res.at(5)!.valueOf() != 12)  { return fail; }
    if (res.at(6)!.valueOf() != 34)  { return fail; }
    if (res.at(7)!.valueOf() != 65)  { return fail; }
    if (res.at(8)!.valueOf() != 148) { return fail; }
    if (res.at(9)!.valueOf() != 237) { return fail; }
    return success;
}

function toSortedFunc(): int {
    let fn = (a: Object, b: Object): number => { return (a as Number).valueOf() - (b as Number).valueOf(); }
    let arrEmpty = new Array<Number>;
    let res0 = arrEmpty.sort(__asGenericABnumber<Number>(fn));
    if (res0.length > 0) { return fail; }
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.toSorted(__asGenericABnumber<Number>(fn));
    if (res.length != arr.length)   { return fail; }
    for (let i: int = 0; i < res.length; ++i) {
        if (res.at(i) == undefined) { return fail; }
    }
    if (res.at(0)!.valueOf() != -5)  { return fail; }
    if (res.at(1)!.valueOf() != -1)  { return fail; }
    if (res.at(2)!.valueOf() != -0)  { return fail; }
    if (res.at(3)!.valueOf() != 3)   { return fail; }
    if (res.at(4)!.valueOf() != 10)  { return fail; }
    if (res.at(5)!.valueOf() != 12)  { return fail; }
    if (res.at(6)!.valueOf() != 34)  { return fail; }
    if (res.at(7)!.valueOf() != 65)  { return fail; }
    if (res.at(8)!.valueOf() != 148) { return fail; }
    if (res.at(9)!.valueOf() != 237) { return fail; }
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
        return fail;
    }
    if(result == success) {
      return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
