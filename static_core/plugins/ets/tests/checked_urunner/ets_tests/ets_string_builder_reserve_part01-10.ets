/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       AOT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*[Rr]eserve[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::reserve1"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::reserve2"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopReserve3"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopReserve4"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=4)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=4)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopReserve5"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=5)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=5)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::nestedLoopReserve6"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=14)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=14)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::ifReserve7"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::ifReserve8"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=4)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=4)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::ifLoopReserve9"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=6)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=6)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopIfReserve10"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=9)"
//!

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*[Rr]eserve[0-9]*' --compiler-inlining=true",
//!               entry: "ets_string_builder_reserve_part01-10.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::reserve1"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::reserve2"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopReserve3"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopReserve4"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=4)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=4)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopReserve5"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=5)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=5)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::nestedLoopReserve6"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=14)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=14)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::ifReserve7"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::ifReserve8"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=4)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=4)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::ifLoopReserve9"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=6)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=6)"
//!
//! METHOD        "ets_string_builder_reserve_part01-10.ETSGLOBAL::loopIfReserve10"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=9)"

function reserve1() : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    return sb.toString()
}

function reserve2() : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    sb.append('1')
    return sb.toString()
}

function loopReserve3() : string {
    let sb = new StringBuilder()            // applied
    for (let i = 0; i < 3; ++i) {
        sb.append(i)
    }
    return sb.toString()
}

function loopReserve4() : string {
    let sb = new StringBuilder()            // applied
    sb.append('0')
    for (let i = 0; i < 3; ++i) {
        sb.append(i + 1)
    }
    return sb.toString()
}

function loopReserve5() : string {
    let sb = new StringBuilder()            // applied
    sb.append('0')
    for (let i = 0; i < 3; ++i) {
        sb.append(i + 1)
    }
    sb.append('4')
    return sb.toString()
}

function nestedLoopReserve6() : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    for (let i = 0; i < 3; ++i) {
        sb.append(i*4 + 1)
        for (let j = 0; j < 2; ++j) {
            sb.append(i*4 + j + 2)
        }
        sb.append(i*4 + 4)
    }
    sb.append(13)
    return sb.toString()
}

function ifReserve7(cond: boolean) : string {
    let sb = new StringBuilder()            // applied
    if (cond) {
        sb.append(0)
    }
    return sb.toString()
}

function ifReserve8(cond: boolean) : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    if (cond) {
        sb.append('1')
        sb.append(2)
    } else {
        sb.append('null')
    }
    sb.append('3')

    return sb.toString()
}

function ifLoopReserve9(cond: boolean) : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    if (cond) {
        for (let i = 0; i < 3; ++i) {
            sb.append(i + 1)
        }
        sb.append(4)
    } else {
        sb.append('null')
    }
    sb.append('5')

    return sb.toString()
}

function loopIfReserve10(cond: boolean) : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    for (let i = 0; i < 3; ++i) {
        if (cond) {
            sb.append(2*i + 1)
            sb.append(2*i + 2)
        } else {
            sb.append('null')
        }
    }
    sb.append(7)
    sb.append('8')

    return sb.toString()
}

function main() : int {
    arktest.assertEQ(reserve1(), '0', 'Wrong result at reserve1')
    arktest.assertEQ(reserve2(), '01', 'Wrong result at reserve2')
    arktest.assertEQ(loopReserve3(), '012', 'Wrong result at loopReserve3')
    arktest.assertEQ(loopReserve4(), '0123', 'Wrong result at loopReserve4')
    arktest.assertEQ(loopReserve5(), '01234', 'Wrong result at loopReserve5')
    arktest.assertEQ(nestedLoopReserve6(), '012345678910111213', 'Wrong result at nestedLoopReserve6')
    arktest.assertEQ(ifReserve7(true), '0', 'Wrong result at ifReserve7')
    arktest.assertEQ(ifReserve8(true), '0123', 'Wrong result at ifReserve8')
    arktest.assertEQ(ifLoopReserve9(true), '012345', 'Wrong result at ifLoopReserve9')
    arktest.assertEQ(loopIfReserve10(true), '012345678', 'Wrong result at loopIfReserve10')

    return 0
}
