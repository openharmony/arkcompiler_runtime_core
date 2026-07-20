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
//! RUN           force_jit: true,  options: "--compiler-regex=.*::test.*array.*",
//!               entry: "ets_escompat_typed_arrays_index_of.ETSGLOBAL::main"
//! METHOD        "testInt8arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int8Array::indexOf/
//! METHOD        "testInt16arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int16Array::indexOf/
//! METHOD        "testInt32arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int32Array::indexOf/
//! METHOD        "testBigint64arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigInt64Array::indexOf/
//!
//! METHOD        "testFloat32arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float32Array::indexOf/
//! METHOD        "testFloat64arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float64Array::indexOf/


//! CHECKER       AOT: indexOf() and lastIndexOf()
//! RUN_PAOC      options: "--compiler-regex=.*::test.*array.*",
//!               entry: "ets_escompat_typed_arrays_index_of.ETSGLOBAL::main"
//! METHOD        "testInt8arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int8Array::indexOf/
//! METHOD        "testInt16arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int16Array::indexOf/
//! METHOD        "testInt32arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int32Array::indexOf/
//! METHOD        "testBigint64arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigInt64Array::indexOf/
//!
//! METHOD        "testFloat32arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float32Array::indexOf/
//! METHOD        "testFloat64arrayIndexOf"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float64Array::indexOf/

function testInt8arrayIndexOf() {
    let array = new Int8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.0);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.0);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -1), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -7),  2.0);
}

function testInt16arrayIndexOf() {
    let array = new Int16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3 ,  0),  2.0);
    arktest.assertEQ(array.indexOf(3 ,  6), -1.0);
    arktest.assertEQ(array.indexOf(3.0,  8), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -1), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -7),  2.0);
}

function testInt32arrayIndexOf() {
    let array = new Int32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3.0,  0),  2.0);
    arktest.assertEQ(array.indexOf(3.0,  6), -1.0);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.0);
    arktest.assertEQ(array.indexOf(3 , -1), -1.0);
    arktest.assertEQ(array.indexOf(3 , -7),  2.0);
}

function testBigint64arrayIndexOf() {
    let array = new BigInt64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(new BigInt(3),  0),  2.0);
    arktest.assertEQ(array.indexOf(new BigInt(3),  6), -1.0);
    arktest.assertEQ(array.indexOf(3,  8), -1.0);
    arktest.assertEQ(array.indexOf(3, -1), -1.0);
    arktest.assertEQ(array.indexOf(3, -7),  2.0);
}

function testFloat32arrayIndexOf() {
    let array = new Float32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3.0,  0),  2.0);
    arktest.assertEQ(array.indexOf(3.0,  6), -1.0);
    arktest.assertEQ(array.indexOf(3 ,  8), -1.0);
    arktest.assertEQ(array.indexOf(3 , -1), -1.0);
    arktest.assertEQ(array.indexOf(3 , -7),  2.0);
}

function testFloat64arrayIndexOf() {
    let array = new Float64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.indexOf(3,   0),  2.0);
    arktest.assertEQ(array.indexOf(3,   6), -1.0);
    arktest.assertEQ(array.indexOf(3.0,  8), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -1), -1.0);
    arktest.assertEQ(array.indexOf(3.0, -7),  2.0);
}

function main(): void {
    testInt8arrayIndexOf()
    testInt16arrayIndexOf()
    testInt32arrayIndexOf()
    testBigint64arrayIndexOf()
    testFloat32arrayIndexOf()
    testFloat64arrayIndexOf()
}
