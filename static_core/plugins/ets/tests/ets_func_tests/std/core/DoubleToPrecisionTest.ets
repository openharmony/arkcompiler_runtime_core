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

function main() {
    let myTestsuite = new ArkTestsuite('myTestsuite');
    myTestsuite.addTest('testDecimal', testDecimal);
    myTestsuite.addTest('testFullDigit', testFullDigit);
    myTestsuite.addTest('testScientific', testScientific);
    myTestsuite.run();
}

function testDecimal() {
    let num1: Double = 0.1;
    let num2: Double = 0.01;
    let num3: Double = 0.001;
    let num4: Double = 0.0001;
    let num5: Double = 0.00001;
    let num6: Double = 0.000001;
    let num7: Double = 0.0000001;
    let num8: Double = 0.00000001;

    const str1 = num1.toPrecision(1);
    const str2 = num2.toPrecision(2);
    const str3 = num3.toPrecision(3);
    const str4 = num4.toPrecision(4);
    const str5 = num5.toPrecision(5);
    const str6 = num6.toPrecision(6);
    const str7 = num7.toPrecision(7);
    const str8 = num8.toPrecision(8);

    assertEQ(str1, '0.1');
    assertEQ(str2, '0.010');
    assertEQ(str3, '0.00100');
    assertEQ(str4, '0.0001000');
    assertEQ(str5, '0.000010000');
    assertEQ(str6, '0.00000100000');
    assertEQ(str7, '1.000000e-7');
    assertEQ(str8, '1.0000000e-8');
}

function testFullDigit() {
    let num1: Double = 1;
    let num2: Double = 12;
    let num3: Double = 123;
    let num4: Double = 1234;
    let num5: Double = 12345;
    let num6: Double = 123456;
    let num7: Double = 1234567;
    let num8: Double = 12345678;

    const str1 = num1.toPrecision(1);
    const str2 = num2.toPrecision(2);
    const str3 = num3.toPrecision(3);
    const str4 = num4.toPrecision(4);
    const str5 = num5.toPrecision(5);
    const str6 = num6.toPrecision(6);
    const str7 = num7.toPrecision(7);
    const str8 = num8.toPrecision(8);

    assertEQ(str1, '1');
    assertEQ(str2, '12');
    assertEQ(str3, '123');
    assertEQ(str4, '1234');
    assertEQ(str5, '12345');
    assertEQ(str6, '123456');
    assertEQ(str7, '1234567');
    assertEQ(str8, '12345678');
}

function testScientific() {
    let num: Double = 1234567890;

    const str1 = num.toPrecision(1);
    const str2 = num.toPrecision(2);
    const str3 = num.toPrecision(3);
    const str4 = num.toPrecision(4);
    const str5 = num.toPrecision(5);
    const str6 = num.toPrecision(6);
    const str7 = num.toPrecision(7);
    const str8 = num.toPrecision(8);
    const str9 = num.toPrecision(9);
    const str10 = num.toPrecision(10);
    const str11 = num.toPrecision(11);
    const str12 = num.toPrecision(12);

    assertEQ(str1, '1e+9');
    assertEQ(str2, '1.2e+9');
    assertEQ(str3, '1.23e+9');
    assertEQ(str4, '1.235e+9');
    assertEQ(str5, '1.2346e+9');
    assertEQ(str6, '1.23457e+9');
    assertEQ(str7, '1.234568e+9');
    assertEQ(str8, '1.2345679e+9');
    assertEQ(str9, '1.23456789e+9');
    assertEQ(str10, '1234567890');
    assertEQ(str11, '1234567890.0');
    assertEQ(str12, '1234567890.00');
}

