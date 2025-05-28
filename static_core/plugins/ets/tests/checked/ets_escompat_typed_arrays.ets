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

//! CHECKER       Int8Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int8array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Int8Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int8Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int8array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array_rangeerror() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Int8Array, src: Int8Array) {
    dst.set(src)
}

//! CHECKER       Int8Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int8array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array_with_offset() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Int8Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int8Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_int8array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_int8array() {
    let sample = Int8Array.of(1, 2, 3, 4)
    let src = new Int8Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Int8Array.of(1, 1, 2, 4))
}

//! CHECKER       Int8Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int8array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array_with_offset_rangeerror() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Int8Array, src: Int8Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Int16Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int16array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Int16Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int16Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int16array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array_rangeerror() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Int16Array, src: Int16Array) {
    dst.set(src)
}

//! CHECKER       Int16Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int16array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array_with_offset() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Int16Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int16Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_int16array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_int16array() {
    let sample = Int16Array.of(1, 2, 3, 4)
    let src = new Int16Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Int16Array.of(1, 1, 2, 4))
}

//! CHECKER       Int16Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int16array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array_with_offset_rangeerror() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Int16Array, src: Int16Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Int32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int32array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Int32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int32array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array_rangeerror() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Int32Array, src: Int32Array) {
    dst.set(src)
}

//! CHECKER       Int32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int32array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array_with_offset() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Int32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_int32array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_int32array() {
    let sample = Int32Array.of(1, 2, 3, 4)
    let src = new Int32Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Int32Array.of(1, 1, 2, 4))
}

//! CHECKER       Int32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_int32array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array_with_offset_rangeerror() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Int32Array, src: Int32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       BigInt64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_bigint64array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, BigInt64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       BigInt64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_bigint64array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array_rangeerror() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: BigInt64Array, src: BigInt64Array) {
    dst.set(src)
}

//! CHECKER       BigInt64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_bigint64array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array_with_offset() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, BigInt64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       BigInt64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_bigint64array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_bigint64array() {
    let sample = BigInt64Array.of(1, 2, 3, 4)
    let src = new BigInt64Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, BigInt64Array.of(1, 1, 2, 4))
}

//! CHECKER       BigInt64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_bigint64array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array_with_offset_rangeerror() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: BigInt64Array, src: BigInt64Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Float32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float32array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Float32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Float32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float32array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array_rangeerror() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Float32Array, src: Float32Array) {
    dst.set(src)
}

//! CHECKER       Float32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float32array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array_with_offset() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Float32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Float32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_float32array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_float32array() {
    let sample = Float32Array.of(1, 2, 3, 4)
    let src = new Float32Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Float32Array.of(1, 1, 2, 4))
}

//! CHECKER       Float32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float32array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array_with_offset_rangeerror() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Float32Array, src: Float32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Float64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float64array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Float64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Float64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float64array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array_rangeerror() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Float64Array, src: Float64Array) {
    dst.set(src)
}

//! CHECKER       Float64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float64array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array_with_offset() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Float64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Float64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_float64array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_float64array() {
    let sample = Float64Array.of(1, 2, 3, 4)
    let src = new Float64Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Float64Array.of(1, 1, 2, 4))
}

//! CHECKER       Float64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_float64array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array_with_offset_rangeerror() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Float64Array, src: Float64Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint8Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint8Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       AOT Uint8Array set array intrinsic with RangeError
//! RUN_PAOC      options: "--compiler-regex-with-signature='.*::set_array\\(escompat.Uint8Array, escompat.Uint8Array\\)'"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
//! RUN           entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8array_rangeerror"

//! CHECKER       Uint8Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array_rangeerror() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint8Array, src: Uint8Array) {
    dst.set(src)
}

//! CHECKER       AOT Uint8Array set array with offset intrinsic
//! RUN_PAOC      options: "--compiler-regex-with-signature='.*::set_array\\(escompat.Uint8Array, escompat.Uint8Array, i32\\)'"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
//! RUN           entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8array_with_offset"

//! CHECKER       Uint8Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array_with_offset() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint8Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint8Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_uint8array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint8array() {
    let sample = Uint8Array.of(1, 2, 3, 4)
    let src = new Uint8Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint8Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint8Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array_with_offset_rangeerror() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint8Array, src: Uint8Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint8ClampedArray set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8clampedarray"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint8ClampedArray.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint8ClampedArray set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8clampedarray_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray_rangeerror() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint8ClampedArray, src: Uint8ClampedArray) {
    dst.set(src)
}

//! CHECKER       Uint8ClampedArray set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8clampedarray_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray_with_offset() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint8ClampedArray.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint8ClampedArray set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_uint8clampedarray"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint8clampedarray() {
    let sample = Uint8ClampedArray.of(1, 2, 3, 4)
    let src = new Uint8ClampedArray(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint8ClampedArray.of(1, 1, 2, 4))
}

