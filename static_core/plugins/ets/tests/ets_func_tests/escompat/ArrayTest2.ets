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
    failures += check((): int => { return key()}, "key");
    failures += check((): int => { return values()}, "values");
    failures += check((): int => { return iterator()}, "iterator");
    failures += check((): int => { return findWithFuncArg1()}, "findWithFuncArg1");
    failures += check((): int => { return findWithFuncArg2()}, "findWithFuncArg2");
    failures += check((): int => { return findWithFuncArg3ArrayNumber()}, "findWithFuncArg3ArrayNumber");
    failures += check((): int => { return findIndexFuncArg1()}, "findIndexFuncArg1");
    failures += check((): int => { return findIndexFuncArg2()}, "findIndexFuncArg2");
    failures += check((): int => { return findIndexFuncArg3ArrayNumber()}, "findIndexFuncArg3ArrayNumber");
    failures += check((): int => { return findLastIndexFuncArg1()}, "findLastIndexFuncArg1");
    failures += check((): int => { return findLastFuncArg1()}, "findLastFuncArg1");
    failures += check((): int => { return mapFuncArg1()}, "mapFuncArg1");
    failures += check((): int => { return mapFuncArg2()}, "mapFuncArg2");
    failures += check((): int => { return mapFuncArg3ArrayNumber()}, "mapFuncArg3ArrayNumber");
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

function key(): int {
    let arr = new Array<Number>;
    let iterator = arr.keys();
    let i: int = 0;
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if (i > 0){
                return fail;
            };
            break;
        }
        i = i + 1;
    }
    for (i = 0; i < dd.length; ++i) {
        arr.push(dd[i]);
    }
    iterator = arr.keys();
    let idx: Number = new Number(0);
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if ((idx != arr.length) || (idx != dd.length)) {
                return fail;
            }
            return success;
        }
        if ((v.value != idx) || (v.value == undefined)) {
            return fail;
        }
        idx = idx + 1;
    }
}

function values(): int {
    let arr = new Array<Number>;
    let iterator = arr.values();
    let i: int = 0;
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if ((i != 0) && (i != arr.length)) {
                return fail;
            }
            break;
        }
        i++;
    }
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i])
    }
    iterator = arr.values();
    i = 0;
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if ((i > 0) && (i != arr.length)) {
                return fail;
            }
            return success;
        }
        if (v.value != arr.at(i)) {
            return fail;
        }
        i++;
    }
}

function iterator(): int {
    let arr = new Array<Number>;
    let iterator = arr.iterator();
    let i: int = 0;
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if ((i > 0) && (i != arr.length)) {
                return fail;
            }
            break;
        }
        i++;
    }
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    iterator = arr.iterator();
    i = 0;
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if ((i > 0) && (i != arr.length)) {
                return fail;
            }
            return success;
        }
        if (v.value != arr.at(i)) {
            return fail;
        }
        i++;
    }
}

function findWithFuncArg1(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length as int; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object): boolean => { return (elem as Number).valueOf() == 12; };
    const fnNegative = (elem: Object): boolean => { return (elem as Number).valueOf() < 0; };
    const fnUndefined = (elem: Object): boolean => { return (elem as Number) == undefined; };
    const fnNull = (elem: Object): boolean => { return (elem as Number) == null; };
    if (arr.find(__asGenericBool1<Number>(fnPositive)) != 12) { return fail; }
    if (arr.find(__asGenericBool1<Number>(fnNegative)) != -1) { return fail; }
    if (arr.find(__asGenericBool1<Number>(fnUndefined)) != undefined) { return fail; }
    if (arr.find(__asGenericBool1<Number>(fnNull)) != undefined) { return fail; }
    return success;
}

function findWithFuncArg2(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object, index: number): boolean => {
        return (elem as Number).valueOf() == 12 && index > 0;
    };
    const fnNegative = (elem: Object, index: number): boolean => {
        return (elem as Number).valueOf() < 0 && index > 0;
    };
    const fnUndefined = (elem: Object, index: number): boolean => {
        return elem == undefined && index < 0;
    };
    const fnNull = (elem: Object, index: number): boolean => {
        return elem == null && index > 0;
    };
    if (arr.find(__asGenericBool2<Number>(fnPositive)) != 12) { return fail; }
    if (arr.find(__asGenericBool2<Number>(fnNegative)) != -1) { return fail; }
    if (arr.find(__asGenericBool2<Number>(fnUndefined)) != undefined) { return fail; }
    if (arr.find(__asGenericBool2<Number>(fnNull)) != undefined) { return fail; }
    return success;
}

