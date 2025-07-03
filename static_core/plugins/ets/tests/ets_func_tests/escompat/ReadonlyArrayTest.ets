/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

class Arrays {
  static copy<T>(src: Array<T>): Array<T> {
    const copy = new Array<T>(src.length);
    for (let i = 0;i < src.length;i++) copy[i] = src[i];
    return copy;
  }

  static copy(src: FixedArray<Number>): Array<Number> {
    const copy = new Array<Number>(src.length);
    for (let i = 0;i < src.length;i++) copy[i] = src[i];
    return copy;
  }

  static equals<T>(left: ReadonlyArray<T>, right: ReadonlyArray<T>): boolean {
    if (left.length != right.length) return false;

    for (let i = 0;i < left.length;i++) {
      if (left[i] != right[i]) return false;
    }

    return true;
  }

  static equals<T>(left: Array<T>, right: Array<T>): boolean {
    const leftReadOnly = left as ReadonlyArray<T>;
    const rightReadOnly = right as ReadonlyArray<T>;
    return Arrays.equals(leftReadOnly, rightReadOnly);
  }

    // NOTE(smirnovandrei) may be left: ArrayLike can help to avoid this code duplication (the same code as equals(ReadonlyArray))
  static equals<T>(left: FixedArray<Number>, right: Array<T>): boolean {
    if (left.length != right.length) return false;

    for (let i = 0;i < left.length;i++) {
      if (left[i] != right[i]) return false;
    }

    return true;
  }
}

const TEST_ARRAY: FixedArray<Number> = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];

function main(): int {
    let failedTestsCount = 0;

    failedTestsCount += runTest("findWithFuncArg3ReadonlyArray", findWithFuncArg3ReadonlyArray);
    failedTestsCount += runTest("findIndexFuncArg3ReadonlyArray", findIndexFuncArg3ReadonlyArray);
    failedTestsCount += runTest("mapFuncArg3ReadonlyArray", mapFuncArg3ReadonlyArray);
    failedTestsCount += runTest("everyFuncArg3ReadonlyArray", everyFuncArg3ReadonlyArray);
    failedTestsCount += runTest("filterFuncArg3ReadonlyArray", filterFuncArg3ReadonlyArray);
    failedTestsCount += runTest("reduceFuncArg4ReadonlyArray", reduceFuncArg4ReadonlyArray);
    failedTestsCount += runTest("reduceFuncArg4ReadonlyArrayWithInitialValue", reduceFuncArg4ReadonlyArrayWithInitialValue);
    failedTestsCount += runTest("reduceRightFuncArg4ReadonlyArray", reduceRightFuncArg4ReadonlyArray);
    failedTestsCount += runTest("reduceRightFuncArg4ReadonlyArrayWithInitialValue", reduceRightFuncArg4ReadonlyArrayWithInitialValue);
    failedTestsCount += runTest("forEachFuncArg3ReadonlyArray", forEachFuncArg3ReadonlyArray);

    failedTestsCount += runTest("testIteratorWithEmptyReadonlyArray", testIteratorWithEmptyReadonlyArray);
    failedTestsCount += runTest("testIteratorWithNonEmptyReadonlyArray", testIteratorWithNonEmptyReadonlyArray);
   
    return printReport("ReadonlyArrayTest failures: ", failedTestsCount);
}

const RESULT_SUCCESS: int = 0;
const RESULT_FAILURE: int = 1;

function filterPositiveElement(elem: Number, index: number, arr: ReadonlyArray<Number>): boolean {
    return arr[index] == elem && elem.valueOf() == 12 && index > 0;
}

function filterNegativeElement(elem: Number, index: number, arr: ReadonlyArray<Number>): boolean {
     return arr[index] == elem && elem.valueOf() < 0 && index > 0;
}

function filterUndefinedElement(elem: Number, index: number, arr: ReadonlyArray<Number>): boolean {
    return arr[index] == elem && elem == undefined && index < 0;
}

