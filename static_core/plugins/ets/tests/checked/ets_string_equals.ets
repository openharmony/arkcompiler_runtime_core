/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

// NOTE(vpukhov): #16340 Build specialized string comparison intrinsic

//! CHECKER      AOT IR Builder, check string-null equality
//! RUN_PAOC     options: "--compiler-regex='.*(equals_).*'"
//!
//! METHOD       "ets_string_equals.ETSGLOBAL::equals_str_str"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST         /Intrinsic.StdCoreStringEquals/
//!
//! METHOD       "ets_string_equals.ETSGLOBAL::equals_str_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic/
//!
//! METHOD       "ets_string_equals.ETSGLOBAL::not_equals_null_str"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic/
//!
//! METHOD       "ets_string_equals.ETSGLOBAL::not_equals_str_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic/
//!
//! RUN          entry: "ets_string_equals.ETSGLOBAL::main"

// Prevents comparison elimination by frontend type inferrence
function getnull(): String | null {
    return null;
}

function equals_str_str(lhs: String | null, rhs: String | null) {
    return lhs == rhs;
}
function equals_null_str(str: String | null) {
    return getnull() == str;
}
function not_equals_null_str(str: String | null) {
    return getnull() != str;
}
function equals_str_null(str: String | null) {
    return str == getnull();
}
function not_equals_str_null(str: String | null) {
    return str != getnull();
}

function main() {
    let str = "abc";

    assertTrue(equals_str_str(str, str));
    assertTrue(!equals_str_str(null, str));
    assertTrue(!equals_str_str(str, null));
    assertTrue(equals_str_str(null, null));

    assertTrue(!equals_null_str(str));
    assertTrue(not_equals_null_str(str));
    assertTrue(!equals_str_null(str));
    assertTrue(not_equals_str_null(str));

    assertTrue(equals_null_str(null));
    assertTrue(!not_equals_null_str(null));
    assertTrue(equals_str_null(null));
    assertTrue(!not_equals_str_null(null));
}
