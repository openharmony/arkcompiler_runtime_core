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

function checkArray<T extends Number | BigInt>(data: Array<T>, elem: T, expected: number, fromIndex: int | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        'data.lastIndexOf(elem: T, fromIndex: number | undefined) must return ' + expected);
}

function checkArray<T extends Number | BigInt>(data: Array<T>, elem: T, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, 'data.lastIndexOf(elem: T) must return ' + expected);
}

function testArrayOfBigInt() {
    let data = Array.of(-1n, BigInt(-9.223e+18), BigInt(-9.223e+18), BigInt(-9.223e+18), -8589934588n, -2147483648n, 0n,
        1802338304n, 4294967294n, 1802338304n, BigInt(2.3058e+16), BigInt(9.223e+18));
    checkArray(data, -1n, 0);
    checkArray(data, -10000000000n, -1);
    checkArray(data, -10000n, -1);
    checkArray(data, BigInt(-9.224e+18), -1);
    checkArray(data, BigInt(-9.223e+18), 3);
    checkArray(data, -8589934588n, 4);
    checkArray(data, 0n, 6);
    checkArray(data, 1802338304n, 9);
    checkArray(data, 4294967294n, 8);
    checkArray(data, BigInt(2.3058e+16), 10);
    checkArray(data, BigInt(9.223e+18), 11);
    checkArray(data, BigInt(9.224e+18), -1);
}

function testArrayOfBigIntFromIndex() {
    let data = Array.of(BigInt(-1.7e+308), BigInt(-3.6e+42), BigInt(-3.6e+42), 0n, BigInt(-0.6e+16), BigInt(0.4e+105),
        BigInt(-3.6e+42), BigInt(1.7e+308), BigInt(1.7e+308), 4294967294n, 1802338304n, -10000000000n,
        9007199254740991n * 9007199254740991n);
    checkArray(data, BigInt(-1.7e+308), 0, undefined);
    checkArray(data, BigInt(-1.7e+308), 0, 0);
    checkArray(data, BigInt(-1.7e+308), 0, -1);
    checkArray(data, BigInt(-1.7e+308), 0, 1);
    checkArray(data, BigInt(-1.7e+308), 0, data.length);
    checkArray(data, BigInt(-1.7e+308), 0, data.length + 1);
    checkArray(data, BigInt(-1.7e+308), 0, -data.length);
    checkArray(data, BigInt(-1.7e+308), 0, -(data.length - 1));
    checkArray(data, BigInt(-1.7e+308), -1, -(data.length + 1));
    checkArray(data, BigInt(1.7e+308), -1, undefined);
    checkArray(data, BigInt(1.7e+308), -1, 0);
    checkArray(data, BigInt(1.7e+308), 8, -1);
    checkArray(data, BigInt(1.7e+308), 8, -2);
    checkArray(data, BigInt(1.7e+308), -1, -7);
    checkArray(data, BigInt(1.7e+308), 7, -6);
    checkArray(data, BigInt(1.7e+308), 8, -5);
    checkArray(data, BigInt(1.7e+308), -1, -data.length);
    checkArray(data, BigInt(1.7e+308), 8, data.length);
    checkArray(data, BigInt(1.7e+308), 7, 7);
    checkArray(data, BigInt(1.7e+308), 8, 8);
    checkArray(data, BigInt(1.7e+308), -1, -(data.length + 1));
    checkArray(data, BigInt(1.7e+308), -1, -(data.length - 1));
    checkArray(data, BigInt(-3.6e+42), -1, undefined);
    checkArray(data, BigInt(-3.6e+42), -1, 0);
    checkArray(data, BigInt(-3.6e+42), 1, 1);
    checkArray(data, BigInt(-3.6e+42), 2, 2);
    checkArray(data, BigInt(-3.6e+42), 2, 3);
    checkArray(data, BigInt(-3.6e+42), 6, 6);
    checkArray(data, BigInt(-3.6e+42), 6, 7);
    checkArray(data, BigInt(-3.6e+42), -1, -(data.length + 1));
    checkArray(data, BigInt(-3.6e+42), 1, -(data.length - 1));
    checkArray(data, BigInt(-3.6e+42), 2, -(data.length - 2));
    checkArray(data, BigInt(-3.6e+42), -1, -data.length);
    checkArray(data, BigInt(-3.6e+42), 6, data.length);
    checkArray(data, BigInt(-3.6e+42), 6, -6);
    checkArray(data, BigInt(-3.6e+42), 6, -7);
    checkArray(data, BigInt(-3.6e+42), 2, -8);
    checkArray(data, 0n, -1, undefined);
    checkArray(data, 0n, -1, 0);
    checkArray(data, 0n, -1, 2);
    checkArray(data, 0n, 3, 3);
    checkArray(data, 0n, 3, 4);
    checkArray(data, 0n, 3, 5);
    checkArray(data, 0n, 3, data.length);
    checkArray(data, 0n, 3, data.length - 1);
    checkArray(data, 0n, -1, -data.length);
    checkArray(data, 0n, -1, -(data.length + 1));
    checkArray(data, 0n, -1, -(data.length - 2));
    checkArray(data, 0n, 3, -(data.length - 3));
    checkArray(data, 0n, 3, -(data.length - 4));
    checkArray(data, BigInt(-0.6e+16), -1, undefined);
    checkArray(data, BigInt(-0.6e+16), -1, 0);
    checkArray(data, BigInt(-0.6e+16), 4, -1);
    checkArray(data, BigInt(-0.6e+16), -1, 3);
    checkArray(data, BigInt(-0.6e+16), 4, 4);
    checkArray(data, BigInt(-0.6e+16), 4, 5);
    checkArray(data, BigInt(-0.6e+16), 4, -8);
    checkArray(data, BigInt(-0.6e+16), 4, -9);
    checkArray(data, BigInt(-0.6e+16), -1, -10);
    checkArray(data, 2147483648n, -1, undefined);
    checkArray(data, 2147483648n, -1, 0);
    checkArray(data, 2147483648n, -1, data.length);
    checkArray(data, 4294967294n, 9, data.length);
    checkArray(data, 4294967294n, 9, 9);
    checkArray(data, 4294967294n, -1, 8);
    checkArray(data, 4294967294n, 9, -4);
    checkArray(data, 1802338304n, 10, data.length);
    checkArray(data, 1802338304n, 10, data.length - 2);
    checkArray(data, 1802338304n, 10, -1);
    checkArray(data, 1802338304n, 10, 10);
    checkArray(data, 1802338304n, -1, 9);
    checkArray(data, -10000000000n, -1, undefined);
    checkArray(data, -10000000001n, -1, 0);
    checkArray(data, -10000000000n, 11, data.length);
    checkArray(data, -10000000000n, 11, -1);
    checkArray(data, -10000000000n, 11, data.length - 1);
    checkArray(data, -10000000000n, 11, data.length - 2);
    checkArray(data, -10000000000n, -1, data.length - 3);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, data.length);
    checkArray(data, 9007199254740991n * 9007199254740991n, 12, data.length - 1);
    checkArray(data, 9007199254740991n * 9007199254740991n, -1, data.length - 2);
}

