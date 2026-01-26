/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function checkArray<T extends Number | BigInt>(data: Array<T>, elem: T, expected: int, fromIndex?: int): void {
    arktest.assertEQ(data.indexOf(elem, fromIndex), expected,
        'data.indexOf(elem: T, fromIndex?: number) must return ' + expected);
}

function checkArray<T extends Number | BigInt>(data: Array<T>, elem: T, expected: number): void {
    arktest.assertEQ(data.indexOf(elem), expected, 'data.indexOf(elem: T) must return ' + expected);
}

function testArrayOfBigInt() {
    let data = Array.of(-1n, BigInt(-9.223e+18), BigInt(-9.223e+18), BigInt(-9.223e+18), -8589934588n, -2147483648n, 0n,
        1802338304n, 4294967294n, 1802338304n, BigInt(2.3058e+16), BigInt(9.223e+18));
    checkArray(data, -1n, 0);
    checkArray(data, -10000000000n, -1);
    checkArray(data, -10000n, -1);
    checkArray(data, BigInt(-9.224e+18), -1);
    checkArray(data, BigInt(-9.223e+18), 1);
    checkArray(data, -8589934588n, 4);
    checkArray(data, 0n, 6);
    checkArray(data, 1802338304n, 7);
    checkArray(data, 4294967294n, 8);
    checkArray(data, BigInt(2.3058e+16), 10);
    checkArray(data, BigInt(9.223e+18), 11);
    checkArray(data, BigInt(9.224e+18), -1);
}

function testArrayOfBigIntFromIndex() {
    let data = Array.of(BigInt(-1.7e+308), BigInt(-3.6e+42), BigInt(-3.6e+42), 0n, BigInt(-0.6e+16), BigInt(0.4e+105),
        BigInt(-3.6e+42), BigInt(1.7e+308), BigInt(1.7e+308), 4294967294n, 1802338304n, -10000000000n,
        9007199254740991n * 9007199254740991n);
    checkArray(data, BigInt(-1.7e+308), 0, 0);
    checkArray(data, BigInt(-1.7e+308), -1, 1);
    checkArray(data, BigInt(-1.7e+308), 0, -data.length);
    checkArray(data, BigInt(-1.7e+308), -1, -(data.length - 1));
    checkArray(data, BigInt(-1.7e+308), -1, -1);
    checkArray(data, BigInt(-1.7e+308), 0, -(data.length + 1));
    checkArray(data, BigInt(1.7e+308), 7, 0);
    checkArray(data, BigInt(1.7e+308), -1, -2);
    checkArray(data, BigInt(1.7e+308), 7, -6);
    checkArray(data, BigInt(1.7e+308), 8, -5);
    checkArray(data, BigInt(1.7e+308), -1, -1);
    checkArray(data, BigInt(1.7e+308), 7, -data.length);
    checkArray(data, BigInt(1.7e+308), -1, data.length);
    checkArray(data, BigInt(1.7e+308), 7, 7);
    checkArray(data, BigInt(1.7e+308), 8, 8);
    checkArray(data, BigInt(1.7e+308), 7, -(data.length + 1));
    checkArray(data, BigInt(1.7e+308), 7, -(data.length - 1));
    checkArray(data, BigInt(-3.6e+42), 1, 1);
    checkArray(data, BigInt(-3.6e+42), 2, 2);
    checkArray(data, BigInt(-3.6e+42), 6, 3);
    checkArray(data, BigInt(-3.6e+42), -1, 7);
    checkArray(data, BigInt(-3.6e+42), 6, 6);
    checkArray(data, BigInt(-3.6e+42), 1, -(data.length + 1));
    checkArray(data, BigInt(-3.6e+42), 1, -(data.length - 1));
    checkArray(data, BigInt(-3.6e+42), 1, -data.length);
    checkArray(data, BigInt(-3.6e+42), -1, data.length);
    checkArray(data, BigInt(-3.6e+42), -1, -6);
    checkArray(data, BigInt(-3.6e+42), 6, -7);
    checkArray(data, 0n, 3, 3);
    checkArray(data, 0n, -1, 4);
    checkArray(data, 0n, -1, 5);
    checkArray(data, 0n, -1, data.length);
    checkArray(data, 0n, 3, -data.length);
    checkArray(data, 0n, 3, -(data.length + 1));
    checkArray(data, 0n, 3, -(data.length - 3));
    checkArray(data, 0n, -1, -(data.length - 4));
    checkArray(data, BigInt(-0.6e+16), 4, 0);
    checkArray(data, BigInt(-0.6e+16), 4, 4);
    checkArray(data, BigInt(-0.6e+16), -1, 5);
    checkArray(data, BigInt(-0.6e+16), -1, -8);
    checkArray(data, BigInt(-0.6e+16), 4, -9);
    checkArray(data, BigInt(-0.6e+16), 4, -10);
    checkArray(data, 2147483648n, -1, 0);
    checkArray(data, 4294967294n, 9, 0);
    checkArray(data, 1802338304n, 10, 0);
    checkArray(data, -10000000000n, 11, 0);
    checkArray(data, -10000000000n, -1, data.length);
    checkArray(data, -10000000000n, -1, data.length - 1);
    checkArray(data, -10000000000n, 11, data.length - 2);
    checkArray(data, -10000000001n, -1, 0);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, 0);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, 1);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, -data.length);
    checkArray(data, 9007199254740991n * 9007199254740991n, -1, data.length);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, data.length - 1);
}

