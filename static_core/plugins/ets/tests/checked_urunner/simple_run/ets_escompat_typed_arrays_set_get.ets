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

//! CHECKER       Int8Array $_set byte/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_byte_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_byte_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_byte_cpp"
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
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[4])
}

//! CHECKER       Int8Array $_set byte/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int8array_set_byte_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_byte_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_byte_ir"
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
function test_int8array_set_byte_ir(): void {
    let ar = new Int8Array(10)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[4])
}

//! CHECKER       Int8Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_int_cpp"
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
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[3])
    arktest.assertEQ(127, ar[4])
    arktest.assertEQ(-128, ar[5])
}

//! CHECKER       Int8Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int8array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_int_ir"
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
function test_int8array_set_int_ir(): void {
    let ar = new Int8Array(10)
    ar[2] = -128
    ar[3] = -129
    ar[4] = 127
    ar[5] = 128
    arktest.assertEQ(-128, ar[2])
    arktest.assertEQ(127, ar[3])
    arktest.assertEQ(127, ar[4])
    arktest.assertEQ(-128, ar[5])
}

//! CHECKER       Int8Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Int8Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testInt8ArrayGetUnsafe"
//! METHOD        "escompat_Int8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Int8Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Int8Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testInt8ArrayGetUnsafe"
//! METHOD        "escompat_Int8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
function testInt8ArrayGetUnsafe(): void {
    let ar = new Int8Array(10)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    arktest.assertEQ(-128, ar.at(2))
    arktest.assertEQ(127, ar.at(4))
}

//! CHECKER       Int8Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_rangeerror_cpp"
function test_int8array_set_rangeerror_cpp(): void {
    let ar = new Int8Array(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       Int8Array $_set number/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int8array_set_number_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_number_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int8array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArraySetByte "
//! INST          "Intrinsic.Int8ArrayGet "
function test_int8array_set_number_cpp(): void {
    let ar = new Int8Array(10)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(0, ar[4])
    arktest.assertEQ(0, ar[5])
}

//! CHECKER       Int16Array $_set short/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_short_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_short_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_short_cpp"
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
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[4])
}

//! CHECKER       Int16Array $_set short/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int16array_set_short_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_short_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_short_ir"
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
function test_int16array_set_short_ir(): void {
    let ar = new Int16Array(10)
    ar[2] = (-32768).toShort()
    ar[4] = (32767).toShort()
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[4])
}

//! CHECKER       Int16Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_int_cpp"
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
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[3])
    arktest.assertEQ(32767, ar[4])
    arktest.assertEQ(-32768, ar[5])
}

//! CHECKER       Int16Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int16array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_int_ir"
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
function test_int16array_set_int_ir(): void {
    let ar = new Int16Array(10)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    arktest.assertEQ(-32768, ar[2])
    arktest.assertEQ(32767, ar[3])
    arktest.assertEQ(32767, ar[4])
    arktest.assertEQ(-32768, ar[5])
}

//! CHECKER       Int16Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Int16Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testInt16ArrayGetUnsafe"
//! METHOD        "escompat_Int16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Int16Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Int16Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testInt16ArrayGetUnsafe"
//! METHOD        "escompat_Int16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_rangeerror_cpp"
function test_int16array_set_rangeerror_cpp(): void {
    let ar = new Int16Array(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       Int16Array $_set number/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int16array_set_number_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_number_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int16array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArraySetShort "
//! INST          "Intrinsic.Int16ArrayGet "
function test_int16array_set_number_cpp(): void {
    let ar = new Int16Array(10)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(0, ar[4])
    arktest.assertEQ(0, ar[5])
}

//! CHECKER       Int32Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int32array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Int32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_int32array_set_int_cpp(): void {
    let ar = new Int32Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar[2])
    arktest.assertEQ(2147483647, ar[4])
}

//! CHECKER       Int32Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_int32array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_int_ir"
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
function test_int32array_set_int_ir(): void {
    let ar = new Int32Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar[2])
    arktest.assertEQ(2147483647, ar[4])
}

