/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

function testInt8Array() {
    let sample = new Int8Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '127'), true);
    arktest.assertEQ(sample[0], 127);

    arktest.assertEQ(Reflect.set(sample, '0', '0x7F'), true);
    arktest.assertEQ(sample[0], 127);

    arktest.assertEQ(Reflect.set(sample, '0', '0o77'), true);
    arktest.assertEQ(sample[0], 63);

    arktest.assertEQ(Reflect.set(sample, '0', '0b101010'), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '3.9'), true);
    arktest.assertEQ(sample[0], 3);

    arktest.assertEQ(Reflect.set(sample, '0', 7.8), true);
    arktest.assertEQ(sample[0], 7);

    arktest.assertEQ(Reflect.set(sample, '0', '-0x1F'), true);
    arktest.assertEQ(sample[0], 0);
}

function testInt16Array() {
    let sample = new Int16Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '32767'), true);
    arktest.assertEQ(sample[0], 32767);

    arktest.assertEQ(Reflect.set(sample, '0', '0x7FFF'), true);
    arktest.assertEQ(sample[0], 32767);

    arktest.assertEQ(Reflect.set(sample, '0', '0o77777'), true);
    arktest.assertEQ(sample[0], 32767);

    arktest.assertEQ(Reflect.set(sample, '0', '0b111111111111111'), true);
    arktest.assertEQ(sample[0], 32767);

    arktest.assertEQ(Reflect.set(sample, '0', '123.4'), true);
    arktest.assertEQ(sample[0], 123);

    arktest.assertEQ(Reflect.set(sample, '0', 456.789), true);
    arktest.assertEQ(sample[0], 456);
}

function testInt32Array() {
    let sample = new Int32Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '2147483647'), true);
    arktest.assertEQ(sample[0], 2147483647);

    arktest.assertEQ(Reflect.set(sample, '0', '0x7FFFFFFF'), true);
    arktest.assertEQ(sample[0], 2147483647);

    arktest.assertEQ(Reflect.set(sample, '0', '0o17777777777'), true);
    arktest.assertEQ(sample[0], 2147483647);

    arktest.assertEQ(Reflect.set(sample, '0', '0b1111111111111111111111111111111'), true);
    arktest.assertEQ(sample[0], 2147483647);

    arktest.assertEQ(Reflect.set(sample, '0', '789.123'), true);
    arktest.assertEQ(sample[0], 789);

    arktest.assertEQ(Reflect.set(sample, '0', 1000.456), true);
    arktest.assertEQ(sample[0], 1000);
}

function testBigInt64Array() {
    let sample = new BigInt64Array([42n]);
    arktest.assertEQ(Reflect.set(sample, '1.1', new Long(1)), true);
    arktest.assertEQ(sample[0], 42n);

    arktest.assertEQ(Reflect.set(sample, '0.0001', new Long(1)), true);
    arktest.assertEQ(sample[0], 42n);

    arktest.assertEQ(Reflect.set(sample, '0', 99n), true);
    arktest.assertEQ(sample[0], 99n);

    arktest.assertEQ(Reflect.set(sample, '0', '0x2A'), true);
    arktest.assertEQ(sample[0], 42n);

    arktest.assertEQ(Reflect.set(sample, '0', '0b101010'), true);
    arktest.assertEQ(sample[0], 42n);
}

function testUint8Array() {
    let sample = new Uint8Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '255'), true);
    arktest.assertEQ(sample[0], 255);

    arktest.assertEQ(Reflect.set(sample, '0', '0xFF'), true);
    arktest.assertEQ(sample[0], 255);

    arktest.assertEQ(Reflect.set(sample, '0', '0b11111111'), true);
    arktest.assertEQ(sample[0], 255);

    arktest.assertEQ(Reflect.set(sample, '0', '0o377'), true);
    arktest.assertEQ(sample[0], 255);

    arktest.assertEQ(Reflect.set(sample, '0', '3.14'), true);
    arktest.assertEQ(sample[0], 3);

    arktest.assertEQ(Reflect.set(sample, '0', 5.9), true);
    arktest.assertEQ(sample[0], 5);
}

