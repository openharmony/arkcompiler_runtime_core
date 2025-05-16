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

function check_array_for_int(data: BigInt64Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected,  "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: BigInt64Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: BigInt64Array, elem: BigInt, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: BigInt, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: BigInt64Array, elem: BigInt, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected,  "data.lastIndexOf(elem: BigInt) must return " + expected);
}

function check_array_for_int(data: BigUint64Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: BigUint64Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: BigUint64Array, elem: BigInt, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.includes(elem: BigInt, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: BigUint64Array, elem: BigInt, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.includes(elem: BigInt) must return " + expected);
}

function check_array_for_int(data: Int32Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: Int32Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: Int32Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Int32Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: number) must return " + expected);
}

function check_array_for_int(data: Uint32Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: Uint32Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: Uint32Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex?: number) must return " + expected);
}

function check_array(data: Uint32Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: number) must return " + expected);
}

function check_array_for_int(data: Int16Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: Int16Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: Int16Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Int16Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: number) must return " + expected);
}

function check_array_for_int(data: Uint16Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: Uint16Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: Uint16Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Uint16Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: number, fromIndex: number | undefined) " +
        "must return " + expected);
}

function check_array_for_int(data: Int8Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array_for_int(data: Int8Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected,
        "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array(data: Int8Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Int8Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: number) must return " + expected);
}

function check_array_for_int(data: Uint8Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array_for_int(data: Uint8Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array(data: Uint8Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Uint8Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected,  "data.lastIndexOf(elem: number) must return " + expected);
}

function check_array_for_int(data: Float32Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array_for_int(data: Float32Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected,  "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array(data: Float32Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf((elem.toFloat()).toDouble(), fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Float32Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf((elem.toFloat()).toDouble()), expected,
        "data.lastIndexOf(elem: number) must return " + expected);
}

function check_array_exactly_number(data: Float32Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, data.length - 1), expected,
        "data.lastIndexOf(elem: number, data.length - 1) must return " + expected);
}

function check_array_for_int(data: Float64Array, elem: int, expected: number, fromIndex: number): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex.toInt()), expected,
        "data.lastIndexOf(elem: int, fromIndex: int) must return " + expected);
}

function check_array_for_int(data: Float64Array, elem: int, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: int) must return " + expected);
}

function check_array(data: Float64Array, elem: number, expected: number, fromIndex: number | undefined): void {
    arktest.assertEQ(data.lastIndexOf(elem, fromIndex), expected,
        "data.lastIndexOf(elem: number, fromIndex: number | undefined) must return " + expected);
}

function check_array(data: Float64Array, elem: number, expected: number): void {
    arktest.assertEQ(data.lastIndexOf(elem), expected, "data.lastIndexOf(elem: number) must return " + expected);
}

function test_iu8_array() {
    let data = Int8Array.of(-128, -128, -42, -14, -14, 0, 16, 48, 127);
    check_array(data, NaN, -1);
    check_array(data, -100., -1);
    check_array_for_int(data, -100, -1);
    check_array(data, -128., 1);
    check_array_for_int(data, -128, 1);
    check_array(data, -42., 2);
    check_array_for_int(data, -42, 2);
    check_array(data, -14., 4);
    check_array_for_int(data, -14, 4);
    check_array(data, 0., 5);
    check_array_for_int(data, 0, 5);
    check_array(data, 1., -1);
    check_array_for_int(data, 1, -1);
    check_array(data, 16., 6);
    check_array_for_int(data, 16, 6);
    check_array(data, 48., 7);
    check_array_for_int(data, 48, 7);
    check_array(data, 127., 8);
    check_array_for_int(data, 127, 8);
    check_array(data, 128., -1);
    check_array_for_int(data, 128, 1); // 128 as byte in the array == -128

    let udata = Uint8Array.of(0, 0, 16, 48, 127, 16, 160, 255, 160);
    check_array(udata, NaN, -1);
    check_array(udata, 1., -1);
    check_array_for_int(udata, 1, -1);
    check_array(udata, 0., 1);
    check_array_for_int(udata, 0, 1);
    check_array(udata, 16., 5);
    check_array_for_int(udata, 16, 5);
    check_array(udata, 48., 3);
    check_array_for_int(udata, 48, 3);
    check_array(udata, 127., 4);
    check_array_for_int(udata, 127, 4);
    check_array(udata, 128., -1);
    check_array_for_int(udata, 128, -1);
    check_array(udata, 160., 8);
    check_array_for_int(udata, 160, 8);
    check_array(udata, 255., 7);
    check_array_for_int(udata, 255, 7);
    check_array(udata, 256., -1);
    check_array_for_int(udata, 256, 1);
}