//! CHECKER       Uint8ClampedArray set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint8clampedarray_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray_with_offset_rangeerror() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint8ClampedArray, src: Uint8ClampedArray, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint16Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint16array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint16Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint16Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint16array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array_rangeerror() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint16Array, src: Uint16Array) {
    dst.set(src)
}

//! CHECKER       Uint16Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint16array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array_with_offset() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint16Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint16Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_uint16array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint16array() {
    let sample = Uint16Array.of(1, 2, 3, 4)
    let src = new Uint16Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint16Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint16Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint16array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array_with_offset_rangeerror() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint16Array, src: Uint16Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint32array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint32array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array_rangeerror() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint32Array, src: Uint32Array) {
    dst.set(src)
}

//! CHECKER       Uint32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint32array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array_with_offset() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_uint32array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint32array() {
    let sample = Uint32Array.of(1, 2, 3, 4)
    let src = new Uint32Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint32Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_uint32array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array_with_offset_rangeerror() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: Uint32Array, src: Uint32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       BigUint64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_biguint64array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, BigUint64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       BigUint64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_biguint64array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array_rangeerror() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(4)
    try {
        set_array(dst, src)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: BigUint64Array, src: BigUint64Array) {
    dst.set(src)
}

//! CHECKER       BigUint64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_biguint64array_with_offset"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array_with_offset() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, BigUint64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       BigUint64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_same_biguint64array"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_biguint64array() {
    let sample = BigUint64Array.of(1, 2, 3, 4)
    let src = new BigUint64Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, BigUint64Array.of(1, 1, 2, 4))
}

//! CHECKER       BigUint64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_set_biguint64array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array_with_offset_rangeerror() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(5)
    try {
        set_array(dst, src, 1)
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

function set_array(dst: BigUint64Array, src: BigUint64Array, offset: int) {
    dst.set(src, offset)
}

function check_array(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) {
        throw new Exception("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Exception("Unexpected element with index " + i)
        }
    }
}

function check_array(arr: BigInt64Array, expected: BigInt64Array) {
    if (arr.length != expected.length) {
        throw new Exception("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Exception("Unexpected element with index " + i)
        }
    }
}

function check_array(arr: BigUint64Array, expected: BigUint64Array) {
    if (arr.length != expected.length) {
        throw new Exception("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Exception("Unexpected element with index " + i)
        }
    }
}

//! CHECKER       Int8Array $_set byte/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_byte_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_byte_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_byte_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetByte "
//! INST_NOT      "Intrinsic.Int8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_int8array_set_byte_cpp(): void {
    let ar = new Int8Array(10)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    assertEquals(-128, ar[2])
    assertEquals(127, ar[4])
}

//! CHECKER       Int8Array $_set byte/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int8array_set_byte_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_byte_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_byte_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArraySetByte "
//! INST_NOT      "Intrinsic.Int8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_int8array_set_byte_ir(): void {
    let ar = new Int8Array(10)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    assertEquals(-128, ar[2])
    assertEquals(127, ar[4])
}

//! CHECKER       Int8Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetInt "
//! INST_NOT      "Intrinsic.Int8ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_int8array_set_int_cpp(): void {
    let ar = new Int8Array(10)
    ar[2] = -128
    ar[3] = -129
    ar[4] = 127
    ar[5] = 128
    assertEquals(-128, ar[2])
    assertEquals(127, ar[3])
    assertEquals(127, ar[4])
    assertEquals(-128, ar[5])
}

//! CHECKER       Int8Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int8array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArraySetInt "
//! INST_NOT      "Intrinsic.Int8ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_int8array_set_int_ir(): void {
    let ar = new Int8Array(10)
    ar[2] = -128
    ar[3] = -129
    ar[4] = 127
    ar[5] = 128
    assertEquals(-128, ar[2])
    assertEquals(127, ar[3])
    assertEquals(127, ar[4])
    assertEquals(-128, ar[5])
}

//! CHECKER       Int8Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Int8Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testInt8ArrayGetUnsafe"
//! METHOD        "escompat_Int8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Int8Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Int8Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testInt8ArrayGetUnsafe"
//! METHOD        "escompat_Int8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testInt8ArrayGetUnsafe(): void {
    let ar = new Int8Array(10)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    arktest.assertEQ(-128, ar.at(2))
    arktest.assertEQ(127, ar.at(4))
}

