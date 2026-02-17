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

//! CHECKER       JIT: indexOf() and lastIndexOf()
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex=.*::test.*array.*",
//!               entry: "ets_escompat_typed_arrays_index_of_part4.ETSGLOBAL::main"
//! METHOD        "testUint8arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::lastIndexOf/
//! METHOD        "testUint8clampedarrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::lastIndexOf/
//! METHOD        "testUint16arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::lastIndexOf/
//! METHOD        "testUint32arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::lastIndexOf/
//! METHOD        "testBiguint64arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::lastIndexOf/


//! CHECKER       AOT: indexOf() and lastIndexOf()
//! RUN_PAOC      options: "--compiler-regex=.*::test.*array.*",
//!               entry: "ets_escompat_typed_arrays_index_of_part4.ETSGLOBAL::main"
//! METHOD        "testUint8arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::lastIndexOf/
//! METHOD        "testUint8clampedarrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::lastIndexOf/
//! METHOD        "testUint16arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::lastIndexOf/
//! METHOD        "testUint32arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::lastIndexOf/
//! METHOD        "testBiguint64arrayLastIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::lastIndexOf/


function testUint8arrayLastIndexOf() {
    let array = new Uint8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -1),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -7), -1.0);
}

function testUint16arrayLastIndexOf() {
    let array = new Uint16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -1),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -7), -1.0);
}

function testUint32arrayLastIndexOf() {
    let array = new Uint32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -1),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -7), -1.0);
}

function testBiguint64arrayLastIndexOf() {
    let array = new BigUint64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.0);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.0);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  8),  4.0);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3), -1),  4.0);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3), -7), -1.0);
}

function testUint8clampedarrayLastIndexOf() {
    let array = new Uint8ClampedArray([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -1),  4.0);
    arktest.assertEQ(array.lastIndexOf(3.0, -7), -1.0);
}


function main(): void {
    testUint8arrayLastIndexOf()
    testUint8clampedarrayLastIndexOf()
    testUint16arrayLastIndexOf()
    testUint32arrayLastIndexOf()
    testBiguint64arrayLastIndexOf()
}