function filterNullElement(elem: Number, index: number, arr: ReadonlyArray<Number>) {
    return arr[index] == elem && elem == null && index > 0;
}

function findWithFuncArg3ReadonlyArray(): int {
    const fnNull = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem == null && index > 0; };

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    if (arr.find(filterPositiveElement) != 12) return RESULT_FAILURE;
    if (arr.find(filterNegativeElement) != -1) return RESULT_FAILURE;
    if (arr.find(filterUndefinedElement) != undefined) return RESULT_FAILURE;
    if (arr.find(filterNullElement) != undefined) return RESULT_FAILURE;

    return RESULT_SUCCESS;
}

function findIndexFuncArg3ReadonlyArray(): int {
    let fnNegative = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem.valueOf() < 0 && index > 0; };
    let fnUndefined = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem == undefined && index < 0; };
    let fnNull = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem == null && index > 0; };

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    if (arr.findIndex(filterPositiveElement) != 9) return RESULT_FAILURE;
    if (arr.findIndex(filterNegativeElement) != 1) return RESULT_FAILURE;
    if (arr.findIndex(filterUndefinedElement) != -1) return RESULT_FAILURE;
    if (arr.findIndex(filterNullElement) != -1) return RESULT_FAILURE;

    return RESULT_SUCCESS;
}

function mapFuncArg3ReadonlyArray(): int {
    let mappingFunc = (v: Number, index: number, arr: ReadonlyArray<Number>): Number =>
        (index < 4) && (index < arr.length) ? Number.valueOf(arr[index] * arr[index]) : Number.valueOf(0);

    let arrEmpty: ReadonlyArray<Number> = new Array<Number>;
    let res0 = arrEmpty.map<Number>(mappingFunc);
    if (res0.length != arrEmpty.length) return RESULT_FAILURE;
    if (res0.length != 0) return RESULT_FAILURE;

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.map<Number>(mappingFunc);
    if (res.length != arr.length) return RESULT_FAILURE;
    for (let i: int = 0; i < res.length; ++i) {
        const expected = i < 4 ? Number.valueOf(arr[i] * arr[i]) : Number.valueOf(0);
        if (res[i].valueOf() != expected.valueOf()) {
            return RESULT_FAILURE;
        }
    }
    return RESULT_SUCCESS;
}