//! CHECKER       Int32Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Int32Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testInt32ArrayGetUnsafe"
//! METHOD        "escompat_Int32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Int32Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Int32Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testInt32ArrayGetUnsafe"
//! METHOD        "escompat_Int32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
function testInt32ArrayGetUnsafe(): void {
    let ar = new Int32Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar.at(2))
    arktest.assertEQ(2147483647, ar.at(4))
}

//! CHECKER       Int32Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int32array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_rangeerror_cpp"
function test_int32array_set_rangeerror_cpp(): void {
    let ar = new Int32Array(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       Int32Array $_set number/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_int32array_set_number_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_number_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_int32array_set_number_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArraySetInt "
//! INST          "Intrinsic.Int32ArrayGet "
function test_int32array_set_number_cpp(): void {
    let ar = new Int32Array(10)
    ar.fill(1)
    ar[2] = -Infinity
    ar[4] = Infinity
    ar[5] = NaN
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(0, ar[4])
    arktest.assertEQ(0, ar[5])
}

//! CHECKER       BigInt64Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_bigint64array_set_int_cpp"
//! METHOD        "escompat_BigInt64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_bigint64array_set_int_cpp(): void {
    let ar = new BigInt64Array(10)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648n, ar[2])
    arktest.assertEQ(2147483647n, ar[4])
}

//! CHECKER       BigInt64Array $_set long/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_bigint64array_set_long_cpp"
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
    arktest.assertEQ(9223372036854775807n, ar[2])
    arktest.assertEQ(2n, ar[3])
    arktest.assertEQ(-9223372036854775808n, ar[4])
    arktest.assertEQ(-2n, ar[5])
}

//! CHECKER       BigInt64Array $_set long/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_bigint64array_set_long_ir"
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
function test_bigint64array_set_long_ir(): void {
    let ar = new BigInt64Array(10)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    arktest.assertEQ(9223372036854775807n, ar[2])
    arktest.assertEQ(2n, ar[3])
    arktest.assertEQ(-9223372036854775808n, ar[4])
    arktest.assertEQ(-2n, ar[5])
}

//! CHECKER       BigInt64Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.BigInt64Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testBigInt64ArrayGetUnsafe"
//! METHOD        "escompat_BigInt64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigInt64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_bigint64array_set_rangeerror_cpp"
//! METHOD        "escompat_BigInt64Array::__set"
function test_bigint64array_set_rangeerror_cpp(): void {
    let ar = new BigInt64Array(10)
    arktest.expectThrow(() => { ar[11] = 1n }, expectedRangeErrors)
}

//! CHECKER       Float32Array $_set float/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float32array_set_float_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float32array_set_float_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float32array_set_float_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetFloat "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Float32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_float32array_set_float_cpp(): void {
    let ar = new Float32Array(10)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar[2])
    arktest.assertEQ(3.4028235e+38f, ar[4])
}

//! CHECKER       Float32Array $_set float/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_float32array_set_float_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float32array_set_float_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float32array_set_float_ir"
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
function test_float32array_set_float_ir(): void {
    let ar = new Float32Array(10)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar[2])
    arktest.assertEQ(3.4028235e+38f, ar[4])
}

//! CHECKER       Float32Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Float32Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testFloat32ArrayGetUnsafe"
//! METHOD        "escompat_Float32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Float32Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Float32Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testFloat32ArrayGetUnsafe"
//! METHOD        "escompat_Float32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
function testFloat32ArrayGetUnsafe(): void {
    let ar = new Float32Array(10)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar.at(2))
    arktest.assertEQ(3.4028235e+38f, ar.at(4))
}

//! CHECKER       Float32Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float32array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float32array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float32array_set_rangeerror_cpp"
function test_float32array_set_rangeerror_cpp(): void {
    let ar = new Float32Array(10)
    arktest.expectThrow(() => { ar[11] = 1.2f }, expectedRangeErrors)
}

