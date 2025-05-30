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

//! CHECKER       Int8Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt8Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt8Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillInt8Array() {
    let a = new Int8Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, Int8Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       Int16Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt16Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt16Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillInt16Array() {
    let a = new Int16Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, Int16Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       Int32Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt32Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillInt32Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillInt32Array() {
    let a = new Int32Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, Int32Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       Float32Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat32Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat32Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillFloat32Array() {
    let a = new Float32Array(6)
    a.fill(0)
    a.fill(77.77, 2, 5)
    a.fill(NaN, 0, 2)
    a.fill(Infinity, 4, 6)
    checkArray(a, Float32Array.of(NaN, NaN, 77.77, 77.77, Infinity, Infinity))
}

//! CHECKER       Float64Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat64Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillFloat64Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillFloat64Array() {
    let a = new Float64Array(6)
    a.fill(0)
    a.fill(77.77, 2, 5)
    a.fill(NaN, 0, 2)
    a.fill(Infinity, 4, 6)
    checkArray(a, Float64Array.of(NaN, NaN, 77.77, 77.77, Infinity, Infinity))
}

//! CHECKER       BigInt64Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillBigInt64Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillBigInt64Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigInt64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillBigInt64Array() {
    let a = new BigInt64Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, BigInt64Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       Uint8Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint8Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint8Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt8ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillUint8Array() {
    let a = new Uint8Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, Uint8Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       Uint8ClampedArray fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint8ClampedArray"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint8ClampedArray"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt8ClampedArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillUint8ClampedArray() {
    let a = new Uint8ClampedArray(6)
    a.fill(0)
    a.fill(256, 2, 5)
    checkArray(a, Uint8ClampedArray.of(0, 0, 255, 255, 255, 0))
}

//! CHECKER       Uint16Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint16Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint16Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt16ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillUint16Array() {
    let a = new Uint16Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, Uint16Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       Uint32Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint32Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillUint32Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillUint32Array() {
    let a = new Uint32Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, Uint32Array.of(0, 0, 77, 77, 77, 0))
}

//! CHECKER       BigUint64Array fill intrinsic
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillBigUint64Array"
//! METHOD        "ets_escompat_typed_arrays_fill.ETSGLOBAL::testFillBigUint64Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUInt64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
function testFillBigUint64Array() {
    let a = new BigUint64Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    checkArray(a, BigUint64Array.of(0, 0, 77, 77, 77, 0))
}

function checkArray(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) {
        throw new Exception('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (isNaN(arr[i]) && isNaN(expected[i])) {
            continue
        }
        if (arr[i] != expected[i]) {
            throw new Exception('Unexpected element with index ' + i)
        }
    }
}

function checkArray(arr: BigInt64Array, expected: BigInt64Array) {
    if (arr.length != expected.length) {
        throw new Exception('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Exception('Unexpected element with index ' + i)
        }
    }
}

function checkArray(arr: BigUint64Array, expected: BigUint64Array) {
    if (arr.length != expected.length) {
        throw new Exception('Unexpected array length')
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Exception('Unexpected element with index ' + i)
        }
    }
}
