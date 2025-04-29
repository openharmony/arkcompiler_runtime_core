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
import * as bar from "@generated/bar";
import {BusinessError} from "@ohos.base";
import * as System from "std/core/System";

loadLibrary("ani_typedarray");

function checkUint8Array() {
    let arr = bar.newUint8Array(2, 0x01 as byte);
    assertEQ(arr.length, 2);
    assertEQ(arr.at(0), 0x01);
    assertEQ(arr.at(1), 0x01);
    let res = bar.sumUint8Array(arr);
    assertEQ(res, 0x01 * 2);
}

function checkUint16Array() {
    let arr = bar.newUint16Array(2, 0x0123 as short);
    assertEQ(arr.at(0), 0x0123);
    assertEQ(arr.at(1), 0x0123);
    let res = bar.sumUint16Array(arr);
    assertEQ(res, 0x0123 * 2);
}

function checkUint32Array() {
    let arr = bar.newUint32Array(2, 0x01234567 as int);
    assertEQ(arr.at(0), 0x01234567);
    assertEQ(arr.at(1), 0x01234567);
    let res = bar.sumUint32Array(arr);
    assertEQ(res, 0x01234567 * 2);
}

function checkBigUint64Array() {
    let arr = bar.newBigUint64Array(2, 21 as long);
    assertEQ(arr.at(0), 21n);
    assertEQ(arr.at(1), 21n);
    let res = bar.sumBigUint64Array(arr);
    assertEQ(res, 42);
}

function checkInt8Array() {
    let arr = bar.newInt8Array(2, 0x01 as byte);
    assertEQ(arr.at(0), 0x01);
    assertEQ(arr.at(1), 0x01);
    let res = bar.sumInt8Array(arr);
    assertEQ(res, 0x01 * 2);
}

function checkInt16Array() {
    let arr = bar.newInt16Array(2, 0x0123 as short);
    assertEQ(arr.at(0), 0x0123);
    assertEQ(arr.at(1), 0x0123);
    let res = bar.sumInt16Array(arr);
    assertEQ(res, 0x0123 * 2);
}

function checkInt32Array() {
    let arr = bar.newInt32Array(2, 0x01234567 as int);
    assertEQ(arr.at(0), 0x01234567);
    assertEQ(arr.at(1), 0x01234567);
    let res = bar.sumInt32Array(arr);
    assertEQ(res, 0x01234567 * 2);
}

function checkBigInt64Array() {
    let arr = bar.newBigInt64Array(2, 21 as long);
    assertEQ(arr.at(0), 21n);
    assertEQ(arr.at(1), 21n);
    let res = bar.sumBigInt64Array(arr);
    assertEQ(res, 42);
}

function checkFloat32Array() {
    let arr = bar.newFloat32Array(2, 0.1 as float);
    let tolerance = 0.00001 as float;
    assertEQ(Math.abs(arr.at(0)! - 0.1) < tolerance, true);
    assertEQ(Math.abs(arr.at(1)! - 0.1) < tolerance, true);
    let res = bar.sumFloat32Array(arr);
    assertEQ(Math.abs(res - 0.1 * 2) < tolerance, true);
}

function checkFloat64Array() {
    let arr = bar.newFloat64Array(2, 0.1 as double);
    let tolerance = 0.00001 as double;
    assertEQ(Math.abs(arr.at(0)! - 0.1) < tolerance, true);
    assertEQ(Math.abs(arr.at(1)! - 0.1) < tolerance, true);
    let res = bar.sumFloat64Array(arr);
    assertEQ(Math.abs(res - 0.1 * 2) < tolerance, true);
}

function main() {
    const suite = new ArkTestsuite("ani_typedarray ut")
    suite.addTest("checkUint8Array", checkUint8Array);
    suite.addTest("checkUint16Array", checkUint16Array);
    suite.addTest("checkUint32Array", checkUint32Array);
    suite.addTest("checkBigUint64Array", checkBigUint64Array);
    suite.addTest("checkInt8Array", checkInt8Array);
    suite.addTest("checkInt16Array", checkInt16Array);
    suite.addTest("checkInt32Array", checkInt32Array);
    suite.addTest("checkBigInt64Array", checkBigInt64Array);
    suite.addTest("checkFloat32Array", checkFloat32Array);
    suite.addTest("checkFloat64Array", checkFloat64Array);
    System.exit(suite.run());
}