//! CHECKER       Float64Array $_set double/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float64array_set_double_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float64array_set_double_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float64array_set_double_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetDouble "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.Float64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
function test_float64array_set_double_cpp(): void {
    let ar = new Float64Array(10)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar[2])
    arktest.assertEQ(3.4028235e+38, ar[4])
}

//! CHECKER       Float64Array $_set double/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_float64array_set_double_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float64array_set_double_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float64array_set_double_ir"
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
function test_float64array_set_double_ir(): void {
    let ar = new Float64Array(10)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar[2])
    arktest.assertEQ(3.4028235e+38, ar[4])
}

//! CHECKER       Float64Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Float64Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testFloat64ArrayGetUnsafe"
//! METHOD        "escompat_Float64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Float64Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Float64Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testFloat64ArrayGetUnsafe"
//! METHOD        "escompat_Float64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
function testFloat64ArrayGetUnsafe(): void {
    let ar = new Float64Array(10)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar.at(2))
    arktest.assertEQ(3.4028235e+38, ar.at(4))
}

//! CHECKER       Float64Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_float64array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float64array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_float64array_set_rangeerror_cpp"
function test_float64array_set_rangeerror_cpp(): void {
    let ar = new Float64Array(10)
    arktest.expectThrow(() => { ar[11] = 1.2 }, expectedRangeErrors)
}

//! CHECKER       AOT Uint8Array $_set int/$_get cpp intrinsic
//! RUN_PAOC      options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_cpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! RUN           entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_cpp"

//! CHECKER       Uint8Array $_set int/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_cpp"
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
    arktest.assertEQ(255, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(1, ar[4])
}