function test_iu8_array_from_index() {
    let data = Int8Array.of(-128, -128, -42, -14, -14, 0, 16, 48, 127);
    check_array(data, NaN, -1, data.length - 1);
    check_array(data, -128., 1, data.length);
    check_array(data, -128., 1, data.length - 1);
    check_array(data, -128., 0, -data.length);
    check_array_for_int(data, -128, 0, -data.length);
    check_array(data, -128., 0, undefined);
    check_array_for_int(data, -128, 0, 0);
    check_array(data, -128., 1, 1);
    check_array_for_int(data, -128, 1, 1);
    check_array(data, -128., 1, -1);
    check_array_for_int(data, -128, 1, -1);
    check_array(data, -128., 1, -(data.length - 1));
    check_array_for_int(data, -128, 1, -(data.length - 1));
    check_array(data, -42., 2, data.length);
    check_array(data, -42., 2, data.length + 1);
    check_array(data, -42., 2, data.length - 1);
    check_array(data, -42., 2, 2);
    check_array_for_int(data, -42, 2, 2);
    check_array(data, -42., -1, undefined);
    check_array_for_int(data, -42, -1, 0);
    check_array(data, -42., -1, -data.length);
    check_array_for_int(data, -42, -1, -data.length);
    check_array(data, -42., -1, 1);
    check_array_for_int(data, -42, -1, 1);
    check_array(data, -14., -1, undefined);
    check_array(data, -14., 4, 4);
    check_array_for_int(data, -14, 4, 4);
    check_array(data, -14., 4, -1);
    check_array_for_int(data, -14, 4, -1);
    check_array(data, -14., 3, 3);
    check_array_for_int(data, -14, 3, 3);
    check_array(data, -14., -1, -data.length);
    check_array_for_int(data, -14, -1, -data.length);
    check_array(data, -14., -1, -(data.length - 2));
    check_array_for_int(data, -14, -1, -(data.length - 2));
    check_array(data, -14., 3, -(data.length - 3));
    check_array_for_int(data, -14, 3, -(data.length - 3));
    check_array(data, -14., 4, -(data.length - 4));
    check_array_for_int(data, -14, 4, -(data.length - 4));
    check_array(data, 128., -1, 0);
    check_array(data, 128., -1, -data.length);
    check_array(data, 128., -1, data.length);
    check_array(data, 128., -1, data.length - 1);
    check_array(data, 128., -1, undefined);
    check_array_for_int(data, 128, 1, -1); // 128 as byte in the array == -128
    check_array_for_int(data, 128, 1, 1);
    check_array_for_int(data, 128, 0, 0);
    check_array_for_int(data, 128, 0, -data.length);
    check_array_for_int(data, 128, 1, -1);
    check_array_for_int(data, 128, 1, 2);

    let udata = Uint8Array.of(0, 16, 48, 127, 16, 160, 255, 160);
    check_array(udata, NaN, -1, udata.length - 1);
    check_array(udata, 0., 0, undefined);
    check_array(udata, 0., 0, 0);
    check_array_for_int(udata, 0, 0, 0);
    check_array(udata, 0., 0, 1);
    check_array_for_int(udata, 0, 0, 1);
    check_array(udata, 0., 0, undefined);
    check_array(udata, 0., 0, udata.length - 1);
    check_array_for_int(udata, 0, 0, udata.length - 1);
    check_array(udata, 0, 0, -udata.length);
    check_array_for_int(udata, 0, 0, -udata.length);
    check_array(udata, 0., -1, -(udata.length + 1));
    check_array_for_int(udata, 0, -1, -(udata.length + 1));
    check_array(udata, 0., 0, -(udata.length - 1));
    check_array_for_int(udata, 0, 0, -(udata.length - 1));
    check_array(udata, 0., 0, -1);
    check_array_for_int(udata, 0, 0, -1);
    check_array(udata, 16., -1, undefined);
    check_array(udata, 16., 4, udata.length);
    check_array_for_int(udata, 16, 4, udata.length);
    check_array(udata, 16., 4, -1);
    check_array_for_int(udata, 16, 4, -1);
    check_array(udata, 16., 1, -(udata.length - 1));
    check_array_for_int(udata, 16, 1, -(udata.length - 1));
    check_array(udata, 16., 1, 1);
    check_array_for_int(udata, 16, 1, 1);
    check_array(udata, 16., 1, -(udata.length - 2));
    check_array_for_int(udata, 16, 1, -(udata.length - 2));
    check_array(udata, 160., -1, -udata.length);
    check_array_for_int(udata, 160, -1, -udata.length);
    check_array(udata, 160., 5, -(udata.length - 5));
    check_array_for_int(udata, 160, 5, -(udata.length - 5));
    check_array(udata, 160., 7, udata.length);
    check_array_for_int(udata, 160, 7, udata.length);
    check_array(udata, 160., 7, udata.length - 1);
    check_array_for_int(udata, 160, 7, udata.length - 1);
    check_array(udata, 160., 5, -(udata.length - 6));
    check_array_for_int(udata, 160, 5, -(udata.length - 6));
    check_array(udata, 160., 7, -(udata.length - 7));
    check_array_for_int(udata, 160, 7, -(udata.length - 7));
}

