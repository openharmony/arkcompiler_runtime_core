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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*::test_.*array_set_.*cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::main"
//!
//! METHOD        "test_int8array_set_byte_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetByte "
//! INST_NOT      "Intrinsic.Int8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_int8array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetInt "
//! INST          "Intrinsic.Int8ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_int8array_set_rangeerror_cpp"
//!
//! METHOD        "test_int8array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArraySetByte "
//! INST          "Intrinsic.Int8ArrayGet "
//!
//! METHOD        "test_int16array_set_short_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetShort "
//! INST_NOT      "Intrinsic.Int16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_int16array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetInt "
//! INST_NOT      "Intrinsic.Int16ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_int16array_set_rangeerror_cpp"
//!
//! METHOD        "test_int16array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArraySetShort "
//! INST          "Intrinsic.Int16ArrayGet "
//!
//! METHOD        "test_int32array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_int32array_set_rangeerror_cpp"
//!
//! METHOD        "test_int32array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST          "Intrinsic.Int32ArrayGet "
//!
//! METHOD        "test_float32array_set_float_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetFloat "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Float32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_float32array_set_rangeerror_cpp"
//!
//! METHOD        "test_float64array_set_double_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetDouble "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Float64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_float64array_set_rangeerror_cpp"
//! 
//! METHOD        "test_bigint64array_set_int_cpp"
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_bigint64array_set_long_cpp"
//! METHOD        "escompat_BigInt64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "test_bigint64array_set_rangeerror_cpp"
//! METHOD        "escompat_BigInt64Array::__set"


//! CHECKER       JIT Test IR intrinsics
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_.*array_set_.*ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::main"
//!
//! METHOD        "test_int8array_set_byte_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArraySetByte "
//! INST_NOT      "Intrinsic.Int8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_int8array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArraySetInt "
//! INST_NOT      "Intrinsic.Int8ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_int16array_set_short_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArraySetShort "
//! INST_NOT      "Intrinsic.Int16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_int16array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArraySetInt "
//! INST_NOT      "Intrinsic.Int16ArraySetByte "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_int32array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Int32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_float32array_set_float_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float32ArraySetFloat "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Float32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_float64array_set_double_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float64ArraySetDouble "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.Float64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "test_bigint64array_set_long_ir"
//! METHOD        "escompat_BigInt64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/


function test_int8array_set_byte_cpp(): void {
    let ar = new Int8Array(6)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[4])
}

function test_int8array_set_byte_ir(): void {
    let ar = new Int8Array(6)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[4])
}

function test_int8array_set_int_cpp(): void {
    let ar = new Int8Array(6)
    ar[2] = -128
    ar[3] = -129
    ar[4] = 127
    ar[5] = 128
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[3])
    arktest.assertEQ(127, ar[4])
    arktest.assertEQ(-128, ar[5])
}

function test_int8array_set_int_ir(): void {
    let ar = new Int8Array(6)
    ar[2] = -128
    ar[3] = -129
    ar[4] = 127
    ar[5] = 128
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[3])
    arktest.assertEQ(127, ar[4])
    arktest.assertEQ(-128, ar[5])
}

function test_int8array_set_rangeerror_cpp(): void {
    let ar = new Int8Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_int8array_set_number_cpp(): void {
    let ar = new Int8Array(6)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(0, ar[4])
    arktest.assertEQ(0, ar[5])
}

function test_int16array_set_short_cpp(): void {
    let ar = new Int16Array(6)
    ar[2] = (-32768).toShort()
    ar[4] = (32767).toShort()
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[4])
}

function test_int16array_set_short_ir(): void {
    let ar = new Int16Array(6)
    ar[2] = (-32768).toShort()
    ar[4] = (32767).toShort()
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[4])
}

function test_int16array_set_int_cpp(): void {
    let ar = new Int16Array(6)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[3])
    arktest.assertEQ(32767, ar[4])
    arktest.assertEQ(-32768, ar[5])
}

function test_int16array_set_int_ir(): void {
    let ar = new Int16Array(6)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[3])
    arktest.assertEQ(32767, ar[4])
    arktest.assertEQ(-32768, ar[5])
}

