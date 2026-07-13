/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER      INT keeps number/int/long/float/double logical-and value semantics
//! RUN          options: "--compiler-enable-jit=false",
//!              entry: "numeric_logical_and.ETSGLOBAL::main"

//! CHECKER      AOT keeps number/int/long/float/double logical-and value semantics
//! RUN_PAOC     options: "--compiler-regex='.*__noinline__.*LogicalAnd.*'"
//! RUN          entry: "numeric_logical_and.ETSGLOBAL::main"
//!
//! METHOD       "numeric_logical_and.ETSGLOBAL::__noinline__numberLogicalAnd"
//! PASS_BEFORE  "Peepholes"
//! INST         /And/
//! INST         /Compare EQ/
//! PASS_AFTER   "Peepholes"
//! INST         /Compare TST_EQ/
//!
//! METHOD       "numeric_logical_and.ETSGLOBAL::__noinline__intLogicalAnd"
//! PASS_BEFORE  "Peepholes"
//! INST         /Compare EQ/
//! PASS_AFTER   "Peepholes"
//! INST         /Compare EQ/
//!
//! METHOD       "numeric_logical_and.ETSGLOBAL::__noinline__longLogicalAnd"
//! PASS_BEFORE  "Peepholes"
//! INST         /Compare EQ/
//! PASS_AFTER   "Peepholes"
//! INST         /Compare EQ/
//!
//! METHOD       "numeric_logical_and.ETSGLOBAL::__noinline__floatLogicalAnd"
//! PASS_BEFORE  "Peepholes"
//! INST         /And/
//! INST         /Compare EQ/
//! PASS_AFTER   "Peepholes"
//! INST         /Compare TST_EQ/
//!
//! METHOD       "numeric_logical_and.ETSGLOBAL::__noinline__doubleLogicalAnd"
//! PASS_BEFORE  "Peepholes"
//! INST         /And/
//! INST         /Compare EQ/
//! PASS_AFTER   "Peepholes"
//! INST         /Compare TST_EQ/

function __noinline__numberLogicalAnd(a: number, b: number): number {
    return a && b;
}

function __noinline__intLogicalAnd(a: int, b: int): int {
    return a && b;
}

function __noinline__longLogicalAnd(a: long, b: long): long {
    return a && b;
}

function __noinline__floatLogicalAnd(a: float, b: float): float {
    return a && b;
}

function __noinline__doubleLogicalAnd(a: double, b: double): double {
    return a && b;
}

function main(): void {
    arktest.assertEQ(__noinline__numberLogicalAnd(5, 10), 10);
    arktest.assertEQ(__noinline__intLogicalAnd(5, 10), 10);
    arktest.assertEQ(__noinline__longLogicalAnd(5, 10), 10);
    arktest.assertEQ(__noinline__floatLogicalAnd(5.0f, 10.0f), 10.0f);
    arktest.assertEQ(__noinline__doubleLogicalAnd(5.0, 10.0), 10.0);
}
