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

function check_array(data: Float64Array, elem: number, expected: boolean, fromIndex?: number): void {
    arktest.assertEQ(data.includes(elem, fromIndex), expected,
        "data.includes(elem: number, fromIndex?: number) must return " + expected);
}

function check_array(data: Float32Array, elem: number, expected: boolean, fromIndex?: number): void {
    arktest.assertEQ(data.includes((elem.toFloat()).toDouble(), fromIndex), expected,
        "data.includes(elem: number, fromIndex?: number) must return " + expected);
}

function check_array_for_float(data: Float32Array, elem: float): void {
    arktest.assertEQ(data.includes(elem), data.indexOf(elem.toInt()) > 0, "data.includes(elem: float) must check " +
        "whether data.indexOf(elem.toInt()) > -1, so whether the truncated float exists within the array.");
}

function test_f64_array_noNaN() {
    let data_no_inf = Float64Array.of(-1.7e+308, -3.6e+42, 0, -0.6e+16, 0.4e+105, 1.7e+308);
    check_array(data_no_inf, Infinity, false);
    check_array(data_no_inf, -1.701e+308, false);
    check_array(data_no_inf, -1.7e+308, true);
    check_array(data_no_inf, 1.7e+308, true);
    check_array(data_no_inf, -1.7e+1, false);
    check_array(data_no_inf, 0, true);
}

function test_f64_array_noNaN_from_index() {
    let data_no_inf = Float64Array.of(-1.7e+308, -3.6e+42, 0, -0.6e+16, 0.4e+105, 1.7e+308);
    check_array(data_no_inf, -1.7e+308, true);
    check_array(data_no_inf, -1.7e+308, true, 0);
    check_array(data_no_inf, -1.7e+308, false, 1);
    check_array(data_no_inf, -1.7e+308, true, -data_no_inf.length);
    check_array(data_no_inf, -1.7e+308, false, -(data_no_inf.length - 1));
    check_array(data_no_inf, -1.7e+308, false, -1);
    check_array(data_no_inf, -1.7e+308, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, 1.7e+308, true);
    check_array(data_no_inf, 1.7e+308, true, 0);
    check_array(data_no_inf, 1.7e+308, true, -1);
    check_array(data_no_inf, 1.7e+308, true, -data_no_inf.length);
    check_array(data_no_inf, 1.7e+308, false, data_no_inf.length);
    check_array(data_no_inf, 1.7e+308, true, data_no_inf.length - 1);
    check_array(data_no_inf, 1.7e+308, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, 1.7e+308, true, -(data_no_inf.length - 1));
    check_array(data_no_inf, -3.6e+42, true);
    check_array(data_no_inf, -3.6e+42, true, 1);
    check_array(data_no_inf, -3.6e+42, false, 2);
    check_array(data_no_inf, -3.6e+42, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, -3.6e+42, true, -(data_no_inf.length - 1));
    check_array(data_no_inf, -3.6e+42, false, -(data_no_inf.length - 2));
    check_array(data_no_inf, -3.6e+42, true, -data_no_inf.length);
    check_array(data_no_inf, -3.6e+42, false, data_no_inf.length);
    check_array(data_no_inf, 0, true);
    check_array(data_no_inf, 0, true, 2);
    check_array(data_no_inf, 0, false, 3);
    check_array(data_no_inf, 0, false, 4);
    check_array(data_no_inf, 0, false, data_no_inf.length);
    check_array(data_no_inf, 0, true, -data_no_inf.length);
    check_array(data_no_inf, 0, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, 0, true, -(data_no_inf.length - 2));
    check_array(data_no_inf, 0, false, -(data_no_inf.length - 3));
}