//! CHECKER       Int8Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_int8array_set_rangeerror_cpp(): void {
    let ar = new Int8Array(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Int8Array $_set number/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_number_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_number_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArraySetByte "
//! INST          "Intrinsic.Int8ArrayGet "
function test_int8array_set_number_cpp(): void {
    let ar = new Int8Array(10)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    assertEquals(0, ar[2])
    assertEquals(0, ar[4])
    assertEquals(0, ar[5])
}

//! CHECKER       Int16Array $_set short/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_short_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_short_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_short_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetShort "
//! INST_NOT      "Intrinsic.Int16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_int16array_set_short_cpp(): void {
    let ar = new Int16Array(10)
    ar[2] = (-32768).toShort()
    ar[4] = (32767).toShort()
    assertEquals(-32768, ar[2])
    assertEquals(32767, ar[4])
}

//! CHECKER       Int16Array $_set short/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int16array_set_short_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_short_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_short_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArraySetShort "
//! INST_NOT      "Intrinsic.Int16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_int16array_set_short_ir(): void {
    let ar = new Int16Array(10)
    ar[2] = (-32768).toShort()
    ar[4] = (32767).toShort()
    assertEquals(-32768, ar[2])
    assertEquals(32767, ar[4])
}

//! CHECKER       Int16Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetInt "
//! INST_NOT      "Intrinsic.Int16ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_int16array_set_int_cpp(): void {
    let ar = new Int16Array(10)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    assertEquals(-32768, ar[2])
    assertEquals(32767, ar[3])
    assertEquals(32767, ar[4])
    assertEquals(-32768, ar[5])
}

//! CHECKER       Int16Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int16array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArraySetInt "
//! INST_NOT      "Intrinsic.Int16ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_int16array_set_int_ir(): void {
    let ar = new Int16Array(10)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    assertEquals(-32768, ar[2])
    assertEquals(32767, ar[3])
    assertEquals(32767, ar[4])
    assertEquals(-32768, ar[5])
}

//! CHECKER       Int16Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Int16Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testInt16ArrayGetUnsafe"
//! METHOD        "escompat_Int16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Int16Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Int16Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testInt16ArrayGetUnsafe"
//! METHOD        "escompat_Int16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testInt16ArrayGetUnsafe(): void {
    let ar = new Int16Array(10)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    arktest.assertEQ(-32768, ar.at(2))
    arktest.assertEQ(32767, ar.at(3))
    arktest.assertEQ(32767, ar.at(4))
    arktest.assertEQ(-32768, ar.at(5))
}

//! CHECKER       Int16Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_int16array_set_rangeerror_cpp(): void {
    let ar = new Int16Array(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Int16Array $_set number/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_number_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_number_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArraySetShort "
//! INST          "Intrinsic.Int16ArrayGet "
function test_int16array_set_number_cpp(): void {
    let ar = new Int16Array(10)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    assertEquals(0, ar[2])
    assertEquals(0, ar[4])
    assertEquals(0, ar[5])
}

//! CHECKER       Int32Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int32array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_int32array_set_int_cpp(): void {
    let ar = new Int32Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    assertEquals(-2147483648, ar[2])
    assertEquals(2147483647, ar[4])
}

//! CHECKER       Int32Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int32array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_int32array_set_int_ir(): void {
    let ar = new Int32Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    assertEquals(-2147483648, ar[2])
    assertEquals(2147483647, ar[4])
}

//! CHECKER       Int32Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Int32Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testInt32ArrayGetUnsafe"
//! METHOD        "escompat_Int32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Int32Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Int32Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testInt32ArrayGetUnsafe"
//! METHOD        "escompat_Int32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testInt32ArrayGetUnsafe(): void {
    let ar = new Int32Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar.at(2))
    arktest.assertEQ(2147483647, ar.at(4))
}

//! CHECKER       Int32Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int32array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_int32array_set_rangeerror_cpp(): void {
    let ar = new Int32Array(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Int32Array $_set number/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int32array_set_number_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_number_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST          "Intrinsic.Int32ArrayGet "
function test_int32array_set_number_cpp(): void {
    let ar = new Int32Array(10)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    assertEquals(0, ar[2])
    assertEquals(0, ar[4])
    assertEquals(0, ar[5])
}

//! CHECKER       BigInt64Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_set_int_cpp"
//! METHOD        "escompat_BigInt64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_bigint64array_set_int_cpp(): void {
    let ar = new BigInt64Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    assertEquals(-2147483648n, ar[2])
    assertEquals(2147483647n, ar[4])
}

//! CHECKER       BigInt64Array $_set long/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_set_long_cpp"
//! METHOD        "escompat_BigInt64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_bigint64array_set_long_cpp(): void {
    let ar = new BigInt64Array(10)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    assertEquals(9223372036854775807n, ar[2])
    assertEquals(2n, ar[3])
    assertEquals(-9223372036854775808n, ar[4])
    assertEquals(-2n, ar[5])
}

