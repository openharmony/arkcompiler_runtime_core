/**
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

function testNoArguments() {
    arktest.assertEQ(Math.hypot(), 0)
}

function testSingleArgument() {
    arktest.assertEQ(Math.hypot(3), 3)
    arktest.assertEQ(Math.hypot(-4), 4)
    arktest.assertEQ(Math.hypot(0), 0)
    arktest.assertEQ(Math.hypot(Infinity), Infinity)
    arktest.assertTrue(Number.isNaN(Math.hypot(NaN)))
}

function testPythagorean() {
    arktest.assertEQ(Math.hypot(3, 4), 5)
    arktest.assertEQ(Math.hypot(5, 12), 13)
    arktest.assertEQ(Math.hypot(8, 15), 17)
    arktest.assertEQ(Math.hypot(6, 8), 10)
}

function testMultipleArguments() {
    // 2^2 + 3^2 + 6^2 = 49 -> 7
    arktest.assertEQ(Math.hypot(2, 3, 6), 7)
    // 1^2 + 2^2 + 3^2 = 14 -> sqrt(14) (irrational; use tolerance)
    arktest.assertTrue(Math.abs(Math.hypot(1, 2, 3) - Math.sqrt(14)) < 1e-12)
    arktest.assertEQ(Math.hypot(0, 0, 5), 5)
}

function testNegativeArguments() {
    // hypot uses absolute values, so sign of args does not change the result.
    arktest.assertEQ(Math.hypot(-3, 4), 5)
    arktest.assertEQ(Math.hypot(-3, -4), 5)
    arktest.assertEQ(Math.hypot(-5, -12), 13)
}

function testWithInfinity() {
    arktest.assertEQ(Math.hypot(Infinity, 0), Infinity)
    arktest.assertEQ(Math.hypot(-Infinity, 3), Infinity)
    arktest.assertEQ(Math.hypot(3, Infinity), Infinity)
    arktest.assertEQ(Math.hypot(Infinity, -Infinity), Infinity)
}

function testInfinityDominatesNaN() {
    // if any arg is +/-Infinity the result is +Infinity, even when NaN is present.
    arktest.assertEQ(Math.hypot(Infinity, NaN), Infinity)
    arktest.assertEQ(Math.hypot(NaN, Infinity), Infinity)
    arktest.assertEQ(Math.hypot(1, Infinity, NaN), Infinity)
}

function testWithNaN() {
    arktest.assertTrue(Number.isNaN(Math.hypot(NaN, 3)))
    arktest.assertTrue(Number.isNaN(Math.hypot(3, NaN)))
    arktest.assertTrue(Number.isNaN(Math.hypot(1, NaN, 2)))
}

function testWithZero() {
    arktest.assertEQ(Math.hypot(0, 0), 0)
    arktest.assertEQ(Math.hypot(0, -0), 0)
    arktest.assertEQ(Math.hypot(-0, -0), 0)
    arktest.assertEQ(Math.hypot(0, 3), 3)
}

function testZeroSign() {
    // hypot must return +0 (not -0) when all args are zero of any sign, and for no args.
    arktest.assertPositiveZero(Math.hypot(0, -0))
    arktest.assertPositiveZero(Math.hypot(-0, -0))
    arktest.assertPositiveZero(Math.hypot(0, 0, -0))
    arktest.assertPositiveZero(Math.hypot())
}

function testOverflowAvoidance() {
    let r: double = Math.hypot(1e154, 1e154)
    arktest.assertTrue(r < Infinity)
    // hypot(a, a) = a*sqrt(2) > a for positive a.
    arktest.assertTrue(r > 1e154)
}

function testUnderflowAvoidance() {
    let r: double = Math.hypot(1e-300, 1e-300)
    arktest.assertTrue(r > 0.0)
    arktest.assertTrue(r > 1e-300)
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.hypot tests')
    suite.addTest('Test No Arguments', testNoArguments)
    suite.addTest('Test Single Argument', testSingleArgument)
    suite.addTest('Test Pythagorean', testPythagorean)
    suite.addTest('Test Multiple Arguments', testMultipleArguments)
    suite.addTest('Test Negative Arguments', testNegativeArguments)
    suite.addTest('Test With Infinity', testWithInfinity)
    suite.addTest('Test Infinity Dominates NaN', testInfinityDominatesNaN)
    suite.addTest('Test With NaN', testWithNaN)
    suite.addTest('Test With Zero', testWithZero)
    suite.addTest('Test Zero Sign', testZeroSign)
    suite.addTest('Test Overflow Avoidance', testOverflowAvoidance)
    suite.addTest('Test Underflow Avoidance', testUnderflowAvoidance)
    return suite.run()
}
