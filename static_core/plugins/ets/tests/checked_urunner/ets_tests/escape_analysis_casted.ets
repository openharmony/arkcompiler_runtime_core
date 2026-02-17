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

//! CHECKER      Check escape analysis create cast in different sign types char
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true,
//!              options: "--compiler-regex=.*mainChar.*",
//!              entry: "escape_analysis_casted.ETSGLOBAL::mainChar"
//! EVENT        /Compilation,escape_analysis_casted.ETSGLOBAL::mainChar,.*,COMPILED/
//! METHOD       "escape_analysis_casted.ETSGLOBAL::mainChar"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "Cast", 12
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "Cast", 12
//! INST_COUNT   ".u16  Cast i8", 0

//! CHECKER      Check escape analysis create cast in different sign types char (ARM32)
//! SKIP_IF      @architecture != "arm32"
//! RUN          force_jit: true,
//!              options: "--compiler-regex=.*mainChar.*",
//!              entry: "escape_analysis_casted.ETSGLOBAL::mainChar"
//! EVENT        /Compilation,escape_analysis_casted.ETSGLOBAL::mainChar,.*,COMPILED/
//! METHOD       "escape_analysis_casted.ETSGLOBAL::mainChar"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "Cast", 12
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "Cast", 12
//! INST_COUNT   ".u16  Cast i8", 0

class A {
    fld: Char
    constructor(p: byte) {
        this.fld = new Char(p.toChar());
        console.log("a side-effect to break LSE")
    }
}

let s: byte = -128 as byte;

function mainChar(): int {
    let c: char = new A(s).fld;
    if (c != c'\uFF80') {
        return 1;
    }
    return 0;
}

// -----------------------------------

//! CHECKER      Check escape analysis create cast in different sign types cutted char
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true,
//!              options: "--compiler-regex=.*mainLong.*",
//!              entry: "escape_analysis_casted.ETSGLOBAL::mainLong"
//! EVENT        /Compilation,escape_analysis_casted.ETSGLOBAL::mainLong,.*,COMPILED/
//! METHOD       "escape_analysis_casted.ETSGLOBAL::mainLong"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "Cast", 13
//! INST_COUNT   ".u16  Cast i8", 0
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "Cast", 13
//! INST_COUNT   ".u16  Cast i8", 0

//! CHECKER      Check escape analysis create cast in different sign types cutted char (ARM32)
//! SKIP_IF      @architecture != "arm32"
//! RUN          force_jit: true,
//!              options: "--compiler-regex=.*mainLong.*",
//!              entry: "escape_analysis_casted.ETSGLOBAL::mainLong"
//! EVENT        /Compilation,escape_analysis_casted.ETSGLOBAL::mainLong,.*,COMPILED/
//! METHOD       "escape_analysis_casted.ETSGLOBAL::mainLong"
//! PASS_BEFORE  "EscapeAnalysis"
//! INST_COUNT   "Cast", 13
//! INST_COUNT   ".u16  Cast i8", 0
//! PASS_AFTER   "EscapeAnalysis"
//! INST_COUNT   "Cast", 13
//! INST_COUNT   ".u16  Cast i8", 0

class B {
    fld: Char
    constructor(p: int) {
        this.fld = p.toByte().toChar();
        console.log("a side-effect to break LSE")
    }
}
//           |will cut|  |will cut|
let b: int = (1 << 17) + (1 << 8) + 1;

function mainLong(): int {
    let c: char = new B(b).fld;
    if (c != c'\u0001') {
        return 1;
    }
    return 0;
}
