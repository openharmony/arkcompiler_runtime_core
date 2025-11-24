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

//! CHECKER       JIT Test cpp intrinsics
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*::test_.*array_set_.*cpp", entry: "ets_escompat_typed_arrays_set_get_part2.ETSGLOBAL::main"
//!
//! METHOD        "test_uint8array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_uint8array_set_rangeerror_cpp"
//!
//! METHOD        "test_uint8clampedarray_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ClampedArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_uint8clampedarray_set_rangeerror_cpp"
//!
//! METHOD        "test_uint16array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_uint16array_set_rangeerror_cpp"
//!
//! METHOD        "test_uint32array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_uint32array_set_rangeerror_cpp"
//!
//! METHOD        "test_uint32array_set_long_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_biguint64array_set_int_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.BigUInt64ArrayGet "
//!
//! METHOD        "test_biguint64array_set_long_cpp"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigUint64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_biguint64array_set_rangeerror_cpp"
//! METHOD        "escompat_BigUint64Array::__set"


//! CHECKER       JIT Test IR intrinsics
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_.*array_set_.*ir", entry: "ets_escompat_typed_arrays_set_get_part2.ETSGLOBAL::main"
//!
//! METHOD        "test_uint8array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_uint8clampedarray_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ClampedArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_uint16array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_uint32array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_uint32array_set_long_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_biguint64array_set_int_ir"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.BigUInt64ArrayGet "
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_biguint64array_set_long_ir"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! METHOD        "escompat_BigUint64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/

function test_uint8array_set_int_cpp(): void {
    let ar = new Uint8Array(6)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(1, ar[4])
}

function test_uint8array_set_rangeerror_cpp(): void {
    let ar = new Uint8Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_uint8array_set_int_ir(): void {
    let ar = new Uint8Array(6)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(1, ar[4])
}

function test_uint8clampedarray_set_int_cpp(): void {
    let ar = new Uint8ClampedArray(6)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(255, ar[4])
}

function test_uint8clampedarray_set_rangeerror_cpp(): void {
    let ar = new Uint8ClampedArray(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_uint8clampedarray_set_int_ir(): void {
    let ar = new Uint8ClampedArray(6)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(255, ar[4])
}

function test_uint16array_set_int_cpp(): void {
    let ar = new Uint16Array(6)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar[2])
    arktest.assertEQ(65535, ar[3])
    arktest.assertEQ(1, ar[4])
}

function test_uint16array_set_rangeerror_cpp(): void {
    let ar = new Uint16Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_uint16array_set_int_ir(): void {
    let ar = new Uint16Array(6)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar[2])
    arktest.assertEQ(65535, ar[3])
    arktest.assertEQ(1, ar[4])
}


function test_uint32array_set_int_cpp(): void {
    let ar = new Uint32Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(2147483647, ar[3])
}

function test_uint32array_set_rangeerror_cpp(): void {
    let ar = new Uint32Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_uint32array_set_int_ir(): void {
    let ar = new Uint32Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(2147483647, ar[3])
}

function test_uint32array_set_long_cpp(): void {
    let ar = new Uint32Array(6)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(4294967295, ar[3])
    arktest.assertEQ(1, ar[4])
}

function test_uint32array_set_long_ir(): void {
    let ar = new Uint32Array(6)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(4294967295, ar[3])
    arktest.assertEQ(1, ar[4])
}

function test_biguint64array_set_int_cpp(): void {
    let ar = new BigUint64Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(18446744073709551615n, ar[2])
    arktest.assertEQ(2147483647n, ar[3])
}

function test_biguint64array_set_int_ir(): void {
    let ar = new BigUint64Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(18446744073709551615n, ar[2])
    arktest.assertEQ(2147483647n, ar[3])
}

function test_biguint64array_set_long_cpp(): void {
    let ar = new BigUint64Array(6)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar[2])
    arktest.assertEQ(18446744073709551614n, ar[3])
    arktest.assertEQ(18446744073709551615n, ar[4])
    arktest.assertEQ(18446744073709551615n, ar[5])
}

function test_biguint64array_set_rangeerror_cpp(): void {
    let ar = new BigUint64Array(6)
    arktest.expectThrow(() => { ar[11] = 1n }, expectedRangeErrors)
}

function test_biguint64array_set_long_ir(): void {
    let ar = new BigUint64Array(6)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar[2])
    arktest.assertEQ(18446744073709551614n, ar[3])
    arktest.assertEQ(18446744073709551615n, ar[4])
    arktest.assertEQ(18446744073709551615n, ar[5])
}

// Helpers
const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError)
}

function main() {
    test_uint8array_set_int_cpp()
    test_uint8array_set_rangeerror_cpp()
    test_uint8array_set_int_ir()

    test_uint8clampedarray_set_int_cpp()
    test_uint8clampedarray_set_rangeerror_cpp()
    test_uint8clampedarray_set_int_ir()

    test_uint16array_set_int_cpp()
    test_uint16array_set_rangeerror_cpp()
    test_uint16array_set_int_ir()

    test_uint32array_set_int_cpp()
    test_uint32array_set_rangeerror_cpp()
    test_uint32array_set_int_ir()
    test_uint32array_set_long_cpp()
    test_uint32array_set_long_ir()

    test_biguint64array_set_int_cpp()
    test_biguint64array_set_int_ir()
    test_biguint64array_set_long_cpp()
    test_biguint64array_set_rangeerror_cpp()
    test_biguint64array_set_long_ir()
}