//! CHECKER       BigInt64Array $_set long/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_set_long_ir"
//! METHOD        "escompat_BigInt64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_bigint64array_set_long_ir(): void {
    let ar = new BigInt64Array(10)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    assertEquals(9223372036854775807n, ar[2])
    assertEquals(2n, ar[3])
    assertEquals(-9223372036854775808n, ar[4])
    assertEquals(-2n, ar[5])
}

//! CHECKER       BigInt64Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.BigInt64Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testBigInt64ArrayGetUnsafe"
//! METHOD        "escompat_BigInt64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testBigInt64ArrayGetUnsafe(): void {
    let ar = new BigInt64Array(10)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    arktest.assertEQ(9223372036854775807n, ar.at(2))
    arktest.assertEQ(2n, ar.at(3))
    arktest.assertEQ(-9223372036854775808n, ar.at(4))
    arktest.assertEQ(-2n, ar.at(5))
}

//! CHECKER       BigInt64Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_set_rangeerror_cpp"
//! METHOD        "escompat_BigInt64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
function test_bigint64array_set_rangeerror_cpp(): void {
    let ar = new BigInt64Array(10)
    try {
        ar[11] = 1n
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Float32Array $_set float/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float32array_set_float_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_set_float_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_set_float_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetFloat "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Float32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_float32array_set_float_cpp(): void {
    let ar = new Float32Array(10)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    assertEquals(-3.4028235e+38f, ar[2])
    assertEquals(3.4028235e+38f, ar[4])
}

//! CHECKER       Float32Array $_set float/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_float32array_set_float_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_set_float_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_set_float_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float32ArraySetFloat "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Float32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_float32array_set_float_ir(): void {
    let ar = new Float32Array(10)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    assertEquals(-3.4028235e+38f, ar[2])
    assertEquals(3.4028235e+38f, ar[4])
}

//! CHECKER       Float32Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Float32Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testFloat32ArrayGetUnsafe"
//! METHOD        "escompat_Float32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Float32Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Float32Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testFloat32ArrayGetUnsafe"
//! METHOD        "escompat_Float32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testFloat32ArrayGetUnsafe(): void {
    let ar = new Float32Array(10)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar.at(2))
    arktest.assertEQ(3.4028235e+38f, ar.at(4))
}

//! CHECKER       Float32Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float32array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetFloat "
//! INST_NOT      /Call.*Array::\$_set/
function test_float32array_set_rangeerror_cpp(): void {
    let ar = new Float32Array(10)
    try {
        ar[11] = 1.2f
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Float64Array $_set double/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float64array_set_double_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_set_double_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_set_double_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetDouble "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Float64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_float64array_set_double_cpp(): void {
    let ar = new Float64Array(10)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    assertEquals(-3.4028235e+38, ar[2])
    assertEquals(3.4028235e+38, ar[4])
}

//! CHECKER       Float64Array $_set double/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_float64array_set_double_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_set_double_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_set_double_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float64ArraySetDouble "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Float64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_float64array_set_double_ir(): void {
    let ar = new Float64Array(10)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    assertEquals(-3.4028235e+38, ar[2])
    assertEquals(3.4028235e+38, ar[4])
}

//! CHECKER       Float64Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Float64Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testFloat64ArrayGetUnsafe"
//! METHOD        "escompat_Float64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Float64Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Float64Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testFloat64ArrayGetUnsafe"
//! METHOD        "escompat_Float64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testFloat64ArrayGetUnsafe(): void {
    let ar = new Float64Array(10)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar.at(2))
    arktest.assertEQ(3.4028235e+38, ar.at(4))
}

//! CHECKER       Float64Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float64array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetDouble "
//! INST_NOT      /Call.*Array::\$_set/
function test_float64array_set_rangeerror_cpp(): void {
    let ar = new Float64Array(10)
    try {
        ar[11] = 1.2
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       AOT Uint8Array $_set int/$_get cpp intrinsic
//! RUN_PAOC      options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! RUN           entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_cpp"

//! CHECKER       Uint8Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_uint8array_set_int_cpp(): void {
    let ar = new Uint8Array(10)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    assertEquals(255, ar[2])
    assertEquals(255, ar[3])
    assertEquals(1, ar[4])
}

//! CHECKER       Uint8Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_uint8array_set_rangeerror_cpp(): void {
    let ar = new Uint8Array(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       AOT Uint8Array $_set int/$_get IR intrinsic
//! RUN_PAOC      options: "--compiler-regex=.*::test_uint8array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! RUN           entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_ir"

//! CHECKER       Uint8Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint8array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_uint8array_set_int_ir(): void {
    let ar = new Uint8Array(10)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    assertEquals(255, ar[2])
    assertEquals(255, ar[3])
    assertEquals(1, ar[4])
}

//! CHECKER       Uint8Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint8Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint8ArrayGetUnsafe"
//! METHOD        "escompat_Uint8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint8Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint8Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint8ArrayGetUnsafe"
//! METHOD        "escompat_Uint8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testUint8ArrayGetUnsafe(): void {
    let ar = new Uint8Array(10)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar.at(2))
    arktest.assertEQ(255, ar.at(3))
    arktest.assertEQ(1, ar.at(4))
}

