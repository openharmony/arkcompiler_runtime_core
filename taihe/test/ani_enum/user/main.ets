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
import {BusinessError} from "@ohos.base";
import * as EnumTest from "enum_test";

loadLibrary("ani_enum");

function test_nextEnum_with_color() {
    const color = EnumTest.Color.GREEN;
    const nextColor = EnumTest.nextEnum(color);
    assertEQ(nextColor, EnumTest.Color.BLUE)
}

function test_getValueOfEnum() {
    let color = EnumTest.Color.GREEN;
    let value = EnumTest.getValueOfEnum(color);
    assertEQ(value, "Green");
}

function test_getValueOfEnumWeekday() {
    let day = EnumTest.Weekday.WEDNESDAY;
    let value = EnumTest.getValueOfEnumWeekday(day);
    assertEQ(value, 3);
}

function test_nextEnumWeekday() {
    let day = EnumTest.Weekday.WEDNESDAY;
    let nextDay = EnumTest.nextEnumWeekday(day);
    assertEQ(nextDay, EnumTest.Weekday.THURSDAY);
}

function test_nextEnum() {
    const color = EnumTest.Color.BLUE;
    const nextColor = EnumTest.nextEnum(color);
    assertEQ(nextColor, EnumTest.Color.RED);
}

function test_nextEnum_with_i8() {
    const first = EnumTest.NumTypeI8.ONE;
    const next = EnumTest.nextEnumI8(first);
    assertEQ(next, EnumTest.NumTypeI8.TWO)
}

function test_nextEnum_with_i16() {
    const first = EnumTest.NumTypeI16.ONE;
    const next = EnumTest.nextEnumI16(first);
    assertEQ(next, EnumTest.NumTypeI16.TWO)
}

function test_nextEnum_with_i32() {
    const first = EnumTest.NumTypeI32.ONE;
    const next = EnumTest.nextEnumI32(first);
    assertEQ(next, EnumTest.NumTypeI32.TWO)
}

function test_nextEnum_with_i64() {
    const first = EnumTest.NumTypeI64.ONE;
    const next = EnumTest.nextEnumI64(first);
    assertEQ(next, EnumTest.NumTypeI64.TWO)
}

function test_nextEnum_with_string() {
    const first = EnumTest.EnumString.ONE;
    const next = EnumTest.nextEnumString(first);
    assertEQ(next, EnumTest.EnumString.TWO)
}

function test_nextEnum_with_const_i8() {
    const first = EnumTest.FLAG_I8_TWO;
    assertEQ(first, 0 as byte)
}

function test_nextEnum_with_const_i16() {
    const first = EnumTest.FLAG_I16_TWO;
    assertEQ(first, 0 as short)
}

function test_nextEnum_with_const_i32() {
    const first = EnumTest.FLAG_I32_TWO;
    assertEQ(first, 0)
}

function test_nextEnum_with_const_i64() {
    const first = EnumTest.FLAG_I64_TWO;
    assertEQ(first, 0 as long)
}

function test_nextEnum_with_const_f32() {
    const first = EnumTest.FLAG_F32_A;
    assertEQ(first, 10.0 as float)
}

function test_nextEnum_with_const_f64() {
    const first = EnumTest.FLAG_F64_A;
    assertEQ(first, 1.0 as double)
}

function test_nextEnum_with_const_string() {
    const first = EnumTest.FLAG_STRING_ONE;
    assertEQ(first, "hello")
}

function test_fromValueToEnum() {
    const first = EnumTest.fromValueToEnum("Red");
    assertEQ(first, EnumTest.Color.RED);
    const second = EnumTest.fromValueToEnum("Green");
    assertEQ(second, EnumTest.Color.GREEN);
    const third = EnumTest.fromValueToEnum("Blue");
    assertEQ(third, EnumTest.Color.BLUE);
    let errCode: number = 0;
    try {
        EnumTest.fromValueToEnum("Yellow");
    } catch (e: BusinessError) {
        errCode = e.code;
    }
    assertEQ(errCode, 1);
}

function test_fromValueToEnumWeekday() {
    const first = EnumTest.fromValueToEnumWeekday(0);
    assertEQ(first, EnumTest.Weekday.SUNDAY);
    let errCode: number = 0;
    try {
        EnumTest.fromValueToEnumWeekday(8);
    } catch (e: BusinessError) {
        errCode = e.code;
    }
    assertEQ(errCode, 1);
}

function main() {
    console.log("##############start#############");
    const suite = new ArkTestsuite("EnumTest Tests");

    suite.addTest("test nextEnum with color", test_nextEnum_with_color);

    suite.addTest("test nextEnum with i8", test_nextEnum_with_i8);
    suite.addTest("test nextEnum with i16", test_nextEnum_with_i16);
    suite.addTest("test nextEnum with i32", test_nextEnum_with_i32);
    suite.addTest("test nextEnum with i64", test_nextEnum_with_i64);
    suite.addTest("test nextEnum with String", test_nextEnum_with_string);

    suite.addTest("test nextEnum with const i8", test_nextEnum_with_const_i8);
    suite.addTest("test nextEnum with const i16", test_nextEnum_with_const_i16);
    suite.addTest("test nextEnum with const i32", test_nextEnum_with_const_i32);
    suite.addTest("test nextEnum with const i64", test_nextEnum_with_const_i64);
    suite.addTest("test nextEnum with const f32", test_nextEnum_with_const_f32);
    suite.addTest("test nextEnum with const f64", test_nextEnum_with_const_f64);
    suite.addTest(
        "test nextEnum with const string", test_nextEnum_with_const_string);

    suite.addTest("test getValueOfEnum", test_getValueOfEnum);
    suite.addTest("test getValueOfEnumWeekday", test_getValueOfEnumWeekday);

    suite.addTest("test nextEnum", test_nextEnum);
    suite.addTest("test nextEnumWeekday", test_nextEnumWeekday);

    suite.addTest("test fromValueToEnum", test_fromValueToEnum);
    suite.addTest("test fromValueToEnumWeekday", test_fromValueToEnumWeekday);

    exit(suite.run());
}