function test_int16array_set_rangeerror_cpp(): void {
    let ar = new Int16Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_int16array_set_number_cpp(): void {
    let ar = new Int16Array(6)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(0, ar[4])
    arktest.assertEQ(0, ar[5])
}

function test_int32array_set_int_cpp(): void {
    let ar = new Int32Array(6)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar[2])
    arktest.assertEQ(2147483647, ar[4])
}

function test_int32array_set_int_ir(): void {
    let ar = new Int32Array(6)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar[2])
    arktest.assertEQ(2147483647, ar[4])
}

function test_int32array_set_rangeerror_cpp(): void {
    let ar = new Int32Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function test_int32array_set_number_cpp(): void {
    let ar = new Int32Array(6)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(0, ar[4])
    arktest.assertEQ(0, ar[5])
}

function test_float32array_set_float_cpp(): void {
    let ar = new Float32Array(6)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar[2])
    arktest.assertEQ(3.4028235e+38f, ar[4])
}

function test_float32array_set_float_ir(): void {
    let ar = new Float32Array(6)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar[2])
    arktest.assertEQ(3.4028235e+38f, ar[4])
}

function test_float32array_set_rangeerror_cpp(): void {
    let ar = new Float32Array(6)
    arktest.expectThrow(() => { ar[11] = 1.2f }, expectedRangeErrors)
}


function test_float64array_set_double_cpp(): void {
    let ar = new Float64Array(6)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar[2])
    arktest.assertEQ(3.4028235e+38, ar[4])
}

function test_float64array_set_rangeerror_cpp(): void {
    let ar = new Float64Array(6)
    arktest.expectThrow(() => { ar[11] = 1.2 }, expectedRangeErrors)
}

function test_float64array_set_double_ir(): void {
    let ar = new Float64Array(6)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar[2])
    arktest.assertEQ(3.4028235e+38, ar[4])
}

function test_bigint64array_set_int_cpp(): void {
    let ar = new BigInt64Array(6)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648n, ar[2])
    arktest.assertEQ(2147483647n, ar[4])
}

function test_bigint64array_set_long_cpp(): void {
    let ar = new BigInt64Array(6)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    arktest.assertEQ(9223372036854775807n, ar[2])
    arktest.assertEQ(2n, ar[3])
    arktest.assertEQ(-9223372036854775808n, ar[4])
    arktest.assertEQ(-2n, ar[5])
}

function test_bigint64array_set_long_ir(): void {
    let ar = new BigInt64Array(6)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    arktest.assertEQ(9223372036854775807n, ar[2])
    arktest.assertEQ(2n, ar[3])
    arktest.assertEQ(-9223372036854775808n, ar[4])
    arktest.assertEQ(-2n, ar[5])
}

function test_bigint64array_set_rangeerror_cpp(): void {
    let ar = new BigInt64Array(6)
    arktest.expectThrow(() => { ar[11] = 1n }, expectedRangeErrors)
}

// Helpers
const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError)
}

function main() {
    test_int8array_set_byte_cpp()
    test_int8array_set_byte_ir()
    test_int8array_set_int_cpp()
    test_int8array_set_int_ir()
    test_int8array_set_rangeerror_cpp()
    test_int8array_set_number_cpp()

    test_int16array_set_short_cpp()
    test_int16array_set_short_ir()
    test_int16array_set_int_cpp()
    test_int16array_set_int_ir()
    test_int16array_set_rangeerror_cpp()
    test_int16array_set_number_cpp()

    test_int32array_set_int_cpp()
    test_int32array_set_int_ir()
    test_int32array_set_rangeerror_cpp()
    test_int32array_set_number_cpp()

    test_float32array_set_float_cpp()
    test_float32array_set_float_ir()
    test_float32array_set_rangeerror_cpp()

    test_float64array_set_double_cpp()
    test_float64array_set_double_ir()
    test_float64array_set_rangeerror_cpp()

    test_bigint64array_set_int_cpp()
    test_bigint64array_set_long_cpp()
    test_bigint64array_set_long_ir()
    test_bigint64array_set_rangeerror_cpp()
}
