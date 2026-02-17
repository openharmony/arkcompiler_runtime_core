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

const beginPlus : int = 1
const endMinus : int = 1

//! CHECKER       JIT TypedArray slice intrinsic
//! RUN           force_jit: true,  options: "--compiler-regex=.*::testSlice.*array",
//!               entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt8array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt16array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt32array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigint64array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat32array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat64array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint16array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint32array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBiguint64array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8clampedarray"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/


//! CHECKER       AOT TypedArray slice intrinsic
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*::testSlice.*array",
//!               entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt8array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt16array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt32array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigint64array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat32array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat64array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint16array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint32array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBiguint64array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8clampedarray"
//! PASS_AFTER    "IrBuilder"

function testSliceInt8array() {
    let src = new Int8Array([-128, -41, -14, 0, 17, 45, 127])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceInt16array() {
    let src = new Int16Array([-32768, -2732, -227, 0, 128, 2563, 32767])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceInt32array() {
    let src = new Int32Array([-2147483648, -1397, 0, 11185, 89478, 536873, 2147483647])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceBigint64array() {
    let src = new BigInt64Array([-9.223e+18, -8589934588, 4294967294, 2.3058e+16, 9.223e+18])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceFloat32array() {
    let src = new Float32Array([-3.4e+38, -2.02e+16, 1.06e+12, 2.8e+24,  3.4e+38])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceFloat64array() {
    let src = new Float64Array([-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceUint8array() {
    let src = new Uint8Array([0, 17, 45, 127, 14, 41, 255])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceUint16array() {
    let src = new Uint16Array([128, 0, 2563, 32767, 227, 2732, 65535])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceUint32array() {
    let src = new Uint32Array([11185, 89478, 0, 536873, 2147483647, 1397, 4294967295])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceBiguint64array() {
    let src = new BigUint64Array([4294967294, 2.3058e+16, 9.223e+18, 8589934588, 18.446e+18])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function testSliceUint8clampedarray() {
    let src = new Uint8ClampedArray([17, 45, 127, 0, 255, 43, 14])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function checkArray<T extends Number | BigInt>(sourceData: ArrayLike<T>, actualData: ArrayLike<T>): void {
    const actualLen = (actualData.length).toInt()
    const expectedLen = (sourceData.length).toInt() - (beginPlus + endMinus)
    if (actualLen != expectedLen) {
        throw new Error('Unexpected array length! Current: ' + actualLen + ' Expected: ' + expectedLen)
    }
    for (let i : int = 0; i < expectedLen; i++) {
        if (sourceData[i + beginPlus] != actualData[i]) {
            throw new Error('Unexpected element with index ' + i)
        }
    }
}

function main(): void {
    testSliceInt8array()
    testSliceInt16array()
    testSliceInt32array()
    testSliceBigint64array()
    testSliceFloat32array()
    testSliceFloat64array()
    testSliceUint8array()
    testSliceUint16array()
    testSliceUint32array()
    testSliceBiguint64array()
    testSliceUint8clampedarray()
}
