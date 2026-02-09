/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

// Prevents comparison elimination by frontend type inferrence
function getfalse(): boolean {
 return false 
}
function getnull(): Any {
 return null 
}
function getundef(): Any {
 return undefined 
}
function id(x: Any) {
 return x 
}
function getobj(): Any {
 return {} as Object; 
}

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER      AOT ets.equals peepholes
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*(equals).*'"
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsDce"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsCse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /Intrinsic.CompilerEtsEquals/, 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   /Intrinsic.CompilerEtsEquals/, 1
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsSameInputs"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsEquals/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsNull"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsUndefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsNullUndefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsReftypedInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsUnionInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsEquals/
//!
//! METHOD       "ets_equals.ETSGLOBAL::equalsObjectInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsEquals/
//!
//! RUN          entry: "ets_equals.ETSGLOBAL::main"

function equalsDce(x: Any, y: Any) {
    return getfalse() && x == y;
}
function equalsCse(x: Any, y: Any) {
    return id(x) == id(y) && id(y) == id(x); // commutative
}
function equalsSameInputs(x: Any) {
    return id(x) == id(x);
}
function equalsNull(x: Any) {
    return getnull() == x;
}
function equalsUndefined(x: Any) {
    return x == getundef();
}
function equalsNullUndefined() {
    return getnull() == getundef();
}
function equalsReftypedInput() {
    return getobj() == getobj();
}
function equalsUnionInput(x: double | string) {
    return x == x;
}
function equalsObjectInput(x: Any) {
    return x == x;
}

function main() {
    let a = new Int(123);
    let b = new Int(123);

    arktest.assertTrue(!equalsDce(a, b));
    arktest.assertTrue(equalsCse(a, b));
    arktest.assertTrue(equalsSameInputs(a));
    arktest.assertTrue(equalsNull(null) && equalsNull(undefined) && !equalsNull(a));
    arktest.assertTrue(equalsUndefined(undefined) && equalsUndefined(null) && !equalsUndefined(a));
    arktest.assertTrue(equalsNullUndefined());
    arktest.assertTrue(!equalsReftypedInput());
    arktest.assertTrue(!equalsUnionInput(NaN));

    let objNaN: object = NaN;
    arktest.assertTrue(!equalsObjectInput(objNaN));
}