function testArrayOfNumber() {
    let data = Array.of(-1.0, -9.223e+18, -9.223e+18, -9.223e+18, -8589934588.00001, -2147483648.0002, 0, 1802338304,
        4294967294.0, 1802338304.1, 2.3058e+16, 9.223e+18);
    checkArray(data, -1, 0);
    checkArray(data, -10000000000.0, -1);
    checkArray(data, -10000, -1);
    checkArray(data, -9.224e+18, -1);
    checkArray(data, -9.223e+18, 1);
    checkArray(data, -8589934588.00001, 4);
    checkArray(data, -2147483648.0002, 5);
    checkArray(data, 0, 6);
    checkArray(data, 1802338304, 7);
    checkArray(data, 4294967294.0, 8);
    checkArray(data, 1802338304.1, 9);
    checkArray(data, 1802338304.11, -1);
    checkArray(data, 2.3058e+16, 10);
    checkArray(data, 9.223e+18, 11);
    checkArray(data, 9.224e+18, -1);
}

function testArrayOfNumberFromIndex() {
    let data = Array.of(-1.7e+308, -3.6e+42, -3.6e+42, 0, -0.6e+16, 0.4e+105, -3.6e+42, 1.7e+308, 1.7e+308, 4294967294,
        1802338304.00001, -10000000000.99999);
    checkArray(data, -1.7e+308, 0, 0);
    checkArray(data, -1.7e+308, -1, 1);
    checkArray(data, -1.7e+308, 0, -data.length);
    checkArray(data, -1.7e+308, -1, -(data.length - 1));
    checkArray(data, -1.7e+308, -1, -1);
    checkArray(data, -1.7e+308, 0, -(data.length + 1));
    checkArray(data, 1.7e+308, 7, 0);
    checkArray(data, 1.7e+308, 7, -5);
    checkArray(data, 1.7e+308, 8, -4);
    checkArray(data, 1.7e+308, -1, -3);
    checkArray(data, 1.7e+308, 7, -data.length);
    checkArray(data, 1.7e+308, -1, data.length);
    checkArray(data, 1.7e+308, 7, data.length - 5);
    checkArray(data, 1.7e+308, 8, data.length - 4);
    checkArray(data, 1.7e+308, 7, -(data.length + 1));
    checkArray(data, 1.7e+308, 7, -(data.length - 1));
    checkArray(data, -3.6e+42, 1, 1);
    checkArray(data, -3.6e+42, 2, 2);
    checkArray(data, -3.6e+42, 6, 3);
    checkArray(data, -3.6e+42, 6, 6);
    checkArray(data, -3.6e+42, -1, 8);
    checkArray(data, -3.6e+42, 1, -(data.length + 1));
    checkArray(data, -3.6e+42, 1, -(data.length - 1));
    checkArray(data, -3.6e+42, -1, -2);
    checkArray(data, -3.6e+42, 6, -6);
    checkArray(data, -3.6e+42, -1, -5);
    checkArray(data, -3.6e+42, 1, -data.length);
    checkArray(data, -3.6e+42, -1, data.length);
    checkArray(data, 0.0, 3, 3);
    checkArray(data, 0.0, -1, 4);
    checkArray(data, 0.0, -1, 5);
    checkArray(data, 0.0, -1, data.length);
    checkArray(data, 0.0, 3, -data.length);
    checkArray(data, 0.0, 3, -(data.length + 1));
    checkArray(data, 0.0, 3, -(data.length - 3));
    checkArray(data, 0.0, -1, -(data.length - 4));
    checkArray(data, -0.6e+16, 4, 0);
    checkArray(data, -0.6e+16, 4, 4);
    checkArray(data, -0.6e+16, -1, 5);
    checkArray(data, -0.6e+16, -1, -7);
    checkArray(data, -0.6e+16, 4, -8);
    checkArray(data, -0.6e+16, 4, -9);
    checkArray(data, 2147483648, -1, 0);
    checkArray(data, 4294967294, 9, 0);
    checkArray(data, 1802338304.00001, 10, 0);
    checkArray(data, 1802338304.00001, -1, data.length);
    checkArray(data, 1802338304.00001, -1, data.length - 1);
    checkArray(data, 1802338304.00001, 10, data.length - 2);
    checkArray(data, 1802338304, -1, 0);
    checkArray(data, -10000000000.99999, 11, 0);
    checkArray(data, -10000000000.99999, -1, data.length);
    checkArray(data, -10000000000.99999, 11, data.length - 1);
    checkArray(data, -10000000001, -1, 0);
}


