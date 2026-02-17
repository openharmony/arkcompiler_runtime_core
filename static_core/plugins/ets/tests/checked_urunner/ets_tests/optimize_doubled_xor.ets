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

/* Run this test manually:
cd runtime_core/static_core/build
ninja optimize_doubled_xor.checked
*/

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--opt-level=0"

//! CHECKER       AOT: Checking xor->xor optimization
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*test.*' ", entry: "optimize_doubled_xor.ETSGLOBAL::main"
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainOptimizationConst1"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainOptimizationConst2"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainOptimizationConst3"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainByte"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainShort"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainInt"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainLong"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainNoOptimizationDiffConst"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainNoOptimizationMultiUsers"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//!

//! CHECKER       JIT: Checking xor->xor optimization
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*' ", entry: "optimize_doubled_xor.ETSGLOBAL::main"
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainOptimizationConst1"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainOptimizationConst2"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainOptimizationConst3"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainByte"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainShort"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainInt"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainLong"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    /Xor/,0
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainNoOptimizationDiffConst"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! METHOD        "optimize_doubled_xor.ETSGLOBAL::testXorChainNoOptimizationMultiUsers"
//! PASS_BEFORE   "Peepholes"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!
//! PASS_AFTER    "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST          /Xor/
//! INST_NEXT     /Xor/
//! INST_COUNT    /Xor/,2
//!

function __noinline__dummyReturn(): Int {
    return 1;
}

function testXorChainOptimizationConst1(param: int): int {
    const XOR_CONST: int = 1;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result;
}

function testXorChainOptimizationConst2(param: int): int {
    const XOR_CONST: int = 0x12345678;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result;
}

function testXorChainOptimizationConst3(param: int): int {
    const XOR_CONST: int = -1;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result;
}

function testXorChainNoOptimizationDiffConst(param: int): int {
    const XOR_CONST1: int = 0x1234567;
    const XOR_CONST2: int = 0x7654321;
    // Should NOT be optimized - different constants
    let result = (param ^ XOR_CONST1) ^ XOR_CONST2;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result;
}

function testXorChainNoOptimizationMultiUsers(param: int): int {
    const XOR_CONST: int = 0x12345678;
    let temp = param ^ XOR_CONST;
    // Should NOT be optimized - temp has multiple users
    let result1 = temp ^ XOR_CONST;
    let result2 = temp + 1;  // Additional user prevents optimization

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result1 + result2;
}

function testXorChainByte(param: byte): byte {
    const XOR_CONST: byte = 0x12 as byte;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result.toByte();
}

function testXorChainShort(param: short): short {
    const XOR_CONST: short = 0x1234 as short;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result.toShort();
}

function testXorChainInt(param: int): int {
    const XOR_CONST: int = 0x12345678;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result;
}

function testXorChainLong(param: long): long {
    const XOR_CONST: long = 0x123456789ABCDEF0 as long;
    // should be optimized to just 'param' since (param ^ XOR_CONST) ^ XOR_CONST == param
    let result = (param ^ XOR_CONST) ^ XOR_CONST;

    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }

    return result;
}
 
function main() {
    let param: int = 42;
    arktest.assertEQ(testXorChainOptimizationConst1(param), param);
    arktest.assertEQ(testXorChainOptimizationConst2(param), param);
    arktest.assertEQ(testXorChainOptimizationConst3(param), param);
    arktest.assertEQ(testXorChainNoOptimizationDiffConst(param), param ^ 0x1234567 ^ 0x7654321);
    arktest.assertEQ(testXorChainNoOptimizationMultiUsers(param),
                     (param ^ 0x12345678 ^ 0x12345678) + ((param ^ 0x12345678) + 1));
    let byteParam: byte = 42 as byte;
    arktest.assertEQ(testXorChainByte(byteParam), byteParam);
    let shortParam: short = 4242 as short;
    arktest.assertEQ(testXorChainShort(shortParam), shortParam);
    let intParam: int = 424242 as int;
    arktest.assertEQ(testXorChainInt(intParam), intParam);
    let longParam: long = 424242424242 as long;
    arktest.assertEQ(testXorChainLong(longParam), longParam);

    return 0;
}
