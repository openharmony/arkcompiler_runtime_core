/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

//! CHECKER       IR Builder inlining min int
//! RUN           force_jit: true, options: "--compiler-regex='.*test_i32.*'", entry: "ETSGLOBAL::test_i32"
//! METHOD        "ETSGLOBAL::test_i32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 6
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining min int
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_i32.*'"
//! METHOD        "ETSGLOBAL::test_i32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 6
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ETSGLOBAL::test_i32"

//! CHECKER       IR Builder inlining min long
//! RUN           force_jit: true, options: "--compiler-regex='.*test_i64.*'", entry: "ETSGLOBAL::test_i64"
//! METHOD        "ETSGLOBAL::test_i64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 6
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining min long
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_i64.*'"
//! METHOD        "ETSGLOBAL::test_i64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 6
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ETSGLOBAL::test_i64"

//! CHECKER       IR Builder inlining min float
//! RUN           force_jit: true, options: "--compiler-regex='.*test_f32.*'", entry: "ETSGLOBAL::test_f32"
//! METHOD        "ETSGLOBAL::test_f32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 8
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining min float
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_f32.*'"
//! METHOD        "ETSGLOBAL::test_f32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 8
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ETSGLOBAL::test_f32"

//! CHECKER      IR Builder inlining min double
//! RUN          force_jit: true, options: "--compiler-regex='.*test_f64.*'", entry: "ETSGLOBAL::test_f64"
//! METHOD       "ETSGLOBAL::test_f64"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT    /Min/, 8
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining min double
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_f64.*'"
//! METHOD        "ETSGLOBAL::test_f64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Min/, 8
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ETSGLOBAL::test_f64"


function test_i32() : int {
    if (min(Int.MAX_VALUE as int, 0 as int) != 0 as int) return 1;
    if (min(Int.MIN_VALUE as int, 0 as int) != Int.MIN_VALUE as int) return 2;
    if (min(0 as int, -2 as int) != -2 as int) return 3;
    if (min(0 as int, 3 as int) != 0 as int) return 4;
    if (min(-2 as int, -4 as int) != -4 as int) return 5;
    if (min(1 as int, 4 as int) != 1 as int) return 6;

    return 0;
}

function test_i64() : int {
    if (min(Long.MAX_VALUE, 0) != 0) return 11;
    if (min(Long.MIN_VALUE, 0) != Long.MIN_VALUE) return 12;
    if (min(0, -2) != -2) return 13;
    if (min(0, 3) != 0) return 14;
    if (min(-2, -4) != -4) return 15;
    if (min(1, 4) != 1) return 16;

    return 0;
}

function test_f32() : int {
    if (min(Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY) != Float.NEGATIVE_INFINITY) return 31;
    if (min(Float.MAX_VALUE, Float.NEGATIVE_INFINITY) != Float.NEGATIVE_INFINITY) return 32;
    if (min(0. as float, Float.NEGATIVE_INFINITY) != Float.NEGATIVE_INFINITY) return 33;
    if (min(0. as float, Float.MIN_VALUE) != 0. as float) return 34;
    if (min(Float.MAX_VALUE, Float.MIN_VALUE) != Float.MIN_VALUE) return 35;
    if (min(0. as float, 2. as float) != 0. as float) return 36;
    if (min(0. as float, -4. as float) != -4. as float) return 37;
    if (min(1.e-10 as float, 2.e+12 as float) != 1.e-10 as float) return 38;

    return 0;
}

function test_f64() : int {
    if (min(Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) return 51;
    if (min(Double.MAX_VALUE, Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) return 52;
    if (min(0, Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) return 53;
    if (min(0, Double.MIN_VALUE) != 0) return 54;
    if (min(Double.MAX_VALUE, Double.MIN_VALUE) != Double.MIN_VALUE) return 55;
    if (min(0, 2) != 0) return 56;
    if (min(0, -4) != -4) return 57;
    if (min(1.e-100, 2.e+200) != 1.e-100) return 58;

    return 0;
}
