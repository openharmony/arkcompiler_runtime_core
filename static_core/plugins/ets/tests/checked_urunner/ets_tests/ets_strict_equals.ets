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

type SomeRef = Object | null | undefined;

// Prevents comparison elimination by frontend type inferrence
function getfalse(): boolean {
 return false 
}
function getnull(): SomeRef {
 return null 
}
function getundef(): SomeRef {
 return undefined 
}
function id(x: SomeRef) {
 return x 
}
function getobj(): SomeRef {
 return {}; 
}

//! CHECKER      AOT ets.strictequals peepholes
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*(equals).*'"
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsDce"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsCse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /Intrinsic.CompilerEtsStrictEquals/, 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   /Intrinsic.CompilerEtsStrictEquals/, 1
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsSameInputs"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsNull"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsUndefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST         /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsNullUndefined"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsReftypedInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     /Intrinsic.CompilerEtsStrictEquals/
//! INST_NOT     /Compare/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsUnionInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//!
//! METHOD       "ets_strict_equals.ETSGLOBAL::strictEqualsObjectInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//! PASS_AFTER   "ChecksElimination"
//! INST         /Intrinsic.CompilerEtsStrictEquals/
//!
//! RUN          entry: "ets_strict_equals.ETSGLOBAL::main"

function strictEqualsDce(x: SomeRef, y: SomeRef) {
    return getfalse() && x === y;
}
function strictEqualsCse(x: SomeRef, y: SomeRef) {
    return id(x) === id(y) && id(y) === id(x); // commutative
}
function strictEqualsSameInputs(x: SomeRef) {
    return id(x) === id(x);
}
function strictEqualsNull(x: SomeRef) {
    return getnull() === x;
}
function strictEqualsUndefined(x: SomeRef) {
    return x === getundef();
}
function strictEqualsNullUndefined() {
    return getnull() === getundef();
}
function strictEqualsReftypedInput() {
    return getobj() === getobj();
}
function strictEqualsUnionInput(x: double | string) {
    return x === x;
}
function strictEqualsObjectInput(x: SomeRef) {
    return x === x;
}

function main() {
    let a = new Int(123);
    let b = new Int(123);

    arktest.assertTrue(!strictEqualsDce(a, b));
    arktest.assertTrue(strictEqualsCse(a, b));
    arktest.assertTrue(strictEqualsSameInputs(a));
    arktest.assertTrue(strictEqualsNull(null) && !strictEqualsNull(undefined) && !strictEqualsNull(a));
    arktest.assertTrue(strictEqualsUndefined(undefined) && !strictEqualsUndefined(null) && !strictEqualsUndefined(a));
    arktest.assertTrue(!strictEqualsNullUndefined());
    arktest.assertTrue(!strictEqualsReftypedInput());
    arktest.assertTrue(!strictEqualsUnionInput(NaN));

    let objNaN: object = NaN;
    arktest.assertTrue(!strictEqualsObjectInput(objNaN));
}
