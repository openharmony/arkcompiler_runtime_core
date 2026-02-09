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
//! RUN_PAOC      options: "--compiler-regex=.*::test.*", entry: "ets_escompat_typed_arrays_with.ETSGLOBAL::main"
//! RUN           entry: "ets_escompat_typed_arrays_with.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testU8ClampedArray"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testI8Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testU8Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testI16Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testU16Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/

//! CHECKER       JIT: TypedArray.with()
//! RUN           force_jit: true, options: "--compiler-regex=.*::test.*",
//!               entry: "ets_escompat_typed_arrays_with.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testU8ClampedArray"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testI8Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testU8Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testI16Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/
//!
//! METHOD        "ets_escompat_typed_arrays_with.ETSGLOBAL::testU16Array"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.*Array::with/

function testU8ClampedArray() {
    let src  = new Uint8ClampedArray([0, 7, 74, 181, 255])
    let exp1 = new Uint8ClampedArray([0, 0, 74, 181, 255])
    let exp2 = new Uint8ClampedArray([0, 255, 74, 181, 255])
    let exp3 = new Uint8ClampedArray([0, 7, 74, 181, 255])
    let val1: int = -1;
    let val2: int = 256;
    checkArray(src.with(1 as int, val1), exp1)
    checkArray(src.with(1 as int, val2), exp2)
    checkArray(src, exp3)
}

function testU8ClampedArrayRangeerror() {
    let src = new Uint8ClampedArray([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => {
 src.with(-1 as int, val) 
}, expectedRangeErrors)
    arktest.expectThrow(() => {
 src.with(src.length.toInt(), val) 
}, expectedRangeErrors)
}

function testI8Array() {
    let src  = new Int8Array([-128, -56, 0, 64, 128])
    let exp1 = new Int8Array([-128, 100, 0, 64, 128])
    let exp2 = new Int8Array([-128, -56, 0, 64, 128])
    let val: byte = 100
    checkArray(src.with(1 as int, val), exp1)
    checkArray(src, exp2)
}

function testI8ArrayRangeerror() {
    let src = new Int8Array([1, 1, 1])
    let val: byte = 0;
    arktest.expectThrow(() => {
 src.with(-1 as int, val) 
}, expectedRangeErrors)
    arktest.expectThrow(() => {
 src.with(src.length.toInt(), val) 
}, expectedRangeErrors)
}

function testU8Array() {
    let src  = new Uint8Array([0, 7, 74, 181, 255])
    let exp1 = new Uint8Array([0, 4, 74, 181, 255])
    let exp2 = new Uint8Array([0, 7, 74, 181, 255])
    let val: int = 260
    checkArray(src.with(1, val), exp1)
    checkArray(src, exp2)
}

function testU8ArrayRangeerror() {
    let src = new Uint8Array([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => {
 src.with(-1 as int, val) 
}, expectedRangeErrors)
    arktest.expectThrow(() => {
 src.with(src.length.toInt(), val) 
}, expectedRangeErrors)
}

function testI16Array() {
    let src  = new Int16Array([-32768, -1321, 0, 10321, 32767])
    let exp1 = new Int16Array([-32768, 10000, 0, 10321, 32767])
    let exp2 = new Int16Array([-32768, -1321, 0, 10321, 32767])
    let val: short = 10000;
    checkArray(src.with(1 as int, val), exp1)
    checkArray(src, exp2)
}

function testI16ArrayRangeerror() {
    let src = new Int16Array([1, 1, 1])
    let val: short = 0;
    arktest.expectThrow(() => {
 src.with(-1 as int, val) 
}, expectedRangeErrors)
    arktest.expectThrow(() => {
 src.with(src.length.toInt(), val) 
}, expectedRangeErrors)
}

function testU16Array() {
    let src  = new Uint16Array([0, 62, 538, 35891, 65535])
    let exp1 = new Uint16Array([0, 14, 538, 35891, 65535])
    let exp2 = new Uint16Array([0, 62, 538, 35891, 65535])
    let val: int = 65550;
    checkArray(src.with(1, val), exp1)
    checkArray(src, exp2)
}

function testU16ArrayRangeerror() {
    let src = new Uint16Array([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => {
 src.with(-1 as int, val) 
}, expectedRangeErrors)
    arktest.expectThrow(() => {
 src.with(src.length.toInt(), val) 
}, expectedRangeErrors)
}

function checkArray<T extends Number | BigInt>(data: ArrayLike<T>, expected: ArrayLike<T>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length')
    for (let i: int = 0; i < data.length; i++) {
        arktest.assertEQ(data[i], expected[i], 'Unexpected element with index ' + i)
    }
}

function main(): void {
    testU8ClampedArray()
    testU8ClampedArrayRangeerror()
    testI8Array()
    testI8ArrayRangeerror()
    testU8Array()
    testU8ArrayRangeerror()
    testI16Array()
    testI16ArrayRangeerror()
    testU16Array()
    testU16ArrayRangeerror()
}