//! CHECKER       Uint8ClampedArray $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8clampedarray_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ClampedArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_uint8clampedarray_set_int_cpp(): void {
    let ar = new Uint8ClampedArray(10)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    assertEquals(0, ar[2])
    assertEquals(255, ar[3])
    assertEquals(255, ar[4])
}

//! CHECKER       Uint8ClampedArray $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8clampedarray_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_uint8clampedarray_set_rangeerror_cpp(): void {
    let ar = new Uint8ClampedArray(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Uint8ClampedArray $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint8clampedarray_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ClampedArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_uint8clampedarray_set_int_ir(): void {
    let ar = new Uint8ClampedArray(10)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    assertEquals(0, ar[2])
    assertEquals(255, ar[3])
    assertEquals(255, ar[4])
}

//! CHECKER       Uint8ClampedArray getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint8ClampedArray::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint8ClampedArrayGetUnsafe"
//! METHOD        "escompat_Uint8ClampedArray::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint8ClampedArray getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint8ClampedArray::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint8ClampedArrayGetUnsafe"
//! METHOD        "escompat_Uint8ClampedArray::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ClampedArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testUint8ClampedArrayGetUnsafe(): void {
    let ar = new Uint8ClampedArray(10)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar.at(2))
    arktest.assertEQ(255, ar.at(3))
    arktest.assertEQ(255, ar.at(4))
}

//! CHECKER       Uint16Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint16array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_uint16array_set_int_cpp(): void {
    let ar = new Uint16Array(10)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    assertEquals(65535, ar[2])
    assertEquals(65535, ar[3])
    assertEquals(1, ar[4])
}

//! CHECKER       Uint16Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint16array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_uint16array_set_rangeerror_cpp(): void {
    let ar = new Uint16Array(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Uint16Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint16array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_uint16array_set_int_ir(): void {
    let ar = new Uint16Array(10)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    assertEquals(65535, ar[2])
    assertEquals(65535, ar[3])
    assertEquals(1, ar[4])
}

//! CHECKER       Uint16Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint16Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint16ArrayGetUnsafe"
//! METHOD        "escompat_Uint16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint16Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint16Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint16ArrayGetUnsafe"
//! METHOD        "escompat_Uint16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testUint16ArrayGetUnsafe(): void {
    let ar = new Uint16Array(10)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar.at(2))
    arktest.assertEQ(65535, ar.at(3))
    arktest.assertEQ(1, ar.at(4))
}

//! CHECKER       Uint32Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint32array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_uint32array_set_int_cpp(): void {
    let ar = new Uint32Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    assertEquals(4294967295, ar[2])
    assertEquals(2147483647, ar[3])
}

//! CHECKER       Uint32Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint32array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_rangeerror_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
function test_uint32array_set_rangeerror_cpp(): void {
    let ar = new Uint32Array(10)
    try {
        ar[11] = 1
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       Uint32Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint32array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_uint32array_set_int_ir(): void {
    let ar = new Uint32Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    assertEquals(4294967295, ar[2])
    assertEquals(2147483647, ar[3])
}

//! CHECKER       Uint32Array $_set long/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint32array_set_long_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_long_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_long_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_uint32array_set_long_cpp(): void {
    let ar = new Uint32Array(10)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    assertEquals(4294967295, ar[2])
    assertEquals(4294967295, ar[3])
    assertEquals(1, ar[4])
}

//! CHECKER       Uint32Array $_set long/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint32array_set_long_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_long_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_set_long_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_uint32array_set_long_ir(): void {
    let ar = new Uint32Array(10)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    assertEquals(4294967295, ar[2])
    assertEquals(4294967295, ar[3])
    assertEquals(1, ar[4])
}

//! CHECKER       Uint32Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint32Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint32ArrayGetUnsafe"
//! METHOD        "escompat_Uint32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint32Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint32Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testUint32ArrayGetUnsafe"
//! METHOD        "escompat_Uint32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testUint32ArrayGetUnsafe(): void {
    let ar = new Uint32Array(10)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar.at(2))
    arktest.assertEQ(4294967295, ar.at(3))
    arktest.assertEQ(1, ar.at(4))
}

