/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER       Int8Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetInt8array() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Int8Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int8Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt8arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetInt8arrayRangeerror() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Int8Array, src: Int8Array) {
    dst.set(src)
}

//! CHECKER       Int8Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt8arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetInt8arrayWithOffset() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Int8Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int8Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameInt8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameInt8array() {
    let sample = Int8Array.of(1, 2, 3, 4)
    let src = new Int8Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Int8Array.of(1, 1, 2, 4))
}

//! CHECKER       Int8Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt8arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetInt8arrayWithOffsetRangeerror() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Int8Array, src: Int8Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Int16Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetInt16array() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Int16Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int16Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt16arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetInt16arrayRangeerror() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Int16Array, src: Int16Array) {
    dst.set(src)
}

//! CHECKER       Int16Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt16arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetInt16arrayWithOffset() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Int16Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int16Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameInt16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameInt16array() {
    let sample = Int16Array.of(1, 2, 3, 4)
    let src = new Int16Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Int16Array.of(1, 1, 2, 4))
}

//! CHECKER       Int16Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt16arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetInt16arrayWithOffsetRangeerror() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Int16Array, src: Int16Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Int32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetInt32array() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Int32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt32arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetInt32arrayRangeerror() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Int32Array, src: Int32Array) {
    dst.set(src)
}

//! CHECKER       Int32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt32arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetInt32arrayWithOffset() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Int32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameInt32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameInt32array() {
    let sample = Int32Array.of(1, 2, 3, 4)
    let src = new Int32Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Int32Array.of(1, 1, 2, 4))
}

//! CHECKER       Int32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetInt32arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetInt32arrayWithOffsetRangeerror() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Int32Array, src: Int32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       BigInt64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBigint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetBigint64array() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, BigInt64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       BigInt64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBigint64arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetBigint64arrayRangeerror() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: BigInt64Array, src: BigInt64Array) {
    dst.set(src)
}

//! CHECKER       BigInt64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBigint64arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetBigint64arrayWithOffset() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, BigInt64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       BigInt64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameBigint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameBigint64array() {
    let sample = BigInt64Array.of(1, 2, 3, 4)
    let src = new BigInt64Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, BigInt64Array.of(1, 1, 2, 4))
}

//! CHECKER       BigInt64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBigint64arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetBigint64arrayWithOffsetRangeerror() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: BigInt64Array, src: BigInt64Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Float32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetFloat32array() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Float32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Float32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat32arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetFloat32arrayRangeerror() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Float32Array, src: Float32Array) {
    dst.set(src)
}

//! CHECKER       Float32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat32arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetFloat32arrayWithOffset() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Float32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Float32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameFloat32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameFloat32array() {
    let sample = Float32Array.of(1, 2, 3, 4)
    let src = new Float32Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Float32Array.of(1, 1, 2, 4))
}

//! CHECKER       Float32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat32arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetFloat32arrayWithOffsetRangeerror() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Float32Array, src: Float32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Float64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetFloat64array() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Float64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Float64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat64arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetFloat64arrayRangeerror() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Float64Array, src: Float64Array) {
    dst.set(src)
}

//! CHECKER       Float64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat64arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetFloat64arrayWithOffset() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Float64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Float64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameFloat64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameFloat64array() {
    let sample = Float64Array.of(1, 2, 3, 4)
    let src = new Float64Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Float64Array.of(1, 1, 2, 4))
}

//! CHECKER       Float64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetFloat64arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetFloat64arrayWithOffsetRangeerror() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Float64Array, src: Float64Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint8Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint8array() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Uint8Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       AOT Uint8Array set array intrinsic with RangeError
//! RUN_PAOC      options:
//!                "--compiler-regex-with-signature='.*::setArray\\(escompat.Uint8Array, escompat.Uint8Array\\)'"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
//! RUN           entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8arrayRangeerror"

//! CHECKER       Uint8Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint8arrayRangeerror() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Uint8Array, src: Uint8Array) {
    dst.set(src)
}

//! CHECKER       AOT Uint8Array set array with offset intrinsic
//! RUN_PAOC      options:
//!                "--compiler-regex-with-signature='.*::setArray\\(escompat.Uint8Array, escompat.Uint8Array, i32\\)'"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
//! RUN           entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8arrayWithOffset"