function test_iu16_array() {
    let data = Int16Array.of(-32768, -2731, -227, -227, 0, 128, 0, 2564, 32767, 0);
    check_array(data, NaN, -1);
    check_array(data, -32767., -1);
    check_array_for_int(data, -32767, -1);
    check_array(data, -32768., 0);
    check_array_for_int(data, -32768, 0);
    check_array(data, -2731., 1);
    check_array_for_int(data, -2731, 1);
    check_array(data, -227., 3);
    check_array_for_int(data, -227, 3);
    check_array(data, 0., 9);
    check_array_for_int(data, 0, 9);
    check_array(data, 128., 5);
    check_array_for_int(data, 128, 5);
    check_array(data, 2564., 7);
    check_array_for_int(data, 2564, 7);
    check_array(data, 32766., -1);
    check_array_for_int(data, 32766, -1);
    check_array(data, 32767., 8);
    check_array_for_int(data, 32767, 8);
    check_array(data, 32768., -1);
    check_array_for_int(data, 32768, 0);

    let udata = Uint16Array.of(0, 0, 0, 0, 128, 2564, 32767, 65535, 65535);
    check_array(udata, NaN, -1);
    check_array(udata, 0., 3);
    check_array_for_int(udata, 0, 3);
    check_array(udata, 1., -1);
    check_array_for_int(udata, 1, -1);
    check_array(udata, 128., 4);
    check_array_for_int(udata, 128, 4);
    check_array(udata, 2564., 5);
    check_array_for_int(udata, 2564, 5);
    check_array(udata, 32767., 6);
    check_array_for_int(udata, 32767, 6);
    check_array(udata, 32768., -1);
    check_array_for_int(udata, 32768, -1);
    check_array(udata, 65534., -1);
    check_array_for_int(udata, 65534, -1);
    check_array(udata, 65535., 8);
    check_array_for_int(udata, 65535, 8);
    check_array(udata, 65536., -1);
    check_array_for_int(udata, 65536, 3);
}

function test_iu32_array() {
    let data = Int32Array.of(-2147483648, -1398, 0, 0, 11184, 89478, 536872, 0, 2147483647, -2147483648)
    check_array(data, NaN, -1);
    check_array(data, -2147483648., 9);
    check_array_for_int(data, -2147483648, 9);
    check_array(data, -2147483647., -1);
    check_array_for_int(data, -2147483647, -1);
    check_array(data, -1398., 1);
    check_array_for_int(data, -1398, 1);
    check_array(data, 0., 7);
    check_array_for_int(data, 0, 7);
    check_array(data, 11184., 4);
    check_array_for_int(data, 11184, 4);
    check_array(data, 89478., 5);
    check_array_for_int(data, 89478, 5);
    check_array(data, 536872., 6);
    check_array_for_int(data, 536872, 6);
    check_array(data, 2147483646., -1);
    check_array_for_int(data, 2147483646, -1);
    check_array(data, 2147483647., 8);
    check_array_for_int(data, 2147483647, 8);
    check_array(data, 2147483648., -1);
    check_array_for_int(data, Long.toInt(2147483648), 9); // 2147483648 as int == -2147483648

    let udata = Uint32Array.of(0, 11184, 89478, 89478, 536872, 89478, 2147483647)
    check_array(udata, NaN, -1);
    check_array(udata, 0., 0);
    check_array_for_int(udata, 0, 0);
    check_array(udata, 1., -1);
    check_array_for_int(udata, 1, -1);
    check_array(udata, 11184., 1);
    check_array_for_int(udata, 11184, 1);
    check_array(udata, 89478., 5);
    check_array_for_int(udata, 89478, 5);
    check_array(udata, 536872., 4);
    check_array_for_int(udata, 536872, 4);
    check_array(udata, 2147483646., -1);
    check_array_for_int(udata, 2147483646, -1);
    check_array(udata, 2147483647., 6);
    check_array_for_int(udata, 2147483647, 6);
    check_array(udata, 2147483648., -1);
    check_array_for_int(udata, Long.toInt(2147483648), -1);
}