//! CHECKER       BigUint64Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*::test_biguint64array_set_int_cpp", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_int_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.BigUInt64ArrayGet "
function test_biguint64array_set_int_cpp(): void {
    let ar = new BigUint64Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    assertEquals(18446744073709551615n, ar[2])
    assertEquals(2147483647n, ar[3])
}

//! CHECKER       BigUint64Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_biguint64array_set_int_ir", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_int_ir"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.BigUInt64ArrayGet "
//! EVENT_NOT     /Deoptimization/
function test_biguint64array_set_int_ir(): void {
    let ar = new BigUint64Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    assertEquals(18446744073709551615n, ar[2])
    assertEquals(2147483647n, ar[3])
}

//! CHECKER       BigUint64Array $_set long/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigUint64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_long_cpp"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigUint64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_biguint64array_set_long_cpp(): void {
    let ar = new BigUint64Array(10)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    assertEquals(2n, ar[2])
    assertEquals(18446744073709551614n, ar[3])
    assertEquals(18446744073709551615n, ar[4])
    assertEquals(18446744073709551615n, ar[5])
}

//! CHECKER       BigUint64Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigUint64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_rangeerror_cpp"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
function test_biguint64array_set_rangeerror_cpp(): void {
    let ar = new BigUint64Array(10)
    try {
        ar[11] = 1n
        throw new Exception("Missing RangeError")
    } catch (e: RangeError) {
    }
}

//! CHECKER       BigUint64Array $_set long/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*BigUint64Array::.*", entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_set_long_ir"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigUint64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! EVENT_NOT     /Deoptimization/
function test_biguint64array_set_long_ir(): void {
    let ar = new BigUint64Array(10)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    assertEquals(2n, ar[2])
    assertEquals(18446744073709551614n, ar[3])
    assertEquals(18446744073709551615n, ar[4])
    assertEquals(18446744073709551615n, ar[5])
}

//! CHECKER       BigUint64Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.BigUint64Array::at", entry: "ets_escompat_typed_arrays.ETSGLOBAL::testBigUint64ArrayGetUnsafe"
//! METHOD        "escompat_BigUint64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! EVENT_NOT     /Deoptimization/
function testBigUint64ArrayGetUnsafe(): void {
    let ar = new BigUint64Array(10)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar.at(2))
    arktest.assertEQ(18446744073709551614n, ar.at(3))
    arktest.assertEQ(18446744073709551615n, ar.at(4))
    arktest.assertEQ(18446744073709551615n, ar.at(5))
}

function assertEquals(expected: number, actual: number): void {
    if (expected != actual) throw new AssertionError(`${expected} != ${actual}`)
}

function assertEquals(expected: float, actual: float): void {
    if (expected != actual) throw new AssertionError(`${expected} != ${actual}`)
}

function assertEquals(expected: BigInt, actual: BigInt): void {
    if (expected != actual) throw new AssertionError(`${expected} != ${actual}`)
}

//! CHECKER       Int8Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_int8array_copy_within() {
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Int8Array.of(1,1,2,3,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Int8Array.of(1,1,2,3,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Int8Array.of(1,1,2,3,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Int8Array.of(1,1,2,3,4))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Int8Array.of(1,2,3,4,5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Int8Array.of(1,1,2,3,4))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Int8Array.of(1,1,2,3,4))
}

//! CHECKER       Int16Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_int16array_copy_within() {
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Int16Array.of(1,1,2,3,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Int16Array.of(1,1,2,3,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Int16Array.of(1,1,2,3,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Int16Array.of(1,1,2,3,4))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Int16Array.of(1,2,3,4,5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Int16Array.of(1,1,2,3,4))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Int16Array.of(1,1,2,3,4))
}

//! CHECKER       Int32Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_int32array_copy_within() {
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Int32Array.of(1,1,2,3,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Int32Array.of(1,1,2,3,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Int32Array.of(1,1,2,3,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Int32Array.of(1,1,2,3,4))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Int32Array.of(1,2,3,4,5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Int32Array.of(1,1,2,3,4))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Int32Array.of(1,1,2,3,4))
}

//! CHECKER       BigInt64Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_bigint64array_copy_within() {
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), BigInt64Array.of(1,1,2,3,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), BigInt64Array.of(1,1,2,3,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), BigInt64Array.of(1,1,2,3,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), BigInt64Array.of(1,1,2,3,4))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), BigInt64Array.of(1,2,3,4,5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), BigInt64Array.of(1,1,2,3,4))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), BigInt64Array.of(1,1,2,3,4))
}

//! CHECKER       Float32Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_float32array_copy_within() {
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Float32Array.of(1,1,2,3,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Float32Array.of(1,1,2,3,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Float32Array.of(1,1,2,3,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Float32Array.of(1,1,2,3,4))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Float32Array.of(1,2,3,4,5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Float32Array.of(1,1,2,3,4))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Float32Array.of(1,1,2,3,4))
}

