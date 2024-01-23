/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
    failures += check((): int => { return createDefault()}, "Create default empty Array");
    failures += check((): int => { return createWithGivenZeroSize()}, "Create Array with given zero size");
    failures += check((): int => { return createWithGivenSize()}, "Create Array with given size");
    failures += check((): int => { return createWithNegativeInitialCapacity()}, "Create Array with negative capacity");
    failures += check((): int => { return isArrayRawArrayNumber()}, "isArrayRawArrayNumber");
    failures += check((): int => { return isArrayArrayNumber()}, "isArrayArrayNumber");
    failures += check((): int => { return isArrayNullUndefined()}, "isArrayNullUndefined");
    failures += check((): int => { return push()}, "push");
    failures += check((): int => { return pushECMA()}, "pushECMA");
    failures += check((): int => { return toSplicedArg2()}, "toSplicedArg2");
    failures += check((): int => { return toSplicedArg1()}, "toSplicedArg1");
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

function createDefault(): int {
    let arr = new Array<Number>;
    if (!(arr instanceof Array)) {
        return fail;
    }
    return arr.length == 0 ? success : fail
}

function createWithNegativeInitialCapacity(): int {
    try {
        new Array<Number>(-1);
    } catch(nse: NegativeArraySizeException) {
        return success;
    }
    return fail;
}

function createWithGivenZeroSize(): int {
    let arr = new Array<Number>(0);
    let result = arr.length == 0 ? success : fail;
    return result;
}

function createWithGivenSize(): int {
    let arr = new Array<Number>(10);
    let result = arr.length == 10 ? success : fail;
    return result;
}

function isArrayRawArrayNumber(): int {
    let n: Number[] = [];
    return Array.isArray(n) == true ? success : fail;
}

function isArrayArrayNumber(): int {
    let arr = new Array<Number>;
    return Array.isArray(arr) == true ? success : fail;
}

function isArrayNullUndefined(): int {
    if (Array.isArray(null) == true) {
        return fail;
    }
    if (Array.isArray(undefined) == true) {
        return fail;
    }
    return success;
}

function push(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < d.length; ++i) {
        arr.push(d[i])
        if (arr.at(i) != d[i]) {
            return fail;
        }
    }
    if (arr.length != d.length) {
        return fail;
    }
    return success;
}

function pushECMA(): int {
    let arr = new Array<Number>;
    arr.pushECMA(...d);
    if (arr.length != d.length) {
        return fail;
    }
    for (let i: int = 0; i < d.length; ++i) {
        if (arr.at(i) != d[i]) {
            return fail;
        }
    }
    return success;
}

function toSplicedArg1(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < dd.length; ++i) {
        arr.push(dd[i]);
    }
    let newArr = arr.toSpliced(1);
    if ((newArr.at(0) != 1) || (newArr.length != 1)) {
        return fail;
    }
    if (arr.length != dd.length) {
        return fail;
    }
    for (let i = 0; i < dd.length; i++) {
        if (arr[i] != dd[i]) {
            return fail;
        }
    }
    return success;
}

function toSplicedArg2(): int {
    let arr = new Array<Number>;
    for (let i: int = 0; i < dd.length; ++i) {
        arr.push(dd[i]);
    }
    let newArr = arr.toSpliced(1, 2);
    if ((newArr.length != 4) || (newArr[0] != 1) ||
        (newArr[1] != 4) || (newArr[2] != 0) ||
        (newArr[3] != 6)) {
        return fail;
    }
    if (arr.length != dd.length) {
        return fail;
    }
    for (let i = 0; i < dd.length; i++) {
        if (arr[i] != dd[i]) {
            return fail;
        }
    }
    return success;
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
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
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
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}
