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

function numberToStringTest1(): void {
    let n: Number = Number(233);
    let radix: Number = 16;
    arktest.assertEQ(n.toString(radix), "e9", "Number(233).toString(16) should be equal to e9");
}

function numberToStringTest2(): void {
    let n: Number = Number(233);
    let radix: Number = 16.99;
    arktest.assertEQ(n.toString(radix), "e9", "Number(233).toString(16.99) should be equal to e9");
}

function numberToStringTest3(): void {
    let n: Number = Number(42);
    arktest.assertEQ(n.toString(), "42", "Number(42).toString() should be equal to 42");
}

function numberToStringTest4(): void {
    let n: Number = Number(10);
    let radix: Number = 2;
    arktest.assertEQ(n.toString(radix), "1010", "Number(10).toString(2) should be equal to 1010");
}

function numberToStringTest5(): void {
    let n: Number = Number(255);
    let radix: Number = 16;
    arktest.assertEQ(n.toString(radix), "ff", "Number(255).toString(16) should be equal to ff");
}

function testNumberToExponential(): void {
    let n: number | undefined = 1
    let m: number = 1
    arktest.assertEQ(m.toExponential(n), "1.0e+0")
}

function testNumberToExponentialUndefined(): void {
    let m: number = 1
    arktest.assertEQ(m.toExponential(undefined), "1e+0")
}

function testNumberForParseFloat1(): void {
    let num1: double = Number.parseFloat('1e-9000000000000000')
    let num2: double = Number.parseFloat('-1e-9000000000000000')
    arktest.assertDoubleEQ(num1, 0.0, 0.000001)
    arktest.assertDoubleEQ(num2, 0.0, 0.000001)
}

function testNumberForParseFloat2(): void {
    let num1: double = Number.parseFloat('1e-100000')
    let num2: double = Number.parseFloat('-1e-100000')
    arktest.assertDoubleEQ(num1, 0.0, 0.000001)
    arktest.assertDoubleEQ(num2, 0.0, 0.000001)
}

function testNumberForParseFloat3(): void {
    let num1: double = Number.parseFloat('1e-5')
    let num2: double = Number.parseFloat('-1e-5')
    arktest.assertDoubleEQ(num1, 0.00001, 0.000001)
    arktest.assertDoubleEQ(num2, -0.00001, 0.000001)
}

function testNumberForParseFloat4(): void {
    let num1: double = Number.parseFloat('1e-0000000')
    let num2: double = Number.parseFloat('-1e-0000000')
    arktest.assertDoubleEQ(num1, 1.0, 0.000001)
    arktest.assertDoubleEQ(num2, -1.0, 0.000001)
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
    myTestsuite.addTest("NumberToStringTest1", numberToStringTest1);
    myTestsuite.addTest("NumberToStringTest2", numberToStringTest2);
    myTestsuite.addTest("NumberToStringTest3", numberToStringTest3);
    myTestsuite.addTest("NumberToStringTest4", numberToStringTest4);
    myTestsuite.addTest("NumberToStringTest5", numberToStringTest5);
    myTestsuite.addTest('testNumberToExponential', testNumberToExponential);
    myTestsuite.addTest('testNumberToExponentialUndefined', testNumberToExponentialUndefined);
    myTestsuite.addTest('testNumberForParseFloat exceeding int16_t', testNumberForParseFloat1);
    myTestsuite.addTest('testNumberForParseFloat not exceeding int16_t', testNumberForParseFloat2);
    myTestsuite.addTest('testNumberForParseFloat normal number', testNumberForParseFloat3);
    myTestsuite.addTest('testNumberForParseFloat special', testNumberForParseFloat4);
    myTestsuite.run();
}