function testArrayOfBoolean() {
    let arr: Array<boolean> = new Array<boolean>(true, false);
    arktest.assertEQ(arr.indexOf(false, 0), 1, '[true, false].indexOf(false, 1)');
    arktest.assertEQ(arr.indexOf(true, +0), 0, '[true, false].indexOf(true, +0)');
    arktest.assertEQ(arr.indexOf(true, -0), 0, '[true].indexOf(true, -0)');
}


function testArrayOfMixedTypes() {
    let a: Array<number | string | object | undefined | null | boolean> = new Array<number | string | object | undefined | null | boolean>(123457);
    let _false = false;
    let b: object = new Object();
    let one = 1;
    let _float = -(4 / 3);

    a[100] = 1;
    a[99999] = "";
    a[10] = b;
    a[14] = _float
    a[5555] = 5.5;
    a[123450] = "0";
    a[123451] = +0;
    a[123452] = -0;
    a[123] = _false;
    a[321] = false
    a[467] = undefined


    arktest.assertEQ(a.indexOf(1), 100, 'a.indexOf(1)');
    arktest.assertEQ(a.indexOf(""), 99999, 'a.indexOf("")');
    arktest.assertEQ(a.indexOf("0"), 123450, 'a.indexOf("0")');
    arktest.assertEQ(a.indexOf(+0), 123451, 'a.indexOf(0) +0, +0===+0');
    arktest.assertEQ(a.indexOf(0), 123452, 'a.indexOf(0) -0, 0===-0');
    arktest.assertEQ(a.indexOf(-0), 123452, 'a.indexOf(-0) -0, -0===-0');
    
    arktest.assertEQ(a.indexOf(5.5), 5555, 'a.indexOf(5.5)');
    arktest.assertEQ(a.indexOf(-(4 / 3)), 14, 'a[14]=_float===-(4/3)');

    arktest.assertEQ(a.indexOf(true), -1, 'a.indexOf(true)');
    arktest.assertEQ(a.indexOf(5), -1, 'a.indexOf(5)');
    arktest.assertEQ(a.indexOf("str1"), -1, 'a.indexOf("str1")');
    
    arktest.assertEQ(a.indexOf(null), -1, 'a.indexOf(null)');
    a[86634] = null
    arktest.assertEQ(a.indexOf(null), 86634, 'a.indexOf(null)');

    arktest.assertEQ(a.indexOf(new Object()), -1, 'a.indexOf(new Object())');
    arktest.assertEQ(a.indexOf(b), 10, 'a.indexOf(b)');

    arktest.assertEQ(a.indexOf(false), 123, 'a.indexOf("false")');
    arktest.assertEQ(a.indexOf(undefined), 0, 'a.indexOf("undefined")');
}