function findWithFuncArg3ArrayNumber(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj[index] == elem && (elem as Number).valueOf() == 12 && index > 0;
    };
    const fnNegative = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj[index] == elem && (elem as Number).valueOf() < 0 && index > 0;
    };
    const fnUndefined = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj[index] == elem && elem == undefined && index < 0;
    };
    const fnNull = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj[index] == elem && elem == null && index > 0;
    };
    if (arr.find(__asGenericBool3<Number>(fnPositive)) != 12) { return fail; }
    if (arr.find(__asGenericBool3<Number>(fnNegative)) != -1) { return fail; }
    if (arr.find(__asGenericBool3<Number>(fnUndefined)) != undefined) { return fail; }
    if (arr.find(__asGenericBool3<Number>(fnNull)) != undefined) { return fail; }
    return success;
}

function findIndexFuncArg1(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object): boolean => { return (elem as Number).valueOf() == 12; };
    const fnNegative = (elem: Object): boolean => { return (elem as Number).valueOf() < 0; };
    const fnUndefined = (elem: Object): boolean => { return elem == undefined; };
    const fnNull = (elem: Object): boolean => { return elem == null; };
    if (arr.findIndex(__asGenericBool1<Number>(fnPositive)) != 9) { return fail; }
    if (arr.findIndex(__asGenericBool1<Number>(fnNegative)) != 1) { return fail; }
    if (arr.findIndex(__asGenericBool1<Number>(fnUndefined)) != -1) { return fail; }
    if (arr.findIndex(__asGenericBool1<Number>(fnNull)) != -1) { return fail; }
    return success;
}

function findIndexFuncArg2(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
       arr.push(d[i]);
    }
    const fnPositive = (elem: Object, index: number): boolean => {
        return (elem as Number).valueOf() == 12 && index > 0;
    };
    const fnNegative = (elem: Object, index: number): boolean => {
        return (elem as Number).valueOf() < 0 && index > 0;
    };
    const fnUndefined = (elem: Object, index: number): boolean => {
        return elem == undefined && index < 0;
    };
    const fnNull = (elem: Object, index: number): boolean => {
        return elem == null && index > 0;
    };
    if (arr.findIndex(__asGenericBool2<Number>(fnPositive)) != 9) { return fail; }
    if (arr.findIndex(__asGenericBool2<Number>(fnNegative)) != 1) { return fail; }
    if (arr.findIndex(__asGenericBool2<Number>(fnUndefined)) != -1) { return fail; }
    if (arr.findIndex(__asGenericBool2<Number>(fnNull)) != -1) { return fail; }
    return success;
}

function findIndexFuncArg3ArrayNumber(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj.at(index) == elem && (elem as Number).valueOf() == 12 && index > 0;
    };
    const fnNegative = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj.at(index) == elem && (elem as Number).valueOf() < 0 && index > 0;
    };
    const fnUndefined = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj.at(index) == elem && elem == undefined && index < 0;
    };
    const fnNull = (elem: Object, index: number, obj: Array<Object>): boolean => {
        return obj.at(index) == elem && elem == null && index > 0;
    };
    if (arr.findIndex(__asGenericBool3<Number>(fnPositive)) != 9) { return fail; }
    if (arr.findIndex(__asGenericBool3<Number>(fnNegative)) != 1) { return fail; }
    if (arr.findIndex(__asGenericBool3<Number>(fnUndefined)) != -1) { return fail; }
    if (arr.findIndex(__asGenericBool3<Number>(fnNull)) != -1) { return fail; }
    return success;
}