function test_f32_array_noNaN() {
    let data_no_inf = Float32Array.of(-3.4e+38, -2.02e+16, 1, 1.5, -1, -3, -1.0031, -.1031, -1.061e+09, 1.0601e+09,
        1.060001e+09, 2.8e+24, 3.4e+38);
    check_array(data_no_inf, Infinity, false);
    check_array(data_no_inf, -3.4e+38, true);
    check_array_for_float(data_no_inf, -3.4e+38f); // -3 exists, indexOf(-3) returns smth > -1
    check_array(data_no_inf, (-3.4e+38 + 1e32), false);
    check_array_for_float(data_no_inf, (-3.4e+38f + 1e32f));
    check_array(data_no_inf, -2.02e+16, true);
    check_array_for_float(data_no_inf, -2.02e+16f); // -2 doesn't exist, indexOf(-2) returns -1
    check_array(data_no_inf, -2.020101e+16, false);
    check_array_for_float(data_no_inf, -2.020101e+16f);
    check_array(data_no_inf, -1.061e+09, true);
    check_array(data_no_inf, -1.0031, true);
    check_array_for_float(data_no_inf, -1.0031f);
    check_array(data_no_inf, -0.1031, true);
    check_array_for_float(data_no_inf, -0.1031f);
    check_array(data_no_inf, -1.061e+09, true);
    check_array_for_float(data_no_inf, -1.061e+09f);
    check_array(data_no_inf, 1., true);
    check_array_for_float(data_no_inf, 1.0f);
    check_array(data_no_inf, 1 + 1.19209290e-07, false);
    check_array_for_float(data_no_inf, 1f + 1.19209290e-07f);
    check_array(data_no_inf, 1.0601e+09, true);
    check_array_for_float(data_no_inf, 1.0601e+09f);
    check_array(data_no_inf, 1.060001e+09, true);
    check_array_for_float(data_no_inf, 1.060001e+09f);
    check_array(data_no_inf, 2.8e+24, true);
    check_array_for_float(data_no_inf, 2.8e+24f);
    check_array(data_no_inf, 3.4e+38, true);
    check_array_for_float(data_no_inf, 3.4e+38f);
    check_array(data_no_inf, (3.4e+38 - 1.175e-38), true);
    check_array_for_float(data_no_inf, (3.4e+38f - 1.175e-38f));
    check_array(data_no_inf, (3.4e+38 - 1e32), false);
    check_array_for_float(data_no_inf, (3.4e+38f - 1e32f));
}
function test_f32_array_noNaN_from_index() {
    let data_no_inf = Float32Array.of(-3.4e+38, -2.02e+16, 1, 1.5, -1, -3, -1.0031, -.1031, -1.061e+09, 1.0601e+09,
        1.060001e+09, 2.8e+24, 3.4e+38);
    check_array(data_no_inf, Infinity, false);
    check_array(data_no_inf, -3.4e+38, true);
    check_array(data_no_inf, -3.4e+38, true, 0);
    check_array(data_no_inf, -3.4e+38, false, 1);
    check_array(data_no_inf, -3.4e+38, true, -data_no_inf.length);
    check_array(data_no_inf, -3.4e+38, false, -(data_no_inf.length - 1));
    check_array(data_no_inf, -3.4e+38, false, -1);
    check_array(data_no_inf, -3.4e+38, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, -2.02e+16, true);
    check_array(data_no_inf, -2.02e+16, true, 1);
    check_array(data_no_inf, -2.02e+16, false, 2);
    check_array(data_no_inf, -2.02e+16, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, -2.02e+16, true, -(data_no_inf.length - 1));
    check_array(data_no_inf, -2.02e+16, false, -(data_no_inf.length - 2));
    check_array(data_no_inf, -2.02e+16, true, -data_no_inf.length);
    check_array(data_no_inf, -2.02e+16, false, data_no_inf.length);
    check_array(data_no_inf, -2.020101e+16, false);
    check_array(data_no_inf, -1.061e+09, true);
    check_array(data_no_inf, -1.0031, true);
    check_array(data_no_inf, -0.1031, true);
    check_array(data_no_inf, -1.061e+09, true);
    check_array(data_no_inf, 1., true);
    check_array(data_no_inf, 1, true, 2);
    check_array(data_no_inf, 1, false, 3);
    check_array(data_no_inf, 1, false, 4);
    check_array(data_no_inf, 1, false, data_no_inf.length);
    check_array(data_no_inf, 1, true, -data_no_inf.length);
    check_array(data_no_inf, 1, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, 1, true, -(data_no_inf.length - 2));
    check_array(data_no_inf, 1, false, -(data_no_inf.length - 3));
    check_array(data_no_inf, 1 + 1.19209290e-08, true);
    check_array(data_no_inf, 1 + 1.19209290e-07, false);
    check_array(data_no_inf, 1.0601e+09, true);
    check_array(data_no_inf, 1.060001e+09, true);
    check_array(data_no_inf, 2.8e+24, true);
    check_array(data_no_inf, 3.4e+38, true);
    check_array(data_no_inf, 3.4e+38, true);
    check_array(data_no_inf, 3.4e+38, true, 0);
    check_array(data_no_inf, 3.4e+38, true, -1);
    check_array(data_no_inf, 3.4e+38, true, -data_no_inf.length);
    check_array(data_no_inf, 3.4e+38, false, data_no_inf.length);
    check_array(data_no_inf, 3.4e+38, true, data_no_inf.length - 1);
    check_array(data_no_inf, 3.4e+38, true, -(data_no_inf.length + 1));
    check_array(data_no_inf, 3.4e+38, true, -(data_no_inf.length - 1));
    check_array(data_no_inf, (3.4e+38 - 1.175e-38), true);
    check_array(data_no_inf, (3.4e+38 - 1e32), false);
}

