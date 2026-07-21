/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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


function int8ArrayWithTest() {
    let arr = new Int8Array([10, 20, 30, 40]);
    // positive index
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    // negative index within bounds
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    // original array unchanged
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    // single element array, negative index
    let single = new Int8Array([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    // index out of bounds (positive)
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    // negative index out of bounds
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function uint8ArrayWithTest() {
    let arr = new Uint8Array([10, 20, 30, 40]);
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new Uint8Array([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function uint8ClampedArrayWithTest() {
    let arr = new Uint8ClampedArray([10, 20, 30, 40]);
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new Uint8ClampedArray([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function int16ArrayWithTest() {
    let arr = new Int16Array([10, 20, 30, 40]);
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new Int16Array([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function uint16ArrayWithTest() {
    let arr = new Uint16Array([10, 20, 30, 40]);
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new Uint16Array([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function int32ArrayWithTest() {
    let arr = new Int32Array([10, 20, 30, 40]);
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new Int32Array([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function uint32ArrayWithTest() {
    let arr = new Uint32Array([10, 20, 30, 40]);
    let r1 = arr.with(0, 99);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new Uint32Array([100]);
    let r5 = single.with(-1, 24);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50) }, new RangeError('Invalid typed array index'));
}

function bigInt64ArrayWithTest() {
    let arr = new BigInt64Array([10n, 20n, 30n, 40n]);
    let r1 = arr.with(0, 99n);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88n);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77n);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66n);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new BigInt64Array([100n]);
    let r5 = single.with(-1, 24n);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50n) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50n) }, new RangeError('Invalid typed array index'));
}

function bigUint64ArrayWithTest() {
    let arr = new BigUint64Array([10n, 20n, 30n, 40n]);
    let r1 = arr.with(0, 99n);
    arktest.assertEQ(r1.toString(), '99,20,30,40');
    let r2 = arr.with(3, 88n);
    arktest.assertEQ(r2.toString(), '10,20,30,88');
    let r3 = arr.with(-1, 77n);
    arktest.assertEQ(r3.toString(), '10,20,30,77');
    let r4 = arr.with(-4, 66n);
    arktest.assertEQ(r4.toString(), '66,20,30,40');
    arktest.assertEQ(arr.toString(), '10,20,30,40');
    let single = new BigUint64Array([100n]);
    let r5 = single.with(-1, 24n);
    arktest.assertEQ(r5.toString(), '24');
    arktest.expectError((): void => { arr.with(4, 50n) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 50n) }, new RangeError('Invalid typed array index'));
}

function float32ArrayWithTest() {
    let arr = new Float32Array([1.5, 2.5, 3.5, 4.5]);
    let r1 = arr.with(0, 9.5);
    arktest.assertEQ(r1.toString(), '9.5,2.5,3.5,4.5');
    let r2 = arr.with(3, 8.5);
    arktest.assertEQ(r2.toString(), '1.5,2.5,3.5,8.5');
    let r3 = arr.with(-1, 7.5);
    arktest.assertEQ(r3.toString(), '1.5,2.5,3.5,7.5');
    let r4 = arr.with(-4, 6.5);
    arktest.assertEQ(r4.toString(), '6.5,2.5,3.5,4.5');
    arktest.assertEQ(arr.toString(), '1.5,2.5,3.5,4.5');
    let single = new Float32Array([10.5]);
    let r5 = single.with(-1, 2.5);
    arktest.assertEQ(r5.toString(), '2.5');
    arktest.expectError((): void => { arr.with(4, 5.5) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 5.5) }, new RangeError('Invalid typed array index'));
}

function float64ArrayWithTest() {
    let arr = new Float64Array([1.5, 2.5, 3.5, 4.5]);
    let r1 = arr.with(0, 9.5);
    arktest.assertEQ(r1.toString(), '9.5,2.5,3.5,4.5');
    let r2 = arr.with(3, 8.5);
    arktest.assertEQ(r2.toString(), '1.5,2.5,3.5,8.5');
    let r3 = arr.with(-1, 7.5);
    arktest.assertEQ(r3.toString(), '1.5,2.5,3.5,7.5');
    let r4 = arr.with(-4, 6.5);
    arktest.assertEQ(r4.toString(), '6.5,2.5,3.5,4.5');
    arktest.assertEQ(arr.toString(), '1.5,2.5,3.5,4.5');
    let single = new Float64Array([10.5]);
    let r5 = single.with(-1, 2.5);
    arktest.assertEQ(r5.toString(), '2.5');
    arktest.expectError((): void => { arr.with(4, 5.5) }, new RangeError('Invalid typed array index'));
    arktest.expectError((): void => { arr.with(-5, 5.5) }, new RangeError('Invalid typed array index'));
}

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray with method tests');
    suite.addTest('for Int8Array', int8ArrayWithTest);
    suite.addTest('for Uint8Array', uint8ArrayWithTest);
    suite.addTest('for Uint8ClampedArray', uint8ClampedArrayWithTest);
    suite.addTest('for Int16Array', int16ArrayWithTest);
    suite.addTest('for Uint16Array', uint16ArrayWithTest);
    suite.addTest('for Int32Array', int32ArrayWithTest);
    suite.addTest('for Uint32Array', uint32ArrayWithTest);
    suite.addTest('for BigInt64Array', bigInt64ArrayWithTest);
    suite.addTest('for BigUint64Array', bigUint64ArrayWithTest);
    suite.addTest('for Float32Array', float32ArrayWithTest);
    suite.addTest('for Float64Array', float64ArrayWithTest);
    return suite.run();
}