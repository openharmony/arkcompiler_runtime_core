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

function testNaN() {
    arktest.assertTrue(isNaN(Math.sign(NaN)))
}

function testPositiveZero() {
    arktest.assertPositiveZero(Math.sign(0.0))
}

function testNegativeZero() {
    arktest.assertNegativeZero(Math.sign(-0.0))
}

function testPositive() {
    arktest.assertEQ(Math.sign(1), 1.0)
    arktest.assertEQ(Math.sign(0.5), 1.0)
    arktest.assertEQ(Math.sign(123.456), 1.0)
    arktest.assertEQ(Math.sign(Double.MAX_VALUE), 1.0)
    arktest.assertEQ(Math.sign(Double.MIN_VALUE), 1.0)
}

function testNegative() {
    arktest.assertEQ(Math.sign(-1), -1.0)
    arktest.assertEQ(Math.sign(-0.5), -1.0)
    arktest.assertEQ(Math.sign(-123.456), -1.0)
    arktest.assertEQ(Math.sign(-Double.MAX_VALUE), -1.0)
    arktest.assertEQ(Math.sign(-Double.MIN_VALUE), -1.0)
}

function testEpsilon() {
    arktest.assertEQ(Math.sign(Double.EPSILON), 1.0)
    arktest.assertEQ(Math.sign(-Double.EPSILON), -1.0)
    arktest.assertEQ(Math.sign(1e-16), 1.0)
    arktest.assertEQ(Math.sign(-1e-16), -1.0)
    arktest.assertEQ(Math.sign(1e-13), 1.0)
    arktest.assertEQ(Math.sign(1e-12), 1.0)
    arktest.assertEQ(Math.sign(-1e-12), -1.0)
}

function testInfinity() {
    arktest.assertEQ(Math.sign(Infinity), 1.0)
    arktest.assertEQ(Math.sign(-Infinity), -1.0)
}

function testNearEpsilonBoundary() {
    arktest.assertEQ(Math.sign(1e-14), 1.0, 'sign(1e-14) => 1 (no epsilon)')
    arktest.assertEQ(Math.sign(-1e-14), -1.0, 'sign(-1e-14) => -1 (no epsilon)')
    arktest.assertEQ(Math.sign(1e-13), 1.0, 'sign(1e-13) => 1')
    arktest.assertEQ(Math.sign(-1e-13), -1.0, 'sign(-1e-13) => -1')
}

function testLargeValues() {
    arktest.assertEQ(Math.sign(1e308), 1.0)
    arktest.assertEQ(Math.sign(-1e308), -1.0)
    arktest.assertEQ(Math.sign(Number.MAX_SAFE_INTEGER), 1.0)
    arktest.assertEQ(Math.sign(-Number.MAX_SAFE_INTEGER), -1.0)
    arktest.assertEQ(Math.sign(Number.MAX_SAFE_INTEGER + 1), 1.0)
}

function testIntegerInputs() {
    arktest.assertEQ(Math.sign(42), 1.0)
    arktest.assertEQ(Math.sign(-42), -1.0)
    arktest.assertEQ(Math.sign(1), 1.0)
    arktest.assertEQ(Math.sign(-1), -1.0)
    arktest.assertEQ(Math.sign(0), 0.0)
}

function testSignReturnType() {
    // Math.sign always returns double even for integer inputs
    let result: double = Math.sign(5)
    arktest.assertTrue(result == 1.0)
    // Verify it can be assigned to double and compared with double values
    arktest.assertEQ(result, 1.0)
    let negResult: double = Math.sign(-5)
    arktest.assertEQ(negResult, -1.0)
}

function testSignTc39ComplianceProbe() {

    // Tiny positive values (well below any plausible epsilon) return +1
    arktest.assertEQ(Math.sign(1e-14), 1.0, 'sign(1e-14) => 1 (TC39)')
    arktest.assertEQ(Math.sign(1e-300), 1.0, 'sign(1e-300) => 1 (TC39)')
    arktest.assertEQ(Math.sign(5e-324), 1.0, 'sign(5e-324 denorm-min) => 1 (TC39)')
    arktest.assertEQ(Math.sign(Double.EPSILON), 1.0, 'sign(Double.EPSILON) => 1 (TC39)')

    // Tiny negative values return -1
    arktest.assertEQ(Math.sign(-1e-14), -1.0, 'sign(-1e-14) => -1 (TC39)')
    arktest.assertEQ(Math.sign(-1e-300), -1.0, 'sign(-1e-300) => -1 (TC39)')
    arktest.assertEQ(Math.sign(-Double.EPSILON), -1.0, 'sign(-Double.EPSILON) => -1 (TC39)')
}

function testComputedSignedZero() {
    // Verify sign correctly handles -0 / +0 produced by arithmetic operations,
    // not just the literal -0.0. Variables prevent constant-folding from
    // collapsing signed zero, exercising the runtime path.
    let negOne: double = -1.0
    let zero: double = 0.0
    let one: double = 1.0

    // -0.0 from multiplication: negative × zero
    arktest.assertNegativeZero(Math.sign(negOne * zero), 'sign(-1.0 * 0.0) => -0')

    // -0.0 from multiplication: zero × negative
    arktest.assertNegativeZero(Math.sign(zero * negOne), 'sign(0.0 * -1.0) => -0')

    // -0.0 from unary negation of +0.0
    arktest.assertNegativeZero(Math.sign(-zero), 'sign(-(0.0)) => -0')

    // +0.0 from multiplication: positive × zero
    arktest.assertPositiveZero(Math.sign(one * zero), 'sign(1.0 * 0.0) => +0')

    // +0.0 from subtraction of equal values (x - x = +0 in round-to-nearest)
    arktest.assertPositiveZero(Math.sign(one - one), 'sign(1.0 - 1.0) => +0')

    // Integer 0 implicitly converts to +0.0; sign should preserve +0
    arktest.assertPositiveZero(Math.sign(0), 'sign(int 0) => +0')
}

function testNegativeDenormMin() {
    // Smallest negative denormalized double values; sign must be -1 (not 0).
    // Mirrors the positive denorm-min cases in testSignTc39ComplianceProbe.
    arktest.assertEQ(Math.sign(-5e-324), -1.0, 'sign(-5e-324 denorm-min) => -1')
    arktest.assertEQ(Math.sign(-1e-320), -1.0, 'sign(-1e-320) => -1')
    arktest.assertEQ(Math.sign(-1e-310), -1.0, 'sign(-1e-310) => -1')
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.sign tests')
    suite.addTest('Test NaN', testNaN)
    suite.addTest('Test Positive Zero', testPositiveZero)
    suite.addTest('Test Negative Zero', testNegativeZero)
    suite.addTest('Test Positive', testPositive)
    suite.addTest('Test Negative', testNegative)
    suite.addTest('Test Epsilon', testEpsilon)
    suite.addTest('Test Infinity', testInfinity)
    suite.addTest('Test Near Epsilon Boundary', testNearEpsilonBoundary)
    suite.addTest('Test Large Values', testLargeValues)
    suite.addTest('Test Integer Inputs', testIntegerInputs)
    suite.addTest('Test Sign Return Type', testSignReturnType)
    suite.addTest('Test TC39 Compliance Probe', testSignTc39ComplianceProbe)
    suite.addTest('Test Computed Signed Zero', testComputedSignedZero)
    suite.addTest('Test Negative Denorm Min', testNegativeDenormMin)
    return suite.run()
}
