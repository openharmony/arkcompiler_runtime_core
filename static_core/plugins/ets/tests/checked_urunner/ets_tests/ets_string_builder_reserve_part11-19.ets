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
//! RUN_PAOC      options: "--compiler-regex='.*Reserve[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::uncountableLoopReserve11"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::uncountableLoopReserve12"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::uncountableLoopReserve13"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::switchReserve14"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=4)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=4)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::switchReserve15"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=10)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=10)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::reverseLoopReserve16"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::step2ReverseLoopReserve17"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::step2ReverseLoopReserve18"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::step5LoopReserve19"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=5)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=5)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*[Rr]eserve[0-9]*' --compiler-inlining=true",
//!               entry: "ets_string_builder_reserve_part11-19.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::uncountableLoopReserve11"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::uncountableLoopReserve12"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::uncountableLoopReserve13"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::switchReserve14"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=4)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=4)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::switchReserve15"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=10)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=10)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::reverseLoopReserve16"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::step2ReverseLoopReserve17"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::step2ReverseLoopReserve18"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part11-19.ETSGLOBAL::step5LoopReserve19"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=5)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=5)"

function uncountableLoopReserve11() : string {
    let sb = new StringBuilder()            // not applied, due to uncountable loop
    sb.append(0)
    for (let i:number = 1.0; i < 3.0; ++i) {
        sb.append(i)
    }
    return sb.toString()
}

function uncountableLoopReserve12() : string {
    let sb = new StringBuilder()            // not applied, due to uncountable loop
    let i = 0;
    sb.append(i++)
    while (sb.toString().getLength() < 3.0) {
        sb.append(i++)
    }
    return sb.toString()
}

function uncountableLoopReserve13() : string {
    let sb = new StringBuilder()            // not applied, due to uncountable loop
    let i = 0;
    sb.append(i++)
    do {
        sb.append(i++)
    } while (sb.toString().getLength() < 3.0)

    return sb.toString()
}

function switchReserve14(n : int) : string {
    let sb = new StringBuilder              // applied
    switch (n) {
      case 0:
        break;
      case 1:
            sb.append(0)
        break;
      case 2:
            sb.append('0')
            sb.append(1)
        break;
      case 3:
            sb.append('0')
            sb.append(1)
            sb.append(2.0)
        break;
      default:
            sb.append('0')
            sb.append(1)
            sb.append(2.0)
            sb.append(3)
        break;
    }
    return sb.toString()
}

function switchReserve15(n : int) : string {
    let sb = new StringBuilder              // applied
    switch (n) {
      case 0:
      case 1:
            sb.append(0)
      case 2:
            sb.append('0')
            sb.append(1)
      case 3:
            sb.append('0')
            sb.append(1)
            sb.append(2.0)
      default:
            sb.append('0')
            sb.append(1)
            sb.append(2.0)
            sb.append(3)
    }
    return sb.toString()
}

function reverseLoopReserve16() : string {
    let sb = new StringBuilder()            // applied
    for (let i = 2; i >= 0; --i) {
        sb.append(i)
    }
    return sb.toString()
}

function step2ReverseLoopReserve17() : string {
    let sb = new StringBuilder()            // applied
    for (let i = 4; i >= 0; i -= 2) {
        sb.append(i)
    }
    return sb.toString()
}

function step2ReverseLoopReserve18() : string {
    let sb = new StringBuilder()            // applied
    for (let i = 5; i >= 0; i -= 2) {
        sb.append(i)
    }
    return sb.toString()
}

function step5LoopReserve19() : string {
    let sb = new StringBuilder()            // applied
    for (let i = 2; i < 24; i += 5) {
        sb.append(i)
    }
    return sb.toString()
}

function main() : int {
    arktest.assertEQ(uncountableLoopReserve11(), '012', 'Wrong result at uncountableLoopReserve11')
    arktest.assertEQ(uncountableLoopReserve12(), '012', 'Wrong result at uncountableLoopReserve12')
    arktest.assertEQ(uncountableLoopReserve13(), '012', 'Wrong result at uncountableLoopReserve13')
    arktest.assertEQ(switchReserve14(3), '012', 'Wrong result at switchReserve14')
    arktest.assertEQ(switchReserve15(3), '0120123', 'Wrong result at switchReserve15')
    arktest.assertEQ(reverseLoopReserve16(), '210', 'Wrong result at reverseLoopReserve16')
    arktest.assertEQ(step2ReverseLoopReserve17(), '420', 'Wrong result at step2ReverseLoopReserve17')
    arktest.assertEQ(step2ReverseLoopReserve18(), '531', 'Wrong result at step2ReverseLoopReserve18')
    arktest.assertEQ(step5LoopReserve19(), '27121722', 'Wrong result at step5LoopReserve19')

    return 0
}