function everyFuncArg3ReadonlyArray(): int {
    const emptyArray: ReadonlyArray<Number> = new Array<Number>;
    if (!emptyArray.every(filterPositiveElement)) return RESULT_FAILURE;

    const testArray: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    const everyResult = testArray.every((n: Number, i: number, ns: ReadonlyArray<Number>) => n >= -5 && n <= 237);

    if (!everyResult) return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

function filterFuncArg3ReadonlyArray(): int {
    let fnTrue = (v: Number, index: number, arr: ReadonlyArray<Number>): boolean => {
        return arr[index].valueOf() == v.valueOf() && v.valueOf() > -1000;
    }
    let arrEmpty = new Array<Number>;
    if (arrEmpty.filter(fnTrue).length != 0) return RESULT_FAILURE;

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.filter(fnTrue);

    if (!Arrays.equals(res, arr)) return RESULT_FAILURE;

    let fnFalse = (v: Number, index: number, arr: ReadonlyArray<Number>): boolean => {
        return arr.at(index) == v && v == undefined;
    }
    if (arr.filter(fnFalse).length != 0) return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

function reduceFunc(a: Number, b: Number, index: number, arr: ReadonlyArray<Number>): Number {
    if (index < 0 || index >= arr.length) {
        return Number.valueOf(-1);
    }
    return new Number(a + b + arr[index]);
}

function reduceFuncArg4ReadonlyArray(): int {
    let arrEmpty: ReadonlyArray<Number> = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduce(reduceFunc);
        return RESULT_FAILURE;
    } catch(e) {
        hasException = true;
    }
    if (!hasException) return RESULT_FAILURE;

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.reduce(reduceFunc);
    if (res.valueOf() != 996) return RESULT_FAILURE;
    if (res.valueOf() ==  -1) return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

// NOTE(smirnovandrei) add 'test' prefix to all test functions
function reduceFuncArg4ReadonlyArrayWithInitialValue(): int {
    let arrEmpty = new Array<Number>;
    let initialValue = new Number(0);
    let res = arrEmpty.reduce(reduceFunc, initialValue);
    if (res.valueOf() != initialValue.valueOf()) return RESULT_FAILURE;

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    initialValue = Number.valueOf(-arr[0].valueOf());
    res = arr.reduce(reduceFunc, initialValue);
    if (res.valueOf() != 996) return RESULT_FAILURE;
    if (res.valueOf() ==  -1) return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

function reduceRightFuncArg4ReadonlyArray(): int {
    let arrEmpty: ReadonlyArray<Number> = new Array<Number>;
    let hasException: boolean = false;
    try {
        let res = arrEmpty.reduceRight(reduceFunc);
        return RESULT_FAILURE;
    } catch(e) {
        hasException = true;
    }
    if (!hasException) return RESULT_FAILURE;

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.reduceRight(reduceFunc);
    if (res.valueOf() != 994) return RESULT_FAILURE;
    if (res.valueOf() == -1) return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

function reduceRightFuncArg4ReadonlyArrayWithInitialValue(): int {
    let arrEmpty = new Array<Number>;
    let initialValue = new Number(0);
    let res = arrEmpty.reduce(reduceFunc, initialValue);
    if (res.valueOf() != initialValue.valueOf()) return RESULT_FAILURE;

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    initialValue = Number.valueOf(-arr[0].valueOf());
    res = arr.reduce(reduceFunc, initialValue);
    if (res.valueOf() != 996) return RESULT_FAILURE;
    if (res.valueOf() ==  -1) return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

function forEachFuncArg3ReadonlyArray(): int {
    let s = new String;
    let fn = (a: Number, index: number, arr: ReadonlyArray<Number>): void => {
        if (index < 0 && index >= arr.length) {
            s = "";
        } else {
            let tmp = new Number(arr[index].valueOf() + a.valueOf());
            s += tmp.toString() + ",";
        }
    }

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    arr.forEach(fn);
    if (s != "20,-2,474,-10,296,130,6,68,0,24,") return RESULT_FAILURE;
    return RESULT_SUCCESS;
}

function runTest(testName: string, testBody: () => int): int {
    try {
        const testResult = testBody();

        if (testResult == RESULT_FAILURE) {
            console.println("\nFAILED: " + testName);
            return RESULT_FAILURE;
        }

        return RESULT_SUCCESS;
    } catch(e) {
        console.println("FAILED: " + testName + " => Exception catched: " + e);
        return RESULT_FAILURE;
    }
}

function testIteratorWithEmptyReadonlyArray(): int {
    const emptyArray: ReadonlyArray<Number> = new Array<Number>;
    const iterator = emptyArray.$_iterator();
    const v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }
    return RESULT_SUCCESS;
}

function testIteratorWithNonEmptyReadonlyArray(): int {
    const arr: ReadonlyArray<Number> = new Array<Number>(1, 2, 3);
    const iterator = arr.$_iterator();

    let v = iterator.next();
    if (v.done || v.value != 1) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (v.done || v.value != 2) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (v.done || v.value != 3) {
        return RESULT_FAILURE;
    }

    v = iterator.next();
    if (!v.done) {
        return RESULT_FAILURE;
    }

    return RESULT_SUCCESS;
}

function printReport(banner: string, failedTestsCount: int): int {
    if (failedTestsCount == 0) {
      return RESULT_SUCCESS;
    }

    console.println("\nFAILED: " + banner);
    return RESULT_FAILURE;
}
