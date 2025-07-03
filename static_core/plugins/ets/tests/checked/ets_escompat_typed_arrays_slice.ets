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

const beginPlus : int = 1
const endMinus : int = 1

//! CHECKER    JIT Int8Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt8Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt8Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.Int8ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Int8Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt8Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt8Array"
//! PASS_AFTER "IrBuilder"

function testSliceInt8Array() {
    let src = new Int8Array([-128, -41, -14, 0, 17, 45, 127])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Int16Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt16Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt16Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.Int16ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Int16Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt16Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt16Array"
//! PASS_AFTER "IrBuilder"

function testSliceInt16Array() {
    let src = new Int16Array([-32768, -2732, -227, 0, 128, 2563, 32767])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Int32Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt32Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt32Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.Int32ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Int32Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt32Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceInt32Array"
//! PASS_AFTER "IrBuilder"

function testSliceInt32Array() {
    let src = new Int32Array([-2147483648, -1397, 0, 11185, 89478, 536873, 2147483647])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT BigInt64Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigInt64Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigInt64Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.BigInt64ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT BigInt64Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigInt64Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigInt64Array"
//! PASS_AFTER "IrBuilder"

function testSliceBigInt64Array() {
    let src = new BigInt64Array([-9.223e+18, -8589934588, 4294967294, 2.3058e+16, 9.223e+18])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Float32Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat32Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat32Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.Float32ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Float32Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat32Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat32Array"
//! PASS_AFTER "IrBuilder"

function testSliceFloat32Array() {
    let src = new Float32Array([-3.4e+38, -2.02e+16, 1.06e+12, 2.8e+24, 3.4e+38])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Float64Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat64Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat64Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.Float64ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Float64rray sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat64Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceFloat64Array"
//! PASS_AFTER "IrBuilder"

function testSliceFloat64Array() {
    let src = new Float64Array([-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Uint8Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.UInt8ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Uint8Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8Array"
//! PASS_AFTER "IrBuilder"

function testSliceUint8Array() {
    let src = new Uint8Array([0, 17, 45, 127, 14, 41, 255])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Uint16Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint16Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint16Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.UInt16ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Uint16Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint16Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint16Array"
//! PASS_AFTER "IrBuilder"

function testSliceUint16Array() {
    let src = new Uint16Array([128, 0, 2563, 32767, 227, 2732, 65535])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Uint32Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint32Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint32Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.UInt32ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Uint32Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint32Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint32Array"
//! PASS_AFTER "IrBuilder"

function testSliceUint32Array() {
    let src = new Uint32Array([11185, 89478, 0, 536873, 2147483647, 1397, 4294967295])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT BigUint64Array sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigUint64Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigUint64Array"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.BigUInt64ArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT BigUint64Array sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUBigUint64Array"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceBigUint64Array"
//! PASS_AFTER "IrBuilder"

function testSliceBigUint64Array() {
    let src = new BigUint64Array([4294967294, 2.3058e+16, 9.223e+18, 8589934588, 18.446e+18])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

//! CHECKER    JIT Uint8ClampedArray sliceImpl intrinsic
//! RUN        force_jit: true, entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8ClampedArray"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8ClampedArray"
//! PASS_AFTER "Inline"
//! INST       "Intrinsic.UInt8ClampedArraySlice "
//! INST_NOT   /Call.*Array::sliceImpl/

//! CHECKER    AOT Uint8ClampedArray sliceImpl intrinsic
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8ClampedArray"
//! METHOD     "ets_escompat_typed_arrays_slice.ETSGLOBAL::testSliceUint8ClampedArray"
//! PASS_AFTER "IrBuilder"

function testSliceUint8ClampedArray() {
    let src = new Uint8ClampedArray([17, 45, 127, 0, 255, 43, 14])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    checkArray(src, dst)
}

function checkArray<T extends Number | BigInt>(data: ArrayLike<T>, expectedData: ArrayLike<T>): void {
    if ((data.length).toInt() != (expectedData.length).toInt() + beginPlus + endMinus) {
        throw new Exception('Unexpected array length')
    }
    for (let i : int = 0; i < (expectedData.length).toInt(); i++) {
        if (data[i + beginPlus] != expectedData[i]) {
            throw new Exception('Unexpected element with index ' + i)
        }
    }
}