function test_iu64_array() {
    let data = BigInt64Array.of(-9.223e+18, -9.223e+18, -9.223e+18, -8589934588, -2147483648, 2147483647, 0, 1802338304,
        4294967294, 1802338304, 2.3058e+16, 2.3058e+16, 9.223e+18, 2147483647);
    check_array_for_int(data, Long.toInt(-10000000000), -1);
    check_array(data, BigInt(-10000000000), -1);
    check_array_for_int(data, -10000, -1);
    check_array(data, BigInt(-10000), -1);
    check_array_for_int(data, Double.toInt(-9.224e+18), 4);
    check_array(data, BigInt(-9.224e+18), -1);
    check_array_for_int(data, Double.toInt(-9.223e+18), 4);
    check_array(data, BigInt(-9.223e+18), 2);
    check_array_for_int(data, Long.toInt(-8589934588), -1);
    check_array(data, BigInt(-8589934588), 3);
    check_array_for_int(data, 0, 6);
    check_array(data, BigInt(0), 6);
    check_array_for_int(data, Long.toInt(1802338304), 9);
    check_array(data, BigInt(1802338304), 9);
    check_array_for_int(data, Long.toInt(4294967294), -1);
    check_array(data, BigInt(4294967294), 8);
    check_array_for_int(data, Double.toInt(2.3058e+16), 13);
    check_array(data, BigInt(2.3058e+16), 11);
    check_array_for_int(data, Double.toInt(9.223e+18), 13);
    check_array(data, BigInt(9.223e+18), 12);
    check_array_for_int(data, Double.toInt(9.224e+18), 13);
    check_array(data, BigInt(9.224e+18), -1);

    let udata = BigUint64Array.of(0, 4294967294, 2.3058e+16, 2.3058e+16, 9.223e+18, 2.3058e+16, 18.446e+18, 2.3058e+16)
    check_array_for_int(udata, Long.toInt(10000000000), -1);
    check_array(udata, BigInt(10000000000), -1);
    check_array_for_int(udata, 10000, -1);
    check_array(udata, BigInt(10000), -1);
    check_array_for_int(udata, 0, 0);
    check_array(udata, BigInt(0), 0);
    check_array_for_int(udata, 1, -1);
    check_array(udata, BigInt(1), -1);
    check_array_for_int(udata, Long.toInt(4294967294), -1);
    check_array(udata, BigInt(4294967294), 1);
    check_array_for_int(udata, Double.toInt(2.3058e+16), -1);
    check_array(udata, BigInt(2.3058e+16), 7);
    check_array_for_int(udata, Double.toInt(9.223e+18), -1);
    check_array(udata, BigInt(9.223e+18), 4);
    check_array_for_int(udata, Double.toInt(9.224e+18), -1);
    check_array(udata, BigInt(9.224e+18), -1);
    check_array_for_int(udata, Double.toInt(18.446e+18), -1);
    check_array(udata, BigInt(18.446e+18), 6);
}

