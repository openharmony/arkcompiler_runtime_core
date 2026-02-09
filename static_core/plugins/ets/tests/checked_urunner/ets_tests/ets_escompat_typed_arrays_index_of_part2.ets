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
//!               entry: "ets_escompat_typed_arrays_index_of_part2.ETSGLOBAL::main"
//! METHOD        "testUint8arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::indexOf/
//! METHOD        "testUint8clampedarrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::indexOf/
//! METHOD        "testUint16arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::indexOf/
//! METHOD        "testUint32arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::indexOf/
//! METHOD        "testBiguint64arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::indexOf/


//! CHECKER       AOT: indexOf() and lastIndexOf()
//! RUN_PAOC      options: "--compiler-regex=.*::test.*array.*",
//!               entry: "ets_escompat_typed_arrays_index_of_part2.ETSGLOBAL::main"
//! METHOD        "testUint8arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::indexOf/
//! METHOD        "testUint8clampedarrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::indexOf/
//! METHOD        "testUint16arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::indexOf/
//! METHOD        "testUint32arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::indexOf/
//! METHOD        "testBiguint64arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::indexOf/

function testUint8arrayIndexOf() {
    let array = new Uint8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.0);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.0);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -1), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -7),  2.0);
}

function testUint8clampedarrayIndexOf() {
    let array = new Uint8ClampedArray([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.0);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.0);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -1), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -7),  2.0);
}

function testUint16arrayIndexOf() {
    let array = new Uint16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.0);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.0);
    arktest.assertEQ(array.indexOf(3.0,  8), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -1), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -7),  2.0);
}

function testUint32arrayIndexOf() {
    let array = new Uint32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3.0,  0),  2.0);
    arktest.assertEQ(array.indexOf(3.0,  6), -1.0);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.0);
    arktest.assertEQ(array.indexOf(3 , -1), -1.0);
    arktest.assertEQ(array.indexOf(3 , -7),  2.0);
}

function testBiguint64arrayIndexOf() {
    let array = new BigUint64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(new BigInt(3),  0),  2.0);
    arktest.assertEQ(array.indexOf(new BigInt(3),  6), -1.0);
    arktest.assertEQ(array.indexOf(3,  8), -1.0);
    arktest.assertEQ(array.indexOf(3, -1), -1.0);
    arktest.assertEQ(array.indexOf(3, -7),  2.0);
}

function main(): void {
    testUint8arrayIndexOf()
    testUint8clampedarrayIndexOf()
    testUint16arrayIndexOf()
    testUint32arrayIndexOf()
    testBiguint64arrayIndexOf()
}
