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

//! CHECKER       Array fill intrinsic
//! RUN           force_jit: true,
//!               options:"--compiler-regex=.*testFill.*", entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testMain"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt8Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt16Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt32Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat32Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat64Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/

//! CHECKER       AOT: Array fill intrinsic with byteOffset
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*testFill.*"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt8ArrayWithByteOffset"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt16ArrayWithByteOffset"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt32ArrayWithByteOffset"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat32ArrayWithByteOffset"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat64ArrayWithByteOffset"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! RUN           entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testMain", result: 0

function testFillInt8Array() {
    let a = new Int8Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, Int8Array.of(0, 0, 77, 77, 77, 0))
}

function testFillInt8ArrayWithByteOffset() {
    let buffer = new ArrayBuffer(5)
    let whole = new Int8Array(buffer)
    whole.set(Int8Array.of(1, 2, 3, 4, 5))
    let view = new Int8Array(buffer, 2, 3)
    view.fill(0)
    return checkArray(whole, Int8Array.of(1, 2, 0, 0, 0))
}

function testFillInt16Array() {
    let a = new Int16Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, Int16Array.of(0, 0, 77, 77, 77, 0))
}

function testFillInt16ArrayWithByteOffset() {
    let buffer = new ArrayBuffer(10)
    let whole = new Int16Array(buffer)
    whole.set(Int16Array.of(1, 2, 3, 4, 5))
    let view = new Int16Array(buffer, 4, 3)
    view.fill(0)
    return checkArray(whole, Int16Array.of(1, 2, 0, 0, 0))
}

function testFillInt32Array() {
    let a = new Int32Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, Int32Array.of(0, 0, 77, 77, 77, 0))
}

function testFillInt32ArrayWithByteOffset() {
    let buffer = new ArrayBuffer(20)
    let whole = new Int32Array(buffer)
    whole.set(Int32Array.of(1, 2, 3, 4, 5))
    let view = new Int32Array(buffer, 8, 3)
    view.fill(0)
    return checkArray(whole, Int32Array.of(1, 2, 0, 0, 0))
}

function testFillFloat32Array() {
    let a = new Float32Array(6)
    a.fill(0)
    a.fill(77.77, 2, 5)
    a.fill(NaN, 0, 2)
    a.fill(Infinity, 4, 6)
    return checkArray(a, Float32Array.of(NaN, NaN, 77.77, 77.77, Infinity, Infinity))
}

function testFillFloat32ArrayWithByteOffset() {
    let buffer = new ArrayBuffer(20)
    let whole = new Float32Array(buffer)
    whole.set(Float32Array.of(1.0, 2.0, 3.0, 4.0, 5.0))
    let view = new Float32Array(buffer, 8, 3)
    view.fill(0.0)
    return checkArray(whole, Float32Array.of(1.0, 2.0, 0.0, 0.0, 0.0))
}

function testFillFloat64Array() {
    let a = new Float64Array(6)
    a.fill(0)
    a.fill(77.77, 2, 5)
    a.fill(NaN, 0, 2)
    a.fill(Infinity, 4, 6)
    return checkArray(a, Float64Array.of(NaN, NaN, 77.77, 77.77, Infinity, Infinity))
}

function testFillFloat64ArrayWithByteOffset() {
    let buffer = new ArrayBuffer(40)
    let whole = new Float64Array(buffer)
    whole.set(Float64Array.of(1.0, 2.0, 3.0, 4.0, 5.0))
    let view = new Float64Array(buffer, 16, 3)
    view.fill(0.0)
    return checkArray(whole, Float64Array.of(1.0, 2.0, 0.0, 0.0, 0.0))
}

function testMain() : int {
	if (!testFillInt8Array()) {
	    return 1;
	}
	if (!testFillInt8ArrayWithByteOffset()) {
	    return 1;
	}
	if (!testFillInt16Array()) {
	    return 1;
	}
	if (!testFillInt16ArrayWithByteOffset()) {
	    return 1;
	}
	if (!testFillInt32Array()) {
	    return 1;
	}
	if (!testFillInt32ArrayWithByteOffset()) {
	    return 1;
	}
	if (!testFillFloat32Array()) {
	    return 1;
	}
	if (!testFillFloat32ArrayWithByteOffset()) {
	    return 1;
	}
	if (!testFillFloat64Array()) {
	    return 1;
	}
	if (!testFillFloat64ArrayWithByteOffset()) {
	    return 1;
	}
	return 0;
}

function checkArray(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) {
        return false;
    }

    for (let i = 0; i < expected.length; i++) {
        if (isNaN(arr[i]) && isNaN(expected[i])) {
            continue
        }
        if (arr[i] != expected[i]) {
            return false;
        }
    }
    return true;
}