function test_f64_array() {
    let data_no_inf = Float64Array.of(-1.7e+308, -3.6e+42, -3.6e+42, 0, NaN, -0.6e+16, 0, 0.4e+105, 0, 1.7e+308);
    check_array(data_no_inf, NaN, -1);
    check_array(data_no_inf, Infinity, -1);
    check_array(data_no_inf, -1.701e+308, -1);
    check_array_for_int(data_no_inf, Double.toInt(-1.701e+308), -1);
    check_array(data_no_inf, -1.7e+308, 0);
    check_array_for_int(data_no_inf, Double.toInt(-1.7e+308), -1);
    check_array(data_no_inf, 1.7e+308, 9);
    check_array_for_int(data_no_inf, Double.toInt(1.7e+308), -1);
    check_array(data_no_inf, -1.7e+1, -1);
    check_array_for_int(data_no_inf, Double.toInt(-1.7e+1), -1);
    check_array(data_no_inf, 0., 8);
    check_array_for_int(data_no_inf, 0, 8);
    check_array(data_no_inf, -0.6e+16, 5);
    check_array_for_int(data_no_inf, Double.toInt(-0.6e+16), -1);
    check_array(data_no_inf, 0.4e+105, 7);
    check_array_for_int(data_no_inf, Double.toInt(0.4e+105), -1);

    let data_inf = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, 0, NaN, 1024, -0.6e+16, 0.4e+105, 1.7e+308, Infinity);
    check_array(data_inf, NaN, -1);
    check_array(data_inf, Infinity, 9);
    check_array(data_inf, -1.701e+308, -1);
    check_array_for_int(data_inf, Double.toInt(-1.701e+308), -1);
    check_array(data_inf, -1.7e+308, 1);
    check_array_for_int(data_inf, Double.toInt(-1.7e+308), -1);
    check_array(data_inf, 1.7e+308, 8);
    check_array_for_int(data_inf, Double.toInt(1.7e+308), -1);
    check_array(data_inf, -1.7e+1, -1);
    check_array_for_int(data_inf, Double.toInt(-1.7e+1), -1);
    check_array(data_inf, 0., 3);
    check_array_for_int(data_inf, 0, 3);
    check_array(data_inf, 1024., 5);
    check_array_for_int(data_inf, Double.toInt(1024.5), 5);
    check_array(data_inf, -0.6e+16, 6);
    check_array_for_int(data_inf, Double.toInt(-0.6e+16), -1);
    check_array(data_inf, 0.4e+105, 7);
    check_array_for_int(data_inf, Double.toInt(0.4e+105), -1);
}

