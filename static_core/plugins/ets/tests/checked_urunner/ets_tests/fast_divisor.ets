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

//! CHECKER       Div/Mod by const optimization, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*(Div|Mod).*'", entry: "fast_divisor.ETSGLOBAL::main"
//! EVENT_NOT     /Compilation,fast_divisor.ETSGLOBAL::main,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intModByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longModByMin,.*,COMPILED/
//! METHOD        "fast_divisor.ETSGLOBAL::intDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::intDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::intDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::intModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::intModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::intModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     "AndI"
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::longDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::longDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::longDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::longModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::longModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::longModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     /And|AndI/
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"

//! CHECKER       Div/Mod by const optimization, AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(Div|Mod).*'"
//! EVENT_NOT     /Compilation,fast_divisor.ETSGLOBAL::main,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::intModByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::longModByMin,.*,COMPILED/
//! METHOD        "fast_divisor.ETSGLOBAL::intDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::intDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::intDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::intModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::intModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::intModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     "AndI"
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::intModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::longDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::longDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::longDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::longModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::longModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::longModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     /And|AndI/
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::longModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! RUN           entry: "fast_divisor.ETSGLOBAL::main"

// Int Div/Mod by 7
function intDivBy7(num: int): int {
    return num / 7;
}

function intModBy7(num: int): int {
    return num % 7;
}

// Long Div/Mod by 7
function longDivBy7(num: long): long {
    return num / 7;
}

function longModBy7(num: long): long {
    return num % 7;
}

// Int Div/Mod by (-7)
function intDivByM7(num: int): int {
    return num / (-7);
}

function intModByM7(num: int): int {
    return num % (-7);
}

// Long Div/Mod by (-7)
function longDivByM7(num: long): long {
    return num / (-7);
}

function longModByM7(num: long): long {
    return num % (-7);
}

// Int Div/Mod by Int.MIN_VALUE
function intDivByMin(num: int): int {
    arktest.assertEQ(Int.MIN_VALUE, -2147483648)
    return num / (-2147483648 as int);
}

function intModByMin(num: int): int {
    arktest.assertEQ(Int.MIN_VALUE, -2147483648)
    return num % (-2147483648 as int);
}

// Long Div/Mod by Long.MIN_VALUE
function longDivByMin(num: long): long {
    arktest.assertEQ(Long.MIN_VALUE, -9223372036854775808)
    return num / (-9223372036854775808 as long);
}

function longModByMin(num: long): long {
    arktest.assertEQ(Long.MIN_VALUE, -9223372036854775808)
    return num % (-9223372036854775808 as long);
}

function main(): int {
    arktest.assertEQ(intDivBy7(1405), 200)
    arktest.assertEQ(intModBy7(1405), 5)
    arktest.assertEQ(longDivBy7(1405), 200)
    arktest.assertEQ(longModBy7(1405), 5)

    arktest.assertEQ(intDivByM7(1405), -200)
    arktest.assertEQ(intModByM7(1405), 5)
    arktest.assertEQ(longDivByM7(1405), -200)
    arktest.assertEQ(longModByM7(1405), 5)

    arktest.assertEQ(intDivByMin(1405), 0)
    arktest.assertEQ(intModByMin(1405), 1405)
    arktest.assertEQ(longDivByMin(1405), 0)
    arktest.assertEQ(longModByMin(1405), 1405)

    arktest.assertEQ(intDivByMin(Int.MIN_VALUE), 1)
    arktest.assertEQ(intModByMin(Int.MIN_VALUE), 0)
    arktest.assertEQ(longDivByMin(Long.MIN_VALUE), 1)
    arktest.assertEQ(longModByMin(Long.MIN_VALUE), 0)

    return 0;
}