//! CHECKER       Float64Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_float64array_copy_within() {
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Float64Array.of(1,1,2,3,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Float64Array.of(1,1,2,3,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Float64Array.of(1,1,2,3,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Float64Array.of(1,1,2,3,4))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Float64Array.of(1,2,3,4,5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Float64Array.of(1,1,2,3,4))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Float64Array.of(1,1,2,3,4))
}

//! CHECKER       Uint8Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_uint8array_copy_within() {
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Uint8Array.of(1,1,2,3,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Uint8Array.of(1,1,2,3,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Uint8Array.of(1,1,2,3,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Uint8Array.of(1,1,2,3,4))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Uint8Array.of(1,2,3,4,5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Uint8Array.of(1,1,2,3,4))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Uint8Array.of(1,1,2,3,4))
}

//! CHECKER       Uint16Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_uint16array_copy_within() {
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Uint16Array.of(1,1,2,3,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Uint16Array.of(1,1,2,3,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Uint16Array.of(1,1,2,3,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Uint16Array.of(1,1,2,3,4))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Uint16Array.of(1,2,3,4,5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Uint16Array.of(1,1,2,3,4))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Uint16Array.of(1,1,2,3,4))
}

//! CHECKER       Uint32Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_uint32array_copy_within() {
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Uint32Array.of(1,1,2,3,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Uint32Array.of(1,1,2,3,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Uint32Array.of(1,1,2,3,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Uint32Array.of(1,1,2,3,4))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Uint32Array.of(1,2,3,4,5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Uint32Array.of(1,1,2,3,4))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Uint32Array.of(1,1,2,3,4))
}

//! CHECKER       BigUint64Array copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_biguint64array_copy_within() {
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), BigUint64Array.of(1,1,2,3,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), BigUint64Array.of(1,1,2,3,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), BigUint64Array.of(1,1,2,3,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), BigUint64Array.of(1,1,2,3,4))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), BigUint64Array.of(1,2,3,4,5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), BigUint64Array.of(1,1,2,3,4))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), BigUint64Array.of(1,1,2,3,4))
}

//! CHECKER       AOT Uint8ClampedArray copyWithin
//! RUN_PAOC      options: "--compiler-regex=.*::test_uint8clampedarray_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Call.*Array::copyWithin/
//! RUN           entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_copy_within"

//! CHECKER       Uint8ClampedArray copyWithin
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_copy_within"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArrayCopyWithin"
//! INST_NOT      /Call.*Array::copyWithin/
function test_uint8clampedarray_copy_within() {
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  0,  3), Uint8ClampedArray.of(1,1,2,3,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(-5,  0,  3), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(-6,  0,  3), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 9,  0,  3), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1, -1,  3), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1, -5,  3), Uint8ClampedArray.of(1,1,2,3,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1, -6,  3), Uint8ClampedArray.of(1,1,2,3,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  5,  3), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  9,  3), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  0, -1), Uint8ClampedArray.of(1,1,2,3,4))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  0, -5), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  0, -6), Uint8ClampedArray.of(1,2,3,4,5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  0,  5), Uint8ClampedArray.of(1,1,2,3,4))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin( 1,  0,  9), Uint8ClampedArray.of(1,1,2,3,4))
}

//! CHECKER       Int8Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_index_of"
function test_int8array_index_of() {
    let array = new Int8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.);
    arktest.assertEQ(array.indexOf(3., -1), -1.);
    arktest.assertEQ(array.indexOf(3., -7),  2.);
}

//! CHECKER       Int16Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_index_of"
function test_int16array_index_of() {
    let array = new Int16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.);
    arktest.assertEQ(array.indexOf(3.,  8), -1.);
    arktest.assertEQ(array.indexOf(3., -1), -1.);
    arktest.assertEQ(array.indexOf(3., -7),  2.);
}

//! CHECKER       Int32Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_index_of"
function test_int32array_index_of() {
    let array = new Int32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3.,  0),  2.);
    arktest.assertEQ(array.indexOf(3.,  6), -1.);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.);
    arktest.assertEQ(array.indexOf(3 , -1), -1.);
    arktest.assertEQ(array.indexOf(3 , -7),  2.);
}

//! CHECKER       BigInt64Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_index_of"
function test_bigint64array_index_of() {
    let array = new BigInt64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(new BigInt(3),  0),  2.);
    arktest.assertEQ(array.indexOf(new BigInt(3),  6), -1.);
    arktest.assertEQ(array.indexOf(3,  8), -1.);
    arktest.assertEQ(array.indexOf(3, -1), -1.);
    arktest.assertEQ(array.indexOf(3, -7),  2.);
}