function testUint16Array() {
    let sample = new Uint16Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '65535'), true);
    arktest.assertEQ(sample[0], 65535);

    arktest.assertEQ(Reflect.set(sample, '0', '0xFFFF'), true);
    arktest.assertEQ(sample[0], 65535);

    arktest.assertEQ(Reflect.set(sample, '0', '0o177777'), true);
    arktest.assertEQ(sample[0], 65535);

    arktest.assertEQ(Reflect.set(sample, '0', '0b1111111111111111'), true);
    arktest.assertEQ(sample[0], 65535);

    arktest.assertEQ(Reflect.set(sample, '0', '1234.56'), true);
    arktest.assertEQ(sample[0], 1234);

    arktest.assertEQ(Reflect.set(sample, '0', 2345.78), true);
    arktest.assertEQ(sample[0], 2345);
}

function testUint32Array() {
    let sample = new Uint32Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1), true);
    arktest.assertEQ(sample[0], 42);

    arktest.assertEQ(Reflect.set(sample, '0', '8765.4321'), true);
    arktest.assertEQ(sample[0], 8765);

    arktest.assertEQ(Reflect.set(sample, '0', 123456.789), true);
    arktest.assertEQ(sample[0], 123456);
}

function testBigUint64Array() {
    let sample = new BigUint64Array([42n]);
    arktest.assertEQ(Reflect.set(sample, '1.1', new Long(1)), true);
    arktest.assertEQ(sample[0], 42n);

    arktest.assertEQ(Reflect.set(sample, '0.0001', new Long(1)), true);
    arktest.assertEQ(sample[0], 42n);

    arktest.assertEQ(Reflect.set(sample, '0', BigInt('123456789')), true);
    arktest.assertEQ(sample[0], 123456789n);
}

function testFloat32Array() {
    let sample = new Float32Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1.0), true);
    arktest.assertDoubleEQ(sample[0], 42.0, 0.001);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1.0), true);
    arktest.assertDoubleEQ(sample[0], 42.0, 0.001);

    arktest.assertEQ(Reflect.set(sample, '0', '3.14'), true);
    arktest.assertDoubleEQ(sample[0], 3.14, 0.001);

    arktest.assertEQ(Reflect.set(sample, '0', 2.718), true);
    arktest.assertDoubleEQ(sample[0], 2.718, 0.001);

    arktest.assertEQ(Reflect.set(sample, '0', '123.456'), true);
    arktest.assertDoubleEQ(sample[0], 123.456, 0.001);

    arktest.assertEQ(Reflect.set(sample, '0', '123'), true);
    arktest.assertDoubleEQ(sample[0], 123, 0.001);
}

function testFloat64Array() {
    let sample = new Float64Array([42]);
    arktest.assertEQ(Reflect.set(sample, '1.1', 1.0), true);
    arktest.assertDoubleEQ(sample[0], 42.0, 0.00001);

    arktest.assertEQ(Reflect.set(sample, '0.0001', 1.0), true);
    arktest.assertDoubleEQ(sample[0], 42.0, 0.00001);

    arktest.assertEQ(Reflect.set(sample, '0', '3.14159'), true);
    arktest.assertDoubleEQ(sample[0], 3.14159, 0.00001);

    arktest.assertEQ(Reflect.set(sample, '0', 2.71828), true);
    arktest.assertDoubleEQ(sample[0], 2.71828, 0.00001);

    arktest.assertEQ(Reflect.set(sample, '0', '123'), true);
    arktest.assertDoubleEQ(sample[0], 123, 0.00001);
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("TypedArray ReflectSet Suite");
    myTestsuite.addTest("testInt8Array", testInt8Array);
    myTestsuite.addTest("testInt16Array", testInt16Array);
    myTestsuite.addTest("testInt32Array", testInt32Array);
    myTestsuite.addTest("testBigInt64Array", testBigInt64Array);
    myTestsuite.addTest("testUint8Array", testUint8Array);
    myTestsuite.addTest("testUint16Array", testUint16Array);
    myTestsuite.addTest("testUint32Array", testUint32Array);
    myTestsuite.addTest("testBigUint64Array", testBigUint64Array);
    myTestsuite.addTest("testFloat32Array", testFloat32Array);
    myTestsuite.addTest("testFloat64Array", testFloat64Array);
    myTestsuite.run();
}