function test_f64_array_inf() {
    const check_every_element = (arr: Float64Array) => {
        check_array(arr, -1.7e+308, true);
        check_array(arr, -3.6e+42, true);
        check_array(arr, -0.6e+16, true);
        check_array(arr, 0.4e+105, true);
        check_array(arr, 1.7e+308, true);
        check_array(arr, Infinity, true);
        check_array(arr, NaN, false);
        check_array(arr, 0, false);
        check_array(arr, 1024.0, false);
    }
    let data_inf0 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    check_every_element(data_inf0);
    let data_inf1 = Float64Array.of(-1.7e+308, Infinity, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    check_every_element(data_inf1);
    let data_inf2 = Float64Array.of(-1.7e+308, -3.6e+42, Infinity, -0.6e+16, 0.4e+105, 1.7e+308);
    check_every_element(data_inf2);
    let data_inf3 = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, Infinity, 0.4e+105, 1.7e+308);
    check_every_element(data_inf3);
    let data_inf4 = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, Infinity, 1.7e+308);
    check_every_element(data_inf4);
    let data_inf5 = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308, Infinity);
    check_every_element(data_inf5);
}

function test_f32_array_inf() {
    const check_every_element = (arr: Float32Array) => {
        check_array(arr, -3.4e+38, true);
        check_array_for_float(arr, -3.4e+38f);
        check_array(arr, -2.02e+16, true);
        check_array_for_float(arr, -2.02e+16f);
        check_array(arr, 0, true);
        check_array_for_float(arr, 0f);
        check_array(arr, 1.06e+12, true);
        check_array_for_float(arr, 1.06e+12f);
        check_array(arr, 2.8e+24, true);
        check_array_for_float(arr, 2.8e+24f);
        check_array(arr, 3.4e+38, true);
        check_array_for_float(arr, 3.4e+38f);
        check_array(arr, Infinity, true);
        check_array_for_float(arr, Infinity.toFloat());
        check_array(arr, NaN, false);
        check_array_for_float(arr, NaN.toFloat());
        check_array(arr, (3.4e+38 - 1e+32), false);
        check_array_for_float(arr, 3.4e+38f - 1e32f);
        check_array(arr, 0.0000001, false);
        check_array_for_float(arr, 0.0000001f);
        check_array(arr, 1024.0, false);
        check_array_for_float(arr, 1024.0f);
    }
    let data_inf0 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_every_element(data_inf0);
    let data_inf1 = Float32Array.of(-3.4e+38, Infinity, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_every_element(data_inf1);
    let data_inf2 = Float32Array.of(-3.4e+38, -2.02e+16, Infinity, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_every_element(data_inf2);
    let data_inf3 = Float32Array.of(-3.4e+38, -2.02e+16, 0, Infinity, 1.06e+12, 2.8e+24, 3.4e+38);
    check_every_element(data_inf3);
    let data_inf4 = Float32Array.of(-3.4e+38, -2.02e+16, 0, 1.06e+12, Infinity, 2.8e+24, 3.4e+38);
    check_every_element(data_inf4);
    let data_inf5 = Float32Array.of(-3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, Infinity, 3.4e+38);
    check_every_element(data_inf5);
    let data_inf6 = Float32Array.of(-3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38, Infinity);
    check_every_element(data_inf6);
}

function test_f64_array_nan() {
    const check_nan = (arr: Float64Array, nan_position: number) => {
        arktest.assertLT(1, arr.length);
        arktest.assertLT(nan_position, arr.length);
        check_array(arr, NaN, true);
        check_array(arr, NaN, true, 0 as number);
        check_array(arr, NaN, true, nan_position);
        check_array(arr, NaN, true, -(arr.length + nan_position));
        check_array(arr, NaN, false, nan_position + 1);
        check_array(arr, NaN, true, -arr.length);
        check_array(arr, NaN, false, arr.length);
        check_array(arr, NaN, nan_position == arr.length - 1, arr.length - 1);
        check_array(arr, NaN, true, -(arr.length + 1));
        check_array(arr, NaN, true, -(arr.length - nan_position));
    }
    const check_no_nan = (arr: Float64Array, nan_position: number) => {
        arktest.assertLT(1, arr.length);
        const val_position: number = nan_position < 4 ? 4 : 3;
        arktest.assertLT(val_position, arr.length);
        const value: number = arr[val_position];
        arktest.assertFalse(isNaN(value));
        check_array(arr, value, true);
        check_array(arr, value, true, val_position);
        check_array(arr, value, false, arr.length + val_position);
        check_array(arr, value, false, arr.length);
        check_array(arr, value, true, -arr.length);
        for (let p = 0; p <= val_position; p++) {
            check_array(arr, value, true, p);
            check_array(arr, value, true, -(arr.length - p));
        }
        check_array(arr, value, false, -(arr.length - val_position - 1));
        check_array(arr, value, true, arr.length - (arr.length - val_position));
        check_array(arr, Infinity, true);
        check_array(arr, -100, false);
    }
    let data_nan0 = Float64Array.of(NaN, Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    check_nan(data_nan0, 0);
    check_no_nan(data_nan0, 0);
    let data_nan1 = Float64Array.of(Infinity, NaN, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    check_nan(data_nan1, 1);
    check_no_nan(data_nan1, 1);
    let data_nan2 = Float64Array.of(Infinity, -1.7e+308, NaN, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308);
    check_nan(data_nan2, 2);
    check_no_nan(data_nan2, 2);
    let data_nan3 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, NaN, -0.6e+16, 0.4e+105, 1.7e+308);
    check_nan(data_nan3, 3);
    check_no_nan(data_nan3, 3);
    let data_nan4 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, NaN, 0.4e+105, 1.7e+308);
    check_nan(data_nan4, 4);
    check_no_nan(data_nan4, 4);
    let data_nan5 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, NaN, 1.7e+308);
    check_nan(data_nan5, 5);
    check_no_nan(data_nan5, 5);
    let data_nan6 = Float64Array.of(Infinity, -1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308, NaN);
    check_nan(data_nan6, 6);
    check_no_nan(data_nan6, 6);
}

function test_f32_array_nan() {
    const check_nan = (arr: Float32Array, nan_position: number) => {
        arktest.assertLT(1, arr.length);
        arktest.assertLT(nan_position, arr.length);
        check_array(arr, NaN, true);
        check_array(arr, NaN, true, 0 as number);
        check_array(arr, NaN, true, nan_position);
        check_array(arr, NaN, true, -(arr.length + nan_position));
        check_array(arr, NaN, false, nan_position + 1);
        check_array(arr, NaN, true, -arr.length);
        check_array(arr, NaN, false, arr.length);
        check_array(arr, NaN, nan_position == arr.length - 1, arr.length - 1);
        check_array(arr, NaN, true, -(arr.length + 1));
        check_array(arr, NaN, true, -(arr.length - nan_position));
        check_array_for_float(arr, NaN.toFloat()); // No special case for NaN in Float32Array.includes(float)
    }
    const check_no_nan = (arr: Float32Array, nan_position: number) => {
        arktest.assertLT(1, arr.length);
        const val_position: number = nan_position < 5 ? 5 : 4;
        arktest.assertLT(val_position, arr.length);
        const value: number = arr[val_position];
        arktest.assertFalse(isNaN(value));
        check_array(arr, 0, true);
        check_array(arr, Infinity, true);
        check_array(arr, value, true);
        check_array(arr, value, true, val_position);
        check_array(arr, value, false, arr.length + val_position);
        check_array(arr, value, false, arr.length);
        check_array(arr, value, true, -arr.length);
        for (let p = 0; p <= val_position; p++) {
            check_array(arr, value, true, p);
            check_array(arr, value, true, -(arr.length - p));
        }
        check_array(arr, value, false, -(arr.length - val_position - 1));
        check_array(arr, value, true, arr.length - (arr.length - val_position));
        check_array_for_float(arr, 0f);
        check_array_for_float(arr, -100.01f);
    }
    let data_nan0 = Float32Array.of(NaN, Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_nan(data_nan0, 0);
    check_no_nan(data_nan0, 0);
    let data_nan1 = Float32Array.of(Infinity, NaN, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_nan(data_nan1, 1);
    check_no_nan(data_nan1, 1);
    let data_nan2 = Float32Array.of(Infinity, -3.4e+38, NaN, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_nan(data_nan2, 2);
    check_no_nan(data_nan2, 2);
    let data_nan3 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, NaN, 0, 1.06e+12, 2.8e+24, 3.4e+38);
    check_nan(data_nan3, 3);
    check_no_nan(data_nan3, 3);
    let data_nan4 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, NaN, 1.06e+12, 2.8e+24, 3.4e+38);
    check_nan(data_nan4, 4);
    check_no_nan(data_nan4, 4);
    let data_nan5 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, NaN, 2.8e+24, 3.4e+38);
    check_nan(data_nan5, 5);
    check_no_nan(data_nan5, 5);
    let data_nan6 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, NaN, 3.4e+38);
    check_nan(data_nan6, 6);
    check_no_nan(data_nan6, 6);
    let data_nan7 = Float32Array.of(Infinity, -3.4e+38, -2.02e+16, 0, 1.06e+12, 2.8e+24, 3.4e+38, NaN);
    check_nan(data_nan7, 7);
    check_no_nan(data_nan7, 7);
}