function testArrayElementTypeDiffersFromSearchElementType() {
    let a1: Array<string | boolean> = new Array<string | boolean>("true");
    arktest.assertEQ(a1.indexOf(true), -1, '["true"].indexOf(true)');

    let a2: Array<string | number> = new Array<string | number>("0");
    arktest.assertEQ(a2.indexOf(0), -1, '["0"].indexOf(0)');

    let a3: Array<boolean | number> = new Array<boolean | number>(false);
    arktest.assertEQ(a3.indexOf(0), -1, '[false].indexOf(0)');

    let a4: Array<undefined | number> = new Array<undefined | number>(undefined);
    arktest.assertEQ(a4.indexOf(0), -1, '[undefined].indexOf(0)');

    let a5: Array<null | number> = new Array<null | number>(null);
    arktest.assertEQ(a5.indexOf(0), -1, '[null].indexOf(0)');

    let arr: Array<number> = new Array<number>();
    let a6: Array<Array<number> | number> = new Array<Array<number> | number>(arr);
    arktest.assertEQ(a6.indexOf(0), -1, '[[]].indexOf(0)');
}

function testArraySelfReference() {
    let a: Array<number> = new Array<number>(0, 1, 2, 3);
    let arr: Array<number | Array<number>> = new Array<number | Array<number>>(0, 1, a, 3);
    arktest.assertEQ(arr.indexOf(a), 2, 'a.indexOf(a)');
    arktest.assertEQ(arr.indexOf(3), 3, 'a.indexOf(3)');
}

function testArrayElementOwnDataProperty() {
    let arr: Array<boolean> = new Array<boolean>(true, true, true);
    arktest.assertEQ(arr.indexOf(true), 0, '[true, true, true].indexOf(true)');
    arr[0] = false;
    arktest.assertEQ(arr.indexOf(true), 1, '[false, true, true].indexOf(true)');
    arr[1] = false;
    arktest.assertEQ(arr.indexOf(true), 2, '[false, false, true].indexOf(true)');
}


function main(): int {
    let testSuite = new arktest.ArkTestsuite('Array.indexOf');
    testSuite.addTest('Arrays of BigInt64: indexOf for arrays', testArrayOfBigInt);
    testSuite.addTest('Arrays of BigInt64: indexOf for arrays with fromIndex', testArrayOfBigIntFromIndex);
    testSuite.addTest('Arrays of Number: indexOf for arrays', testArrayOfNumber);
    testSuite.addTest('Arrays of Number: indexOf for arrays with fromIndex', testArrayOfNumberFromIndex);
    testSuite.addTest('Arrays of Boolean: indexOf for arrays', testArrayOfBoolean);
    testSuite.addTest('Arrays of Mixed Types: indexOf for arrays with mixed types', testArrayOfMixedTypes);
    testSuite.addTest('Arrays of Mixed Types: type of array element is different from type of search element', testArrayElementTypeDiffersFromSearchElementType);
    testSuite.addTest('Arrays with self-reference: indexOf for arrays with array elements', testArraySelfReference);
    testSuite.addTest('Arrays of Boolean: element to be retrieved is own data property on an Array', testArrayElementOwnDataProperty);
    return testSuite.run();
}