//! CHECKER       Float32Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_index_of"
function test_float32array_index_of() {
    let array = new Float32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3.,  0),  2.);
    arktest.assertEQ(array.indexOf(3.,  6), -1.);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.);
    arktest.assertEQ(array.indexOf(3 , -1), -1.);
    arktest.assertEQ(array.indexOf(3 , -7),  2.);
}

//! CHECKER       Float64Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_index_of"
function test_float64array_index_of() {
    let array = new Float64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3,   0),  2.);
    arktest.assertEQ(array.indexOf(3,   6), -1.);
    arktest.assertEQ(array.indexOf(3.,  8), -1.);
    arktest.assertEQ(array.indexOf(3., -1), -1.);
    arktest.assertEQ(array.indexOf(3., -7),  2.);
}

//! CHECKER       Uint8Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_index_of"
function test_uint8array_index_of() {
    let array = new Uint8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.);
    arktest.assertEQ(array.indexOf(3., -1), -1.);
    arktest.assertEQ(array.indexOf(3., -7),  2.);
}

//! CHECKER       Uint16Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_index_of"
function test_uint16array_index_of() {
    let array = new Uint16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.);
    arktest.assertEQ(array.indexOf(3.,  8), -1.);
    arktest.assertEQ(array.indexOf(3., -1), -1.);
    arktest.assertEQ(array.indexOf(3., -7),  2.);
}

//! CHECKER       Uint32Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_index_of"
function test_uint32array_index_of() {
    let array = new Uint32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3.,  0),  2.);
    arktest.assertEQ(array.indexOf(3.,  6), -1.);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.);
    arktest.assertEQ(array.indexOf(3 , -1), -1.);
    arktest.assertEQ(array.indexOf(3 , -7),  2.);
}

//! CHECKER       BigUint64Array indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_index_of"
function test_biguint64array_index_of() {
    let array = new BigUint64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(new BigInt(3),  0),  2.);
    arktest.assertEQ(array.indexOf(new BigInt(3),  6), -1.);
    arktest.assertEQ(array.indexOf(3,  8), -1.);
    arktest.assertEQ(array.indexOf(3, -1), -1.);
    arktest.assertEQ(array.indexOf(3, -7),  2.);
}

//! CHECKER       UInt8ClampedArray indexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_index_of"
function test_uint8clampedarray_index_of() {
    let array = new Uint8ClampedArray([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.);
    arktest.assertEQ(array.indexOf(3., -1), -1.);
    arktest.assertEQ(array.indexOf(3., -7),  2.);
}

//! CHECKER       Int8Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int8array_last_index_of"
function test_int8array_last_index_of() {
    let array = new Int8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

//! CHECKER       Int16Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int16array_last_index_of"
function test_int16array_last_index_of() {
    let array = new Int16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3.,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3.,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3.,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3 , -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3 , -7), -1.);
}

//! CHECKER       Int32Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_int32array_last_index_of"
function test_int32array_last_index_of() {
    let array = new Int32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3.,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3 , -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

//! CHECKER       BigInt64Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_bigint64array_last_index_of"
function test_bigint64array_last_index_of() {
    let array = new BigInt64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  0), -1.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3, -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3, -7), -1.);
}

//! CHECKER       Float32Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float32array_last_index_of"
function test_float32array_last_index_of() {
    let array = new Float32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3.,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3.,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3.,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3 , -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3 , -7), -1.);
}

//! CHECKER       Float64Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_float64array_last_index_of"
function test_float64array_last_index_of() {
    let array = new Float64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

//! CHECKER       Uint8Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8array_last_index_of"
function test_uint8array_last_index_of() {
    let array = new Uint8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

//! CHECKER       Uint16Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint16array_last_index_of"
function test_uint16array_last_index_of() {
    let array = new Uint16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

//! CHECKER       Uint32Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint32array_last_index_of"
function test_uint32array_last_index_of() {
    let array = new Uint32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

//! CHECKER       BigUint64Array lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_biguint64array_last_index_of"
function test_biguint64array_last_index_of() {
    let array = new BigUint64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  8),  4.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3), -1),  4.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3), -7), -1.);
}

//! CHECKER       AOT Uint8ClampedArray lastIndexOf
//! RUN_PAOC      options: "--compiler-regex=.*::test_uint8clampedarray_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! RUN           entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_last_index_of"

//! CHECKER       Uint8ClampedArray lastIndexOf
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_last_index_of"
//! METHOD        "ets_escompat_typed_arrays.ETSGLOBAL::test_uint8clampedarray_last_index_of"
function test_uint8clampedarray_last_index_of() {
    let array = new Uint8ClampedArray([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3 ,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}