function test_f64_array_nan_only() {
    const check_nan_only = (arr: Float64Array) => {
        check_array(arr, NaN, true);
        check_array(arr, NaN, false, arr.length);
        check_array(arr, NaN, true, -arr.length);
        check_array(arr, NaN, true, -(arr.length + 1));
        check_array(arr, 0, false);
    }
    check_nan_only(Float64Array.of(NaN));
    check_nan_only(Float64Array.of(NaN, NaN));
    check_nan_only(Float64Array.of(NaN, NaN, NaN));
}

function test_f32_array_nan_only() {
    const check_nan_only = (arr: Float32Array) => {
        check_array(arr, NaN, true);
        check_array(arr, NaN, false, arr.length);
        check_array(arr, NaN, true, -arr.length);
        check_array(arr, NaN, true, -(arr.length + 1));
        check_array_for_float(arr, NaN.toFloat());
        check_array(arr, 0, false);
        check_array_for_float(arr, 0f);
    }
    check_nan_only(Float32Array.of(NaN));
    check_nan_only(Float32Array.of(NaN, NaN));
    check_nan_only(Float32Array.of(NaN, NaN, NaN));
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("typedArray.includes");
    testSuite.addTest("Float64 Arrays: includes for arrays w/o NaN", test_f64_array_noNaN);
    testSuite.addTest("Float64 Arrays: includes for arrays w/o NaN with fromIndex", test_f64_array_noNaN_from_index);
    testSuite.addTest("Float64 Arrays: includes for arrays with Infinity", test_f64_array_inf);
    testSuite.addTest("Float64 Arrays: includes for arrays with NaN", test_f64_array_nan);
    testSuite.addTest("Float64 Arrays: includes for arrays of NaN only", test_f64_array_nan_only);
    testSuite.addTest("Float32 Arrays: includes for arrays w/o NaN", test_f32_array_noNaN);
    testSuite.addTest("Float32 Arrays: includes for arrays w/o NaN with fromIndex", test_f32_array_noNaN_from_index);
    testSuite.addTest("Float32 Arrays: includes for arrays with Infinity", test_f32_array_inf);
    testSuite.addTest("Float32 Arrays: includes for arrays with NaN", test_f32_array_nan);
    testSuite.addTest("Float32 Arrays: includes for arrays of NaN only", test_f32_array_nan_only);
    return testSuite.run();
}
