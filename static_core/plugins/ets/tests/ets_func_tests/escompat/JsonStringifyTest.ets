/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

const RESULT_SUCCESS = 0
const RESULT_FAILURE = 1

const INT8_MAX: number = Byte.MAX_VALUE; //127
const INT8_MIN: number = Byte.MIN_VALUE; //-128

const UINT8_MAX: number = INT8_MAX * 2 + 1; //255
const UINT8_MIN: number = 0;

const INT16_MAX: number = Short.MAX_VALUE; // 32767
const INT16_MIN: number = Short.MIN_VALUE; // -32768

const UINT16_MAX: number = INT16_MAX * 2 + 1; //65535
const UINT16_MIN: number = 0;

const INT32_MAX: number = Int.MAX_VALUE;
const INT32_MIN: number = Int.MIN_VALUE;

const UINT32_MAX: number = INT32_MAX * 2 + 1; // 4294967295
const UINT32_MIN: number = 0;

const FLOAT32_MAX: number = Float.MAX_VALUE; // 3.4028235e+38
const FLOAT32_MIN: number = Float.MIN_VALUE; // 1.4e-45

const FLOAT64_MAX: number = Double.MAX_VALUE; // 1.7976931348623157e+308
const FLOAT64_MIN: number = Double.MIN_VALUE; // 5e-324

 function jsonStringifyUint8Array(): void {
    let value: Uint8Array = Uint8Array.of(UINT8_MIN - 1, UINT8_MIN, UINT8_MAX, UINT8_MAX + 1);
    assertEQ(`{"0":255,"1":0,"2":255,"3":0}`, JSON.stringify(value));
}

function jsonStringifyInt8Array(): void {
    let value: Int8Array = Int8Array.of(INT8_MAX,INT8_MIN,100,INT8_MIN + 1);
    assertEQ(`{"0":127,"1":-128,"2":100,"3":-127}`, JSON.stringify(value));
}

function jsonStringifyUint8ClampedArray(): void {
    let value: Uint8ClampedArray = Uint8ClampedArray.of(UINT8_MIN - 1, UINT8_MIN, UINT8_MAX, UINT8_MAX + 1);
    assertEQ(`{"0":0,"1":0,"2":255,"3":255}`, JSON.stringify(value));
}

function jsonStringifyInt16Array(): void {
    let value: Int16Array = Int16Array.of(INT16_MIN,INT16_MAX,12345,-12345);
    assertEQ(`{"0":-32768,"1":32767,"2":12345,"3":-12345}`, JSON.stringify(value));
}

function jsonStringifyUint16Array(): void {
    let value: Uint16Array = Uint16Array.of(UINT16_MIN,UINT16_MAX);
    assertEQ(`{"0":0,"1":65535}`, JSON.stringify(value));
}

function jsonStringifyInt32Array(): void {
    let value: Int32Array = Int32Array.of(INT32_MIN,INT32_MAX);
    assertEQ(`{"0":-2147483648,"1":2147483647}`, JSON.stringify(value));
}

function jsonStringifyUint32Array(): void {
    let uint32Array = new Uint32Array(5);
    let array = [UINT32_MIN,UINT32_MAX,INT32_MIN,INT32_MAX];
    let uint32ArrayFromArray = new Uint32Array(array);
    assertEQ(`{"0":0,"1":4294967295,"2":2147483648,"3":2147483647}`, JSON.stringify(uint32ArrayFromArray));
}

function jsonStringifyFloat32Array(): void {
    let value: Float32Array = Float32Array.of(FLOAT32_MIN,FLOAT32_MAX);
    assertEQ(`{"0":1.401298464324817e-45,"1":3.4028234663852886e+38}`, JSON.stringify(value));
}

function jsonStringifyFloat64Array(): void {
    let value: Float64Array = Float64Array.of(FLOAT64_MIN,FLOAT64_MAX);
    assertEQ(`{"0":5e-324,"1":1.7976931348623157e+308}`, JSON.stringify(value));
}

function jsonStringifyPromise(): void {
    let pro = new Promise<Object>((resolve: (value: Object) => void): void => {
        console.log("test");
    });
    assertEQ("{}", JSON.stringify(pro));
}

function jsonStringifySet(): void {
    let set: Set<string> = new Set<string>();
    set.add("foo");
    assertEQ("{}", JSON.stringify(set));
}

function main(): int {
    const suite = new ArkTestsuite("Json stringify for ArrayLike tests ")
    suite.addTest("JSON.stringify for Uint8Array", jsonStringifyUint8Array)
    suite.addTest("JSON.stringify for Int8Array", jsonStringifyInt8Array)
    suite.addTest("JSON.stringify for Uint8ClampedArray", jsonStringifyUint8ClampedArray)
    suite.addTest("JSON.stringify for Int16Array", jsonStringifyInt16Array)
    suite.addTest("JSON.stringify for Uint16Array", jsonStringifyUint16Array)
    suite.addTest("JSON.stringify for Int32Array", jsonStringifyInt32Array)
    suite.addTest("JSON.stringify for Uint32Array", jsonStringifyUint32Array)
    suite.addTest("JSON.stringify for Float32Array", jsonStringifyFloat32Array)
    suite.addTest("JSON.stringify for Float64Array", jsonStringifyFloat64Array)
    suite.addTest("JSON.stringify for Promise", jsonStringifyPromise)
    suite.addTest("JSON.stringify for Set", jsonStringifySet)

    return suite.run()
}