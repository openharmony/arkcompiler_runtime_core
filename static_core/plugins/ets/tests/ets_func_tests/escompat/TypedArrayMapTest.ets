/**
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

function xorEncrypt(data: Int32Array, key: number): Int32Array {
    return new Int32Array(data.map(val => val ^ key));
}

function testInt32ArrayMapXor() {
    let original = new Int32Array([0x12345678, 0xABCDEF12, 0x11223344]);
    let key = 0xDEADBEEF;

    let encrypted = xorEncrypt(original, key);
    let decrypted = xorEncrypt(encrypted, key);

    arktest.assertEQ(encrypted[0], -862328681);
    arktest.assertEQ(encrypted[1], 1969246717);
    arktest.assertEQ(encrypted[2], -812675669);

    arktest.assertEQ(decrypted[0], original[0]);
    arktest.assertEQ(decrypted[1], original[1]);
    arktest.assertEQ(decrypted[2], original[2]);
}

function testInt8ArrayMap() {
    let arr = new Int8Array([1, 2, 3, 4, Byte.MAX_VALUE, Byte.MIN_VALUE]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 2);
    arktest.assertEQ(result[1], 4);
    arktest.assertEQ(result[2], 6);
    arktest.assertEQ(result[3], 8);
    arktest.assertEQ(result[4], -2);
    arktest.assertEQ(result[5], 0);
}

function testInt16ArrayMap() {
    let arr = new Int16Array([100, 200, 300, 400]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 200);
    arktest.assertEQ(result[1], 400);
    arktest.assertEQ(result[2], 600);
    arktest.assertEQ(result[3], 800);
}

function testInt32ArrayMap() {
    let arr = new Int32Array([100000, 200000, 300000, 400000]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 200000);
    arktest.assertEQ(result[1], 400000);
    arktest.assertEQ(result[2], 600000);
    arktest.assertEQ(result[3], 800000);
}

function testFloat32ArrayMap() {
    let arr = new Float32Array([1.5, 2.5, 3.5, 4.5]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 3.0);
    arktest.assertEQ(result[1], 5.0);
    arktest.assertEQ(result[2], 7.0);
    arktest.assertEQ(result[3], 9.0);
}

function testFloat64ArrayMap() {
    let arr = new Float64Array([1.5, 2.5, 3.5, 4.5]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 3.0);
    arktest.assertEQ(result[1], 5.0);
    arktest.assertEQ(result[2], 7.0);
    arktest.assertEQ(result[3], 9.0);
}

function testUint8ClampedArrayMap() {
    let arr = new Uint8ClampedArray([100, 150, 200, 250]);
    let result = arr.map(val => val + 50);

    arktest.assertEQ(result[0], 150);
    arktest.assertEQ(result[1], 200);
    arktest.assertEQ(result[2], 250);
    arktest.assertEQ(result[3], 255);
}

function testUint8ArrayMap() {
    let arr = new Uint8Array([100, 150, 200, 250]);
    let result = arr.map(val => val + 50);

    arktest.assertEQ(result[0], 150);
    arktest.assertEQ(result[1], 200);
    arktest.assertEQ(result[2], 250);
    arktest.assertEQ(result[3], 44);
}

function testUint16ArrayMap() {
    let arr = new Uint16Array([1000, 2000, 3000, 4000]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 2000);
    arktest.assertEQ(result[1], 4000);
    arktest.assertEQ(result[2], 6000);
    arktest.assertEQ(result[3], 8000);
}

function testUint32ArrayMap() {
    let arr = new Uint32Array([100000, 200000, 300000, 400000]);
    let result = arr.map(val => val * 2);

    arktest.assertEQ(result[0], 200000);
    arktest.assertEQ(result[1], 400000);
    arktest.assertEQ(result[2], 600000);
    arktest.assertEQ(result[3], 800000);
}

function testBigUint64ArrayMap() {
    let arr = new BigUint64Array([100000n, 200000n, 300000n, 400000n]);
    let result = arr.map(val => val * 2n);

    arktest.assertEQ(result[0], 200000n);
    arktest.assertEQ(result[1], 400000n);
    arktest.assertEQ(result[2], 600000n);
    arktest.assertEQ(result[3], 800000n);
}

function testBigInt64ArrayMap() {
    let arr = new BigInt64Array([100000n, -200000n, 300000n, -400000n]);
    let result = arr.map(val => val * 2n);

    arktest.assertEQ(result[0], 200000n);
    arktest.assertEQ(result[1], -400000n);
    arktest.assertEQ(result[2], 600000n);
    arktest.assertEQ(result[3], -800000n);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray.map tests')
    suite.addTest('Int32Array.map XOR encryption', testInt32ArrayMapXor)
    suite.addTest('Int8Array.map', testInt8ArrayMap)
    suite.addTest('Int16Array.map', testInt16ArrayMap)
    suite.addTest('Int32Array.map', testInt32ArrayMap)
    suite.addTest('Float32Array.map', testFloat32ArrayMap)
    suite.addTest('Float64Array.map', testFloat64ArrayMap)
    suite.addTest('Uint8ClampedArray.map', testUint8ClampedArrayMap)
    suite.addTest('Uint8Array.map', testUint8ArrayMap)
    suite.addTest('Uint16Array.map', testUint16ArrayMap)
    suite.addTest('Uint32Array.map', testUint32ArrayMap)
    suite.addTest('BigUint64Array.map', testBigUint64ArrayMap)
    suite.addTest('BigInt64Array.map', testBigInt64ArrayMap)
    return suite.run()
}
