/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError) || (e instanceof IndexOutOfBoundsError)
}

//! CHECKER       AOT: TypedArray.with()
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*::test.*", entry: "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::main"
//! RUN           entry: "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testI32Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testU32Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testI64Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testU64Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testF32Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testF64Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!

//! CHECKER       JIT: TypedArray.with()
//! RUN           force_jit: true, options: "--compiler-regex=.*::test.*", entry: "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testI32Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testU32Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testI64Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testU64Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testF32Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with_part2.ETSGLOBAL::testF64Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/

function testI32Array() {
    let src  = new Int32Array([-2147483648, -20543, 0, 20543, 2147483647])
    let exp1 = new Int32Array([-2147483648, 100000, 0, 20543, 2147483647])
    let exp2 = new Int32Array([-2147483648, -20543, 0, 20543, 2147483647])
    let val: int = 100000;
    checkArray(src.with(1 as int, val), exp1)
    checkArray(src, exp2)
}

function testI32ArrayRangeerror() {
    let src = new Int32Array([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function testU32Array() {
    let src  = new Uint32Array([0, 25974, 57356, 2054306, 4294967295])
    let exp1 = new Uint32Array([0, 10004, 57356, 2054306, 4294967295])
    let exp2 = new Uint32Array([0, 25974, 57356, 2054306, 4294967295])
    let val: long = 4294977300;
    checkArray(src.with(1, val), exp1)
    checkArray(src, exp2)
}

function testU32ArrayRangeerror() {
    let src = new Uint32Array([1, 1, 1])
    let val: long = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function testI64Array() {
    let src  = new BigInt64Array([-9.223e+18, -404221, 0, 404221, 9.223e+18])
    let exp1 = new BigInt64Array([-9.223e+18, 1000000, 0, 404221, 9.223e+18])
    let exp2 = new BigInt64Array([-9.223e+18, -404221, 0, 404221, 9.223e+18])
    let val: long = 1000000;
    checkArray(src.with(1 as int, val), exp1)
    checkArray(src, exp2)
}

function testI64ArrayRangeerror() {
    let src = new BigInt64Array([1, 1, 1])
    let val: long = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function testU64Array() {
    let src  = new BigUint64Array([0, 2598574, 4042214, 6.446e+11, 18.446e+18])
    let exp1 = new BigUint64Array([0, 1000000, 4042214, 6.446e+11, 18.446e+18])
    let exp2 = new BigUint64Array([0, 2598574, 4042214, 6.446e+11, 18.446e+18])
    let val: long = 1000000;
    checkArray(src.with(1, val), exp1)
    checkArray(src, exp2)
}

function testU64ArrayRangeerror() {
    let src = new BigUint64Array([1, 1, 1])
    let val: long = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function testF32Array() {
    let src  = new Float32Array([-3.4e+38, -1.5e+16, 0.0, 1.5e+16, 3.4e+38])
    let exp1 = new Float32Array([-3.4e+38, 5.05e+16, 0.0, 1.5e+16, 3.4e+38])
    let exp2 = new Float32Array([-3.4e+38, -1.5e+16, 0.0, 1.5e+16, 3.4e+38])
    let val: float = 5.05e+16f;
    checkArray(src.with(1 as int, val), exp1)
    checkArray(src, exp2)
}

function testF32ArrayRangeerror() {
    let src = new Float32Array([1, 1, 1])
    let val: float = 0.0f;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}


function testF64Array() {
    let src  = new Float64Array([-1.7e+308, -3.4e+72, 0.0, 3.4e+72, 1.7e+308])
    let exp1 = new Float64Array([-1.7e+308, 5.05e+32, 0.0, 3.4e+72, 1.7e+308])
    let exp2 = new Float64Array([-1.7e+308, -3.4e+72, 0.0, 3.4e+72, 1.7e+308])
    let val: double = 5.05e+32;
    checkArray(src.with(1 as int, val), exp1)
    checkArray(src, exp2)
}

function testF64ArrayRangeerror() {
    let src = new Float64Array([1, 1, 1])
    let val: double = 0.0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function checkArray<T extends Number | BigInt>(data: ArrayLike<T>, expected: ArrayLike<T>): void {
    arktest.assertEQ(data.length, expected.length, "Unexpected array length")
    for (let i: int = 0; i < data.length; i++) {
        arktest.assertEQ(data[i], expected[i], "Unexpected element with index " + i)
    }
}

function main(): void {
    testI32Array()
    testI32ArrayRangeerror()
    testU32Array()
    testU32ArrayRangeerror()
    testI64Array()
    testI64ArrayRangeerror()
    testU64Array()
    testU64ArrayRangeerror()
    testF32Array()
    testF32ArrayRangeerror()
    testF64Array()
    testF64ArrayRangeerror()
}
