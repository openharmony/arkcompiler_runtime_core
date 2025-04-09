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
import * as bigint_test from "@generated/bigint_test";
import * as System from "std/core/System";

loadLibrary("ani_bigint");

function main() {
    console.info("bigint_test run start...");
    const suite = new ArkTestsuite("bigint_test ut")
    suite.addTest("testBigint01", testBigint01);
    suite.addTest("testBigint02", testBigint02);
    suite.addTest("testBigint03", testBigint03);
    suite.addTest("testBigint04", testBigint04);
    suite.addTest("testBigint05", testBigint05);
    suite.addTest("testBigint06", testBigint06);
    System.exit(suite.run());
    console.info("bigint_test run end...");
}

// test function Bigint01(a : number, b : BigInt) : boolean
function testBigint01() {
    let a: number = 666666;
    let b: bigint = 77777777777777777777777777n;
    let result: boolean = bigint_test.Bigint01(a, b);
    assertEQ(result, true);
}

// test function Bigint02(a: number) : number
function testBigint02() {
    let a: number = 77777777777;
    let result: number = bigint_test.Bigint02(a);
    assertEQ(result, a);
}

// test function Bigint03(a : BigInt) : BigInt
function testBigint03() {
    let a: bigint = 8888888888888888888888888888n;
    let result: bigint = bigint_test.Bigint03(a);
    assertEQ(result, a);
}

// test function Bigint04(a : BigInt) : void
function testBigint04() {
    let a: bigint = 8888888888888888888888888888n;
    bigint_test.Bigint04(a);
}

// test function Bigint05(a : number, b : BigInt) : BigInt
function testBigint05() {
    let a: number = 1;
    let b: bigint = 8888888888888888888888888888n;
    let result: bigint = bigint_test.Bigint05(a, b);
    assertEQ(result, b);
}

// test function Bigint06(a : number, b : BigInt) : number
function testBigint06() {
    let a: number = 1;
    let b: bigint = 8888888888888888888888888888n;
    let result: number = bigint_test.Bigint06(a, b);
    assertEQ(result, a);
}