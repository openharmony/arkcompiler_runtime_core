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
    assertEQ(n.toString(radix), "e9", "Number(233).toString(16) should be equal to e9");
}

function numberToStringTest2(): void {
    let n: Number = Number(233);
    let radix: Number = 16.99;
    assertEQ(n.toString(radix), "e9", "Number(233).toString(16.99) should be equal to e9");
}

function numberToStringTest3(): void {
    let n: Number = Number(42);
    assertEQ(n.toString(), "42", "Number(42).toString() should be equal to 42");
}

function numberToStringTest4(): void {
    let n: Number = Number(10);
    let radix: Number = 2;
    assertEQ(n.toString(radix), "1010", "Number(10).toString(2) should be equal to 1010");
}

function numberToStringTest5(): void {
    let n: Number = Number(255);
    let radix: Number = 16;
    assertEQ(n.toString(radix), "ff", "Number(255).toString(16) should be equal to ff");
}

function main() {
    let myTestsuite = new ArkTestsuite("myTestsuite");
    myTestsuite.addTest("NumberToStringTest1", numberToStringTest1);
    myTestsuite.addTest("NumberToStringTest2", numberToStringTest2);
    myTestsuite.addTest("NumberToStringTest3", numberToStringTest3);
    myTestsuite.addTest("NumberToStringTest4", numberToStringTest4);
    myTestsuite.addTest("NumberToStringTest5", numberToStringTest5);
    myTestsuite.run();
}