function test_f64_array_from_index() {
    let data = Float64Array.of(-1.7e+308, -3.6e+42, -3.6e+42, 0, NaN, -0.6e+16, 0.4e+105, -3.6e+42, 1.7e+308, 1.7e+308);
    check_array(data, NaN, -1, undefined);
    check_array(data, NaN, -1, 0);
    check_array(data, NaN, -1, data.length - 1);
    check_array(data, Infinity, -1, undefined);
    check_array(data, Infinity, -1, 0);
    check_array(data, Infinity, -1, data.length - 1);
    check_array(data, -1.7e+308, 0, undefined);
    check_array(data, -1.7e+308, 0, 0);
    check_array(data, -1.7e+308, 0, 1);
    check_array(data, -1.7e+308, 0, -data.length);
    check_array(data, -1.7e+308, -1, -(data.length + 1));
    check_array(data, -1.7e+308, 0, -(data.length - 1));
    check_array(data, -1.7e+308, 0, -1);
    check_array(data, 1.7e+308, -1, undefined);
    check_array(data, 1.7e+308, -1, 0);
    check_array(data, 1.7e+308, (data.length - 2), -2);
    check_array(data, 1.7e+308, (data.length - 1), -1);
    check_array(data, 1.7e+308, -1, -data.length);
    check_array(data, 1.7e+308, (data.length - 1), data.length);
    check_array(data, 1.7e+308, (data.length - 2), data.length - 2);
    check_array(data, 1.7e+308, (data.length - 1), data.length - 1);
    check_array(data, 1.7e+308, -1, -(data.length + 1));
    check_array(data, 1.7e+308, -1, -(data.length - 1));
    check_array(data, 1.7e+308, (data.length - 1), -1);
    check_array(data, -3.6e+42, -1, undefined);
    check_array(data, -3.6e+42, -1, 0);
    check_array(data, -3.6e+42, 1, 1);
    check_array(data, -3.6e+42, 2, 2);
    check_array(data, -3.6e+42, 2, 3);
    check_array(data, -3.6e+42, 7, 7);
    check_array(data, -3.6e+42, 7, 8);
    check_array(data, -3.6e+42, -1, -(data.length + 1));
    check_array(data, -3.6e+42, -1, -data.length);
    check_array(data, -3.6e+42, 1, -(data.length - 1));
    check_array(data, -3.6e+42, 2, -(data.length - 2));
    check_array(data, -3.6e+42, (data.length - 3), -2);
    check_array(data, -3.6e+42, (data.length - 3), -3);
    check_array(data, -3.6e+42, (data.length - 3), data.length);
    check_array(data, 0., -1, undefined);
    check_array(data, 0., -1, 0);
    check_array(data, 0., 3, 3);
    check_array_for_int(data, 0, 3, 3);
    check_array(data, 0., 3, 4);
    check_array_for_int(data, 0, 3, 4);
    check_array(data, 0., 3, -1);
    check_array_for_int(data, 0, 3, -1);
    check_array(data, 0., -1, -data.length);
    check_array_for_int(data, 0, -1, -data.length);
    check_array(data, 0., -1, -(data.length + 3));
    check_array_for_int(data, 0, -1, -(data.length + 3));
    check_array(data, 0., 3, -(data.length - 4));
    check_array_for_int(data, 0, 3, -(data.length - 4));
    check_array(data, 0., 3, -(data.length - 5));
    check_array_for_int(data, 0, 3, -(data.length - 5));
    check_array(data, 0., 3, -(data.length - 3));
    check_array_for_int(data, 0, 3, -(data.length - 3));
    check_array(data, 0., -1, -(data.length - 2));
    check_array_for_int(data, 0, -1, -(data.length - 2));
    check_array(data, 0., -1, 2);
    check_array_for_int(data, 0, -1, 2);
    check_array(data, 0., -1, 2);
    check_array_for_int(data, 0, -1, 2);
    check_array(data, 0., 3, data.length);
    check_array_for_int(data, 0, 3, data.length);
    check_array(data, 0., 3, data.length -1);
    check_array_for_int(data, 0, 3, data.length - 1);
    check_array(data, -0.6e+16, -1, 0);
    check_array(data, -0.6e+16, -1, undefined);
    check_array(data, -0.6e+16, 5, 5);
    check_array(data, -0.6e+16, 5, 6);
    check_array(data, -0.6e+16, -1, 4);
    check_array(data, -0.6e+16, 5, -4);
    check_array(data, -0.6e+16, 5, -5);
    check_array(data, -0.6e+16, -1, -6);
}

function test_f32_array() {
    let data = Float32Array.of(Infinity, -3.4e+38, -3.4e+38, -2.02e+16, 1, 1, 1, 1.5, -1, -3, -3.4e+38, -1.0031,
        -.1031, -1.061e+09, 1.0601e+09, 1.060001e+09, Infinity, 2.8e+24, Infinity, 2.8e+24, 3.4e+38, -1, Infinity);
    check_array(data, NaN, -1);
    check_array(data, Infinity, 22);
    check_array_exactly_number(data, Infinity, 22);
    check_array(data, -3.4e+38, 10);
    check_array_exactly_number(data, -3.4e+38, -1);
    check_array(data, (-3.4e+38 + 1e32), -1);
    check_array(data, -2.02e+16, 3);
    check_array_for_int(data, Double.toInt(-2.02e+16), -1);
    check_array_exactly_number(data, -2.02e+16, -1);
    check_array(data, -2.020101e+16, -1);
    check_array(data, -1.061e+09, 13);
    check_array_exactly_number(data, -1.061e+09, 13); // this is exactly c1cf9ec9a0000000 as well as the float
    check_array(data, -1.0031, 11);
    check_array_for_int(data, Double.toInt(-1.0031), 21); // this is looking for -1
    check_array_exactly_number(data, -1.0031, -1);
    check_array(data, -0.1031, 12);
    check_array_exactly_number(data, -0.1031, -1);
    check_array_for_int(data, Double.toInt(-0.0031), -1); // there is no zero in the array
    check_array(data, 1., 6);
    check_array_exactly_number(data, 1., 6);
    check_array_for_int(data, 1, 6);
    check_array(data, -1., 21);
    check_array_exactly_number(data, -1., 21);
    check_array_for_int(data, -1, 21);
    check_array(data, 1 + 1.19209290e-07, -1);
    check_array(data, 1.0601e+09, 14);
    check_array_exactly_number(data, 1.0601e+09, -1);
    check_array(data, 1.060001e+09, 15);
    check_array_exactly_number(data, 1.060001e+09, -1);
    check_array(data, 2.8e+24, 19);
    check_array_exactly_number(data, 2.8e+24, -1);
    check_array(data, 3.4e+38, 20);
    check_array_exactly_number(data, 3.4e+38, -1);
    check_array(data, (3.4e+38 - 1.175e-38), 20);
    check_array_exactly_number(data, (3.4e+38 - 1.175e-38), -1);
    check_array(data, (3.4e+38 - 1e32), -1);
}