function testArrayOfNumber() {
    let data = Array.of(-1.0, -9.223e+18, -9.223e+18, -9.223e+18, -8589934588.00001, -2147483648.0002, 0, 1802338304,
        4294967294.0, 1802338304.1, 2.3058e+16, 9.223e+18);
    checkArray(data, -1, 0);
    checkArray(data, -10000000000.0, -1);
    checkArray(data, -10000, -1);
    checkArray(data, -9.224e+18, -1);
    checkArray(data, -9.223e+18, 3);
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
    checkArray(data, -1.7e+308, 0, undefined);
    checkArray(data, -1.7e+308, 0, 0);
    checkArray(data, -1.7e+308, 0, 1);
    checkArray(data, -1.7e+308, 0, data.length);
    checkArray(data, -1.7e+308, 0, data.length - 1);
    checkArray(data, -1.7e+308, 0, data.length - 2);
    checkArray(data, -1.7e+308, 0, -data.length);
    checkArray(data, -1.7e+308, 0, -(data.length - 1));
    checkArray(data, -1.7e+308, 0, -1);
    checkArray(data, -1.7e+308, -1, -(data.length + 1));
    checkArray(data, 1.7e+308, -1, undefined);
    checkArray(data, 1.7e+308, -1, 0);
    checkArray(data, 1.7e+308, 7, -5);
    checkArray(data, 1.7e+308, 8, -4);
    checkArray(data, 1.7e+308, 8, -3);
    checkArray(data, 1.7e+308, -1, -data.length);
    checkArray(data, 1.7e+308, 8, data.length);
    checkArray(data, 1.7e+308, 7, data.length - 5);
    checkArray(data, 1.7e+308, 8, data.length - 4);
    checkArray(data, 1.7e+308, -1, -(data.length + 1));
    checkArray(data, 1.7e+308, -1, -(data.length - 1));
    checkArray(data, -3.6e+42, -1, undefined);
    checkArray(data, -3.6e+42, -1, 0);
    checkArray(data, -3.6e+42, 1, 1);
    checkArray(data, -3.6e+42, 2, 2);
    checkArray(data, -3.6e+42, 2, 3);
    checkArray(data, -3.6e+42, 6, 6);
    checkArray(data, -3.6e+42, 6, 8);
    checkArray(data, -3.6e+42, -1, -(data.length + 1));
    checkArray(data, -3.6e+42, 1, -(data.length - 1));
    checkArray(data, -3.6e+42, 6, -1);
    checkArray(data, -3.6e+42, 6, -6);
    checkArray(data, -3.6e+42, 6, -5);
    checkArray(data, -3.6e+42, 2, -7);
    checkArray(data, -3.6e+42, -1, -data.length);
    checkArray(data, -3.6e+42, 6, data.length);
    checkArray(data, 0.0, 3, 3);
    checkArray(data, 0.0, 3, 4);
    checkArray(data, 0.0, -1, 2);
    checkArray(data, 0.0, 3, data.length);
    checkArray(data, 0.0, -1, -data.length);
    checkArray(data, 0.0, -1, -(data.length + 1));
    checkArray(data, 0.0, -1, -(data.length - 1));
    checkArray(data, 0.0, 3, -(data.length - 3));
    checkArray(data, 0.0, -1, -(data.length - 2));
    checkArray(data, -0.6e+16, 4, data.length);
    checkArray(data, -0.6e+16, 4, 4);
    checkArray(data, -0.6e+16, 4, 5);
    checkArray(data, -0.6e+16, -1, 3);
    checkArray(data, -0.6e+16, -1, -9);
    checkArray(data, -0.6e+16, 4, -7);
    checkArray(data, -0.6e+16, 4, -8);
    checkArray(data, 2147483648, -1, data.length);
    checkArray(data, 4294967294, 9, data.length);
    checkArray(data, 4294967294, 9, -3);
    checkArray(data, 4294967294, -1, -4);
    checkArray(data, 1802338304.00001, -1, undefined);
    checkArray(data, 1802338304.00001, -1, 0);
    checkArray(data, 1802338304.00001, 10, data.length);
    checkArray(data, 1802338304.00001, 10, data.length - 1);
    checkArray(data, 1802338304.00001, 10, data.length - 2);
    checkArray(data, 1802338304.00001, -1, data.length - 3);
    checkArray(data, 1802338304.00001, 10, -1);
    checkArray(data, 1802338304.00001, 10, -2);
    checkArray(data, 1802338304.00001, -1, -3);
    checkArray(data, 1802338304, -1, data.length);
    checkArray(data, 1802338304, -1, -1);
    checkArray(data, -10000000000.99999, 11, data.length);
    checkArray(data, -10000000000.99999, 11, data.length - 1);
    checkArray(data, -10000000000.99999, -1, data.length - 2);
    checkArray(data, -10000000000.99999, 11, -1);
    checkArray(data, -10000000000.99999, -1, -2);
    checkArray(data, -10000000001, -1, data.length);
    checkArray(data, -10000000001, -1, -1);
}