//! CHECKER       Uint8Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_rangeerror_cpp"
function test_uint8array_set_rangeerror_cpp(): void {
    let ar = new Uint8Array(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       AOT Uint8Array $_set int/$_get IR intrinsic
//! RUN_PAOC      options: "--compiler-regex=.*::test_uint8array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_ir"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! RUN           entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_ir"

//! CHECKER       Uint8Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint8array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8array_set_int_ir"
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
function test_uint8array_set_int_ir(): void {
    let ar = new Uint8Array(10)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(1, ar[4])
}

//! CHECKER       Uint8Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint8Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint8ArrayGetUnsafe"
//! METHOD        "escompat_Uint8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint8Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint8Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint8ArrayGetUnsafe"
//! METHOD        "escompat_Uint8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8clampedarray_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8clampedarray_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8clampedarray_set_int_cpp"
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
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(255, ar[4])
}

//! CHECKER       Uint8ClampedArray $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint8clampedarray_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8clampedarray_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8clampedarray_set_rangeerror_cpp"
function test_uint8clampedarray_set_rangeerror_cpp(): void {
    let ar = new Uint8ClampedArray(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       Uint8ClampedArray $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint8clampedarray_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8clampedarray_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint8clampedarray_set_int_ir"
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
function test_uint8clampedarray_set_int_ir(): void {
    let ar = new Uint8ClampedArray(10)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(255, ar[4])
}

//! CHECKER       Uint8ClampedArray getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint8ClampedArray::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint8ClampedArrayGetUnsafe"
//! METHOD        "escompat_Uint8ClampedArray::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint8ClampedArray getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint8ClampedArray::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint8ClampedArrayGetUnsafe"
//! METHOD        "escompat_Uint8ClampedArray::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ClampedArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint16array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint16array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint16array_set_int_cpp"
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
    arktest.assertEQ(65535, ar[2])
    arktest.assertEQ(65535, ar[3])
    arktest.assertEQ(1, ar[4])
}

//! CHECKER       Uint16Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint16array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint16array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint16array_set_rangeerror_cpp"
function test_uint16array_set_rangeerror_cpp(): void {
    let ar = new Uint16Array(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       Uint16Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint16array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint16array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint16array_set_int_ir"
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
function test_uint16array_set_int_ir(): void {
    let ar = new Uint16Array(10)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar[2])
    arktest.assertEQ(65535, ar[3])
    arktest.assertEQ(1, ar[4])
}

//! CHECKER       Uint16Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint16Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint16ArrayGetUnsafe"
//! METHOD        "escompat_Uint16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint16Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint16Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint16ArrayGetUnsafe"
//! METHOD        "escompat_Uint16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint32array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_int_cpp"
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
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(2147483647, ar[3])
}

//! CHECKER       Uint32Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint32array_set_rangeerror_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_rangeerror_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_rangeerror_cpp"
function test_uint32array_set_rangeerror_cpp(): void {
    let ar = new Uint32Array(10)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

//! CHECKER       Uint32Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint32array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_int_ir"
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
function test_uint32array_set_int_ir(): void {
    let ar = new Uint32Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(2147483647, ar[3])
}

//! CHECKER       Uint32Array $_set long/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=.*::test_uint32array_set_long_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_long_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_long_cpp"
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
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(4294967295, ar[3])
    arktest.assertEQ(1, ar[4])
}

//! CHECKER       Uint32Array $_set long/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_uint32array_set_long_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_long_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_uint32array_set_long_ir"
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
function test_uint32array_set_long_ir(): void {
    let ar = new Uint32Array(10)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(4294967295, ar[3])
    arktest.assertEQ(1, ar[4])
}

//! CHECKER       Uint32Array getUnsafe cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-regex=escompat\.Uint32Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint32ArrayGetUnsafe"
//! METHOD        "escompat_Uint32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       Uint32Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.Uint32Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testUint32ArrayGetUnsafe"
//! METHOD        "escompat_Uint32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*::test_biguint64array_set_int_cpp", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_int_cpp"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_int_cpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.BigUInt64ArrayGet "
function test_biguint64array_set_int_cpp(): void {
    let ar = new BigUint64Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(18446744073709551615n, ar[2])
    arktest.assertEQ(2147483647n, ar[3])
}

//! CHECKER       BigUint64Array $_set int/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_biguint64array_set_int_ir", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_int_ir"
//! METHOD        "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_int_ir"
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
function test_biguint64array_set_int_ir(): void {
    let ar = new BigUint64Array(10)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(18446744073709551615n, ar[2])
    arktest.assertEQ(2147483647n, ar[3])
}

//! CHECKER       BigUint64Array $_set long/$_get cpp intrinsic
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigUint64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_long_cpp"
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
    arktest.assertEQ(2n, ar[2])
    arktest.assertEQ(18446744073709551614n, ar[3])
    arktest.assertEQ(18446744073709551615n, ar[4])
    arktest.assertEQ(18446744073709551615n, ar[5])
}

//! CHECKER       BigUint64Array $_set cpp intrinsic rangeerror
//! RUN           force_jit: true, options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false --compiler-regex=.*BigUint64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_rangeerror_cpp"
//! METHOD        "escompat_BigUint64Array::__set"
function test_biguint64array_set_rangeerror_cpp(): void {
    let ar = new BigUint64Array(10)
    arktest.expectThrow(() => { ar[11] = 1n }, expectedRangeErrors)
}

//! CHECKER       BigUint64Array $_set long/$_get IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*BigUint64Array::.*", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::test_biguint64array_set_long_ir"
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
function test_biguint64array_set_long_ir(): void {
    let ar = new BigUint64Array(10)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar[2])
    arktest.assertEQ(18446744073709551614n, ar[3])
    arktest.assertEQ(18446744073709551615n, ar[4])
    arktest.assertEQ(18446744073709551615n, ar[5])
}

//! CHECKER       BigUint64Array getUnsafe IR intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=escompat\.BigUint64Array::at", entry: "ets_escompat_typed_arrays_set_get.ETSGLOBAL::testBigUint64ArrayGetUnsafe"
//! METHOD        "escompat_BigUint64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
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

// Helpers
const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError)
}
