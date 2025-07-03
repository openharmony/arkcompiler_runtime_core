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

//! CHECKER       Checking VN, LICM optimization for LoadObject for boxed types.
//! RUN           options: "--compiler-regex='.*test.*' ", entry: "optimize_load_object_boxed.ETSGLOBAL::main"
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testByte"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Byte/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Byte/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testChar"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Char/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Char/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testShort"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Short/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Short/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testInt"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Int/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Int/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testLong"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Long/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Long/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testFloat"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Float/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Float/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testNumber"
//! PASS_BEFORE   "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Double/, 0
//! PASS_AFTER    "LICM"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /LoadObject.*Double/, 1
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testByteVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Byte/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Byte/, 2
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testCharVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Char/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Char/, 2
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testShortVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Short/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Short/, 2
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testIntVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Int/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Int/, 2
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testLongVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Long/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Long/, 2
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testFloatVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Float/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Float/, 2
//!
//! METHOD        "optimize_load_object_boxed.ETSGLOBAL::testNumberVN"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject.*Double/, 8
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject.*Double/, 2

function testByte(input: Byte): byte {
    let result: byte = 0 as byte;

    for (let i = 0; i < 5000; i++) {
        result += Byte.valueOf(input);
    }

    return result;
}

function testChar(input: Char): char {
    let result: char = c'A';

    for (let i = 0; i < 5000; i++) {
        result += Char.valueOf(input);
    }

    return result;
}

function testShort(input: Short): short {
    let result: short = 0 as short;

    for (let i = 0; i < 5000; i++) {
        result += Short.valueOf(input);
    }

    return result;
}

function testInt(input: Int): int {
    let result: int = 0;

    for (let i = 0; i < 5000; i++) {
        result += Int.valueOf(input);
    }

    return result;
}

function testLong(input: Long): long {
    let result: long = 0;

    for (let i = 0; i < 5000; i++) {
        result += Long.valueOf(input);
    }

    return result;
}

function testFloat(input: Float): float {
    let result: float = 0 as float;

    for (let i = 0; i < 5000; i++) {
        result += Float.valueOf(input);
    }

    return result;
}

function testNumber(input: Number): number {
    let result: number = 0;

    for (let i = 0; i < 5000; i++) {
        result += Number.valueOf(input);
    }

    return result;
}

function __noinline__dummy(o: Object) {
}

function __noinline__dummyReturn(): Int {
    return 1;
}

function testByteVN(a: Byte, b: Byte): byte {
    let x: byte = Byte.valueOf(a) + Byte.valueOf(b) as byte;
    __noinline__dummy(x);
    let y: byte = Byte.valueOf(a) + Byte.valueOf(b) as byte;
    __noinline__dummy(y);
    let z: byte = Byte.valueOf(b) + Byte.valueOf(a) as byte;
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z as byte;
}

function testCharVN(a: Char, b: Char): char {
    let x: char = Char.valueOf(a) + Char.valueOf(b) as char;
    __noinline__dummy(x);
    let y: char = Char.valueOf(a) + Char.valueOf(b) as char;
    __noinline__dummy(y);
    let z: char = Char.valueOf(b) + Char.valueOf(a) as char;
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z as char;
}

function testShortVN(a: Short, b: Short): short {
    let x: short = Short.valueOf(a) + Short.valueOf(b) as short;
    __noinline__dummy(x);
    let y: short = Short.valueOf(a) + Short.valueOf(b) as short;
    __noinline__dummy(y);
    let z: short = Short.valueOf(b) + Short.valueOf(a) as short;
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z as short;
}

function testIntVN(a: Int, b: Int): int {
    let x: int = Int.valueOf(a) + Int.valueOf(b);
    __noinline__dummy(x);
    let y: int = Int.valueOf(a) + Int.valueOf(b);
    __noinline__dummy(y);
    let z: int = Int.valueOf(b) + Int.valueOf(a);
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z;
}

function testLongVN(a: Long, b: Long): long {
    let x: long = Long.valueOf(a) + Long.valueOf(b);
    __noinline__dummy(x);
    let y: long = Long.valueOf(a) + Long.valueOf(b);
    __noinline__dummy(y);
    let z: long = Long.valueOf(b) + Long.valueOf(a);
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z;
}

function testFloatVN(a: Float, b: Float): float {
    let x: float = Float.valueOf(a) + Float.valueOf(b);
    __noinline__dummy(x);
    let y: float = Float.valueOf(a) + Float.valueOf(b);
    __noinline__dummy(y);
    let z: float = Float.valueOf(b) + Float.valueOf(a);
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z as float;
}

function testNumberVN(a: Number, b: Number): number {
    let x: number = Number.valueOf(a) + Number.valueOf(b);
    __noinline__dummy(x);
    let y: number = Number.valueOf(a) + Number.valueOf(b);
    __noinline__dummy(y);
    let z: number = Number.valueOf(b) + Number.valueOf(a);
    __noinline__dummy(z);

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return x + y + z;
}

function main(): int {
    let valByte: Byte = new Byte(1 as byte);
    let resultByte: byte = testByte(valByte);

    let valChar: Char = new Char(c'A');
    let resultChar: char = testChar(valChar);

    let valShort: Short = new Short(42 as short);
    let resultShort: short = testShort(valShort);

    let valInt: Int = new Int(42);
    let resultInt: int = testInt(valInt);

    let valLong: Long = new Long(42);
    let resultLong: long = testLong(valLong);

    let valFloat: Float = new Float(42 as float);
    let resultFloat: float = testFloat(valFloat);

    let valNumber: Number = new Number(42);
    let resultNumber: number = testNumber(valNumber);

    let inputByteA: Byte = new Byte(1 as byte);
    let inputByteB: Byte = new Byte(2 as byte);
    resultByte = testByteVN(inputByteA, inputByteB);

    let inputCharA: Char = new Char(c'A');
    let inputCharB: Char = new Char(c'B');
    resultChar = testCharVN(inputCharA, inputCharB);

    let inputShortA: Short = new Short(42 as short);
    let inputShortB: Short = new Short(43 as short);
    resultShort = testShortVN(inputShortA, inputShortB);

    let inputIntA: Int = new Int(42);
    let inputIntB: Int = new Int(43);
    resultInt = testIntVN(inputIntA, inputIntB);

    let inputLongA: Long = new Long(42);
    let inputLongB: Long = new Long(43);
    resultLong = testLongVN(inputLongA, inputLongB);

    let inputFloatA: Float = new Float(42 as float);
    let inputFloatB: Float = new Float(43 as float);
    resultFloat = testFloatVN(inputFloatA, inputFloatB);

    let inputNumberA: Number = new Number(42);
    let inputNumberB: Number = new Number(43);
    resultNumber = testNumberVN(inputNumberA, inputNumberB);

    return 0;
}