function test_f32_array_from_index() {
    let data = Float32Array.of(Infinity, -3.4e+38, -3.4e+38, -2.02e+16, 1, 1, 1, 1.5, -1, -3, -3.4e+38, -1.0031,
        -.1031, -1.061e+09, 1.0601e+09, 1.060001e+09, Infinity, 2.8e+24, Infinity, 2.8e+24, 3.4e+38, Infinity);
    check_array(data, Infinity, 0, undefined);
    check_array(data, Infinity, 0, 0);
    check_array(data, Infinity, 0, 1);
    check_array(data, Infinity, 21, -1);
    check_array(data, Infinity, 18, -2);
    check_array(data, Infinity, 18, -4);
    check_array(data, Infinity, 16, -6);
    check_array(data, Infinity, 0, -data.length);
    check_array(data, Infinity, -1, -(data.length + 1));
    check_array(data, Infinity, 0, -(data.length - 1));
    check_array(data, 1, -1, undefined);
    check_array(data, 1, -1, 0);
    check_array_for_int(data, 1, -1, 0);
    check_array(data, 1, -1, 3);
    check_array_for_int(data, 1, -1, 3);
    check_array(data, 1, 4, 4);
    check_array_for_int(data, 1, 4, 4);
    check_array(data, 1, 5, 5);
    check_array_for_int(data, 1, 5, 5);
    check_array(data, 1, 6, 6);
    check_array_for_int(data, 1, 6, 6);
    check_array(data, 1, 6, 7);
    check_array_for_int(data, 1, 6, 7);
    check_array(data, 1, 4, -(data.length - 4));
    check_array_for_int(data, 1, 4, -(data.length - 4));
    check_array(data, 1, 5, -(data.length - 5));
    check_array_for_int(data, 1, 5, -(data.length - 5));
    check_array(data, 1, -1, -(data.length - 3));
    check_array_for_int(data, 1, -1, -(data.length - 3));
    check_array(data, -1.0031, -1, undefined);
    check_array(data, -1.0031, -1, 0);
    check_array(data, -1.0031, -1, 9);
    check_array(data, -1.0031, 11, 11);
    check_array(data, -1.0031, 11, 12);
    check_array_for_int(data, Double.toInt(-1.0031), -1, 0); // this is looking for -1
    check_array_for_int(data, Double.toInt(-1.0031), 8, 8);
    check_array_for_int(data, Double.toInt(-1.0031), 8, 9);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("typedArray.lastIndexOf");
    testSuite.addTest("Int8,UInt8 Arrays: lastIndexOf for arrays", test_iu8_array);
    testSuite.addTest("Int8,UInt8 Arrays: lastIndexOf for arrays", test_iu8_array_from_index);
    testSuite.addTest("Int16,UInt16 Arrays: lastIndexOf for arrays", test_iu16_array);
    testSuite.addTest("Int32,UInt32 Arrays: lastIndexOf for arrays", test_iu32_array);
    testSuite.addTest("BigInt64,BigUint64 Arrays: lastIndexOf for arrays", test_iu64_array);
    testSuite.addTest("Float32 Arrays: lastIndexOf for arrays", test_f32_array);
    testSuite.addTest("Float32 Arrays: lastIndexOf for arrays with fromIndex", test_f32_array_from_index);
    testSuite.addTest("Float64 Arrays: lastIndexOf for arrays", test_f64_array);
    testSuite.addTest("Float64 Arrays: lastIndexOf for arrays with fromIndex", test_f64_array_from_index);
    return testSuite.run();
}