class A {
    toString1 = (): number => {
        return 0;
    }
}

function testArrayOfMixedTypes() {
    let obj: A = {
        toString1: (): number => {
            return 0;
        }
    };
    let one = 1;
    let _float = -(4 / 3);
    let a: Array<number | string | undefined | null | boolean | object> =
    new Array<number | string | undefined | null | boolean | object>(+0, 0, -0, false, "false", undefined, "0", obj, _float, -(4 / 3), -1.3333333333333, "str", one, 1);

    arktest.assertEQ(a.lastIndexOf(-(4 / 3)), 9, 'a[9]=-(4/3)');
    arktest.assertEQ(a.lastIndexOf(0), 2, 'a[2] = -0, but using === -0 and 0 are equal');
    arktest.assertEQ(a.lastIndexOf(-0), 2, 'a[2] = -0');
    arktest.assertEQ(a.lastIndexOf(1), 13, 'a[13] = 1');
    arktest.assertEQ(a.lastIndexOf(obj), 7, 'a[7] = obj');

    arktest.assertEQ(a.lastIndexOf(true), -1, 'a.lastIndexOf(true)');
    arktest.assertEQ(a.lastIndexOf(5), -1, 'a.lastIndexOf(5)');
    arktest.assertEQ(a.lastIndexOf("str1"), -1, 'a.lastIndexOf("str1")');
    arktest.assertEQ(a.lastIndexOf(null), -1, 'a.lastIndexOf(null)');
    arktest.assertEQ(a.lastIndexOf(new Object()), -1, 'a.lastIndexOf(new Object())');
}