function findLastIndexFuncArg1(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object): boolean => { return (elem as Number).valueOf() == 12; };
    const fnNegative = (elem: Object): boolean => { return (elem as Number).valueOf() < 0; };
    const fnUndefined = (elem: Object): boolean => { return elem == undefined; };
    const fnNull = (elem: Object): boolean => { return elem == null; };
    if (arr.findLastIndex(__asGenericBool1<Number>(fnPositive)) != 9) { return fail; }
    if (arr.findLastIndex(__asGenericBool1<Number>(fnNegative)) != 3) { return fail; }
    if (arr.findLastIndex(__asGenericBool1<Number>(fnUndefined)) != -1) { return fail; }
    if (arr.findLastIndex(__asGenericBool1<Number>(fnNull)) != -1) { return fail; }
    return success;
}

function findLastFuncArg1(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    const fnPositive = (elem: Object): boolean => { return (elem as Number).valueOf() == 12; };
    const fnNegative = (elem: Object): boolean => { return (elem as Number).valueOf() < 0; };
    const fnUndefined = (elem: Object): boolean => { return elem == undefined; };
    const fnNull = (elem: Object): boolean => { return elem == null; };
    if (arr.findLast(__asGenericBool1<Number>(fnPositive)) != 12) { return fail; }
    if (arr.findLast(__asGenericBool1<Number>(fnNegative)) != -5) { return fail; }
    if (arr.findLast(__asGenericBool1<Number>(fnUndefined)) != undefined) { return fail; }
    if (arr.findLast(__asGenericBool1<Number>(fnNull)) != undefined) { return fail; }
    return success;
}

function mapFuncArg1(): int {
    let fn = (elem: Object): Object => {
        return new Number((elem as Number).valueOf() * (elem as Number).valueOf());
    };
    let arrEmpty = new Array<Number>;
    let res0 = arrEmpty.map<Number>(__asGenericU1<Number, Number>(fn));
    if (res0.length != arrEmpty.length) { return fail; }
    if (res0.length != 0) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.map<Number>(__asGenericU1<Number, Number>(fn));
    if (res.length != arr.length) { return fail; }
    for (let i: int = 0; i < res.length; ++i) {
        if (res.at(i) == undefined) { return fail; }
        if (res.at(i)!.valueOf() != arr.at(i)!.valueOf() * arr.at(i)!.valueOf()) {
            return fail;
        }
    }
    return success;
}

function mapFuncArg2(): int {
    let fn = (elem: Object, index: number): Object => {
        if (index < 4) {
            return new Number((elem as Number).valueOf() * (elem as Number).valueOf());
        } else {
            return new Number(0);
        }
    }
    let arrEmpty = new Array<Number>;
    let res0 = arrEmpty.map<Number>(__asGenericU2<Number, Number>(fn));
    if (res0.length != arrEmpty.length) { return fail; }
    if (res0.length != 0) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.map<Number>(__asGenericU2<Number, Number>(fn));
    if (res.length != arr.length) { return fail; }
    let tmp: Number;
    for (let i: int = 0; i < res.length; ++i) {
        if (res.at(i) == undefined) { return fail; }
        if (i < 4) {
            tmp = new Number(arr.at(i)!.valueOf() * arr.at(i)!.valueOf());
        } else {
            tmp = new Number(0);
        }
        if (res.at(i)!.valueOf() != tmp.valueOf()) {
            return fail;
        }
    }
    return success;
}

function mapFuncArg3ArrayNumber(): int {
    let fn = (elem: Object, index: number, arr: Array<Object>): Object => {
        if ((index < 4) && (index < arr.length)) {
            return new Number((arr.at(index) as Number).valueOf() * (arr.at(index) as Number).valueOf());
        } else {
            return new Number(0);
        }
    }
    let arrEmpty = new Array<Number>;
    let res0 = arrEmpty.map<Number>(__asGenericU3<Number, Number>(fn));
    if (res0.length != arrEmpty.length) { return fail; }
    if (res0.length != 0) { return fail; }

    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i]);
    }
    let res = arr.map<Number>(__asGenericU3<Number, Number>(fn));
    if (res.length != arr.length) { return fail; }
    let tmp: Number;
    for (let i: int = 0; i < res.length; ++i) {
        if (res.at(i) == undefined) { return fail; }
        if (i < 4) {
            tmp = new Number(arr.at(i)!.valueOf() * arr.at(i)!.valueOf());
        } else {
            tmp = new Number(0);
        }
        if (res.at(i)!.valueOf() != tmp.valueOf()) {
            return fail;
        }
    }
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
