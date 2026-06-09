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

function main(): int {
    let mathTestsuite = new arktest.ArkTestsuite("mathTest")
    mathTestsuite.addTest("logTest01", logTest01)
    mathTestsuite.addTest("logTest02", logTest02)
    mathTestsuite.addTest("logTest03", logTest03)
    mathTestsuite.addTest("logTest04", logTest04)
    mathTestsuite.addTest("logTest05", logTest05)
    mathTestsuite.addTest("logTest06", logTest06)
    mathTestsuite.addTest("logTest07", logTest07)
    mathTestsuite.addTest("logTest08", logTest08)
    mathTestsuite.addTest("logTest09", logTest09)
    mathTestsuite.addTest("logTest10", logTest10)
    mathTestsuite.addTest("logTest11", logTest11)
    mathTestsuite.addTest("logTest12", logTest12)
    mathTestsuite.addTest("expmTest01", expmTest01)
    mathTestsuite.addTest("expmTest02", expmTest02)
    mathTestsuite.addTest("expmTest03", expmTest03)
    mathTestsuite.addTest("expmTest04", expmTest04)
    return mathTestsuite.run()
}


function logTest01() {
    const result: double = log1p(1.0);
    const expected: double = Math.log(2.0);
    arktest.assertEQ(result, expected);
}

function logTest02() {
    const result: double = log1p(10.0);
    const expected: double = Math.log(11.0);
    arktest.assertEQ(result, expected);
}

function logTest03() {
    const result: float = log1p(1.0f);
    const expected: float = Math.log(2.0f);
    arktest.assertEQ(result, expected);
}

function logTest04() {
    const result: float = log1p(10.0f);
    const expected: float = Math.log(11.0f);
    arktest.assertEQ(result, expected);
}

function logTest05() {
    const result: double = log1p(Number.MIN_VALUE);
    arktest.assertTrue(result > 0.0);
}

function logTest06() {
    const result: double = log1p(0.0);
    arktest.assertEQ(result, 0.0);
}

function logTest07() {
    const result: double = log1p(-1.0);
    arktest.assertEQ(result, -Infinity);
}

function logTest08() {
    const result: double = log1p(Number.EPSILON);
    arktest.assertTrue(result > 0.0);
}

function logTest09() {
    const result: float = log1p(Float.MIN_VALUE);
    arktest.assertTrue(result > 0.0f);
}

function logTest10() {
    const result: float = log1p(0.0f);
    arktest.assertEQ(result, 0.0f);
}

function logTest11() {
    const result: float = log1p(-1.0f);
    arktest.assertEQ(result, Float.NEGATIVE_INFINITY);
}

function logTest12() {
    const result: float = log1p(Float.EPSILON);
    arktest.assertTrue(result > 0.0f);
}

function expmTest01() {
    const result: double = expm1(1.0);
    const expected: double = Math.exp(1.0) - 1;
    arktest.assertEQ(result, expected);
}

function expmTest02() {
    const result: double = expm1(10.0);
    const expected: double = Math.exp(10.0) - 1;
    arktest.assertEQ(result, expected);
}

function expmTest03() {
    const result: float = expm1(1.0f);
    const expected: float = Math.exp(1.0f) - 1;
    arktest.assertEQ(result, expected);
}

function expmTest04() {
    const result: float = expm1(10.0f);
    const expected: float = Math.exp(10.0f) - 1;
    arktest.assertEQ(result, expected);
}