function testArrayElementTypeDiffersFromSearchElementType() {
    let a1: Array<string | boolean> = new Array<string | boolean>("true");
    arktest.assertEQ(a1.lastIndexOf(true), -1, 'a1: ["true"].lastIndexOf(true)');

    let a2: Array<string | number> = new Array<string | number>("0");
    arktest.assertEQ(a2.lastIndexOf(0), -1, 'a2: ["0"].lastIndexOf(0)');

    let a3: Array<boolean | number> = new Array<boolean | number>(false);
    arktest.assertEQ(a3.lastIndexOf(0), -1, 'a3: [false].lastIndexOf(0)');

    let a4: Array<undefined | number> = new Array<undefined | number>(undefined);
    arktest.assertEQ(a4.lastIndexOf(0), -1, 'a4: [undefined].lastIndexOf(0)');

    let a5: Array<null | number> = new Array<null | number>(null);
    arktest.assertEQ(a5.lastIndexOf(0), -1, 'a5: [null].lastIndexOf(0)');

    let arr: Array<number> = new Array<number>();
    let a6: Array<Array<number> | number> = new Array<Array<number> | number>(arr);
    arktest.assertEQ(a6.lastIndexOf(0), -1, 'a6: [[]].lastIndexOf(0)');

    let _undefined1 = undefined;
    let _undefined2: undefined;
    let a7: Array<number | string | undefined | null | boolean> = new Array<number | string | undefined | null | boolean>(undefined, _undefined1, _undefined2, true, 0, false, null, 1, "undefined", 1);
    arktest.assertEQ(a7.lastIndexOf(undefined), 2, 'a7.lastIndexOf(undefined)');

    let _false = false;
    let a8: Array<number | string | undefined | null | boolean> =  new Array<number | string | undefined | null | boolean>(_false, false, 0, "false", null, undefined)
    arktest.assertEQ(a8.lastIndexOf(_false), 1, 'a8.lastIndexOf(_false)');

    let _sfalse = "false"
    let a9: Array<number | string | undefined | null | boolean> = new Array<number | string | undefined | null | boolean>(false, 0, _sfalse, "false", null, undefined)
    arktest.assertEQ(a9.lastIndexOf(_sfalse), 3, 'a9.lastIndexOf(_sfalse)');

    let obj1 = new A();
    let obj2 = new A();
    let obj3 = obj1;
    let a10: Array<object> = new Array<object>(obj2, obj1, obj3, obj1, obj2)
    arktest.assertEQ(a10.lastIndexOf(obj3), 3, 'a10.lastIndexOf(obj3)');

    let _null = null;
    let a11: Array<number | string | undefined | null | boolean> =
    new Array<number | string | undefined | null | boolean>(null, _null, "null", undefined, 0, false, );
    arktest.assertEQ(a11.lastIndexOf(null), 1, 'a11.lastIndexOf(null)');
}

function testArraySelfReference() {
    let a: Array<number> = new Array<number>(0, 1, 2, 3);
    let arr1: Array<number | Array<number>> = new Array<number | Array<number>>(0, 1, a, 3);
    arktest.assertEQ(arr1.lastIndexOf(a), 2, 'a.lastIndexOf(a)');

    let b: Array<string> = new Array<string>("0,1");
    let arr2: Array<number | string | Array<string>> = new Array<number | string | Array<string>>(0, b, "0,1", 3);

    arktest.assertEQ(arr2.lastIndexOf(b.toString()), 2, 'arr2.lastIndexOf(b.toString())');
    arktest.assertEQ(arr2.lastIndexOf("0,1"), 2, 'arr2.lastIndexOf("0,1")');
}

function testArrayElementOwnDataProperty() {
    let arr: Array<boolean> = new Array<boolean>(true, true, true);
    arktest.assertEQ(arr.lastIndexOf(true), 2, '[true, true, true].lastIndexOf(true)');
    arr[2] = false;
    arktest.assertEQ(arr.lastIndexOf(true), 1, '[true, true, false].lastIndexOf(true)');
    arr[1] = false;
    arktest.assertEQ(arr.lastIndexOf(true), 0, '[true, false, false].lastIndexOf(true)');
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('Array.lastIndexOf');
    testSuite.addTest('Arrays of BigInt64: lastIndexOf for arrays', testArrayOfBigInt);
    testSuite.addTest('Arrays of BigInt64: lastIndexOf for arrays with fromIndex', testArrayOfBigIntFromIndex);
    testSuite.addTest('Arrays of Number: lastIndexOf for arrays', testArrayOfNumber);
    testSuite.addTest('Arrays of Number: lastIndexOf for arrays with fromIndex', testArrayOfNumberFromIndex);
    testSuite.addTest('Arrays of Mixed types: lastIndexOf for arrays', testArrayOfMixedTypes);
    testSuite.addTest('Arrays of Mixed types: type of array element is different from type of search element', testArrayElementTypeDiffersFromSearchElementType);
    testSuite.addTest('Arrays with self-reference: lastIndexOf for arrays with array elements', testArraySelfReference);
    testSuite.addTest('Arrays of Boolean: element to be retrieved is own data property on an Array', testArrayElementOwnDataProperty);
    return testSuite.run();
}