//! CHECKER       Uint8Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint8arrayWithOffset() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Uint8Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint8Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameUint8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameUint8array() {
    let sample = Uint8Array.of(1, 2, 3, 4)
    let src = new Uint8Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Uint8Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint8Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint8arrayWithOffsetRangeerror() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Uint8Array, src: Uint8Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint8ClampedArray set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8clampedarray"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint8clampedarray() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Uint8ClampedArray.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint8ClampedArray set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8clampedarrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint8clampedarrayRangeerror() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Uint8ClampedArray, src: Uint8ClampedArray) {
    dst.set(src)
}

//! CHECKER       Uint8ClampedArray set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8clampedarrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint8clampedarrayWithOffset() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Uint8ClampedArray.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint8ClampedArray set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameUint8clampedarray"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameUint8clampedarray() {
    let sample = Uint8ClampedArray.of(1, 2, 3, 4)
    let src = new Uint8ClampedArray(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Uint8ClampedArray.of(1, 1, 2, 4))
}

//! CHECKER       Uint8ClampedArray set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint8clampedarrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint8clampedarrayWithOffsetRangeerror() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Uint8ClampedArray, src: Uint8ClampedArray, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint16Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint16array() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Uint16Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint16Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint16arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint16arrayRangeerror() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Uint16Array, src: Uint16Array) {
    dst.set(src)
}

//! CHECKER       Uint16Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint16arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint16arrayWithOffset() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Uint16Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint16Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameUint16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameUint16array() {
    let sample = Uint16Array.of(1, 2, 3, 4)
    let src = new Uint16Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Uint16Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint16Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint16arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint16arrayWithOffsetRangeerror() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Uint16Array, src: Uint16Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint32array() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, Uint32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint32arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetUint32arrayRangeerror() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: Uint32Array, src: Uint32Array) {
    dst.set(src)
}

//! CHECKER       Uint32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint32arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint32arrayWithOffset() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, Uint32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameUint32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameUint32array() {
    let sample = Uint32Array.of(1, 2, 3, 4)
    let src = new Uint32Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, Uint32Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetUint32arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetUint32arrayWithOffsetRangeerror() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

function setArray(dst: Uint32Array, src: Uint32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       BigUint64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBiguint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetBiguint64array() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(10)
    dst.fill(77)
    setArray(dst, src)
    checkArray(dst, BigUint64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       BigUint64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBiguint64arrayRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function testSetBiguint64arrayRangeerror() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(4)
    arktest.expectThrow(() => {
 setArray(dst, src) 
}, expectedRangeErrors)
}

function setArray(dst: BigUint64Array, src: BigUint64Array) {
    dst.set(src)
}

//! CHECKER       BigUint64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBiguint64arrayWithOffset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetBiguint64arrayWithOffset() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(10)
    dst.fill(77)
    setArray(dst, src, 4)
    checkArray(dst, BigUint64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       BigUint64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetSameBiguint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetSameBiguint64array() {
    let sample = BigUint64Array.of(1, 2, 3, 4)
    let src = new BigUint64Array(sample.buffer, 0, 2)
    setArray(sample, src, 1)
    checkArray(sample, BigUint64Array.of(1, 1, 2, 4))
}

//! CHECKER       BigUint64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::setArray",
//!               entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::testSetBiguint64arrayWithOffsetRangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::setArray"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function testSetBiguint64arrayWithOffsetRangeerror() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(5)
    arktest.expectThrow(() => {
 setArray(dst, src, 1) 
}, expectedRangeErrors)
}

// Helpers
const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError)
}

function setArray(dst: BigUint64Array, src: BigUint64Array, offset: int) {
    dst.set(src, offset)
}

function checkArray(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) {
        throw new Error('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error('Unexpected element with index ' + i)
        }
    }
}

function checkArray(arr: BigInt64Array, expected: BigInt64Array) {
    if (arr.length != expected.length) {
        throw new Error('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error('Unexpected element with index ' + i)
        }
    }
}

function checkArray(arr: BigUint64Array, expected: BigUint64Array) {
    if (arr.length != expected.length) {
        throw new Error('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error('Unexpected element with index ' + i)
        }
    }
}
