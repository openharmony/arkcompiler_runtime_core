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

function testNormalRounding() {
    arktest.assertEQ(Math.round(2.3), 2.0)
    arktest.assertEQ(Math.round(2.7), 3.0)
    arktest.assertEQ(Math.round(-2.3), -2.0)
    arktest.assertEQ(Math.round(-2.7), -3.0)
    arktest.assertEQ(Math.round(0.0), 0.0)
    arktest.assertEQ(Math.round(1.0), 1.0)
    arktest.assertEQ(Math.round(-1.0), -1.0)
    arktest.assertEQ(Math.round(100.4), 100.0)
    arktest.assertEQ(Math.round(100.6), 101.0)
}

function testHalfRounding() {
    // Round half toward positive infinity (round half up)
    arktest.assertEQ(Math.round(0.5), 1.0)
    arktest.assertEQ(Math.round(1.5), 2.0)
    arktest.assertEQ(Math.round(2.5), 3.0)
    arktest.assertEQ(Math.round(-0.5), 0.0)
    arktest.assertEQ(Math.round(-1.5), -1.0)
    arktest.assertEQ(Math.round(-2.5), -2.0)
    arktest.assertEQ(Math.round(10.5), 11.0)
    arktest.assertEQ(Math.round(-10.5), -10.0)
}

function testZeroSign() {
    // round(-0.4) should produce -0.0
    arktest.assertNegativeZero(Math.round(-0.4), 'round(-0.4) => -0')

    // round(0.4) should produce +0.0
    arktest.assertPositiveZero(Math.round(0.4), 'round(0.4) => +0')

    arktest.assertNegativeZero(Math.round(-0.5), 'round(-0.5) => -0')

    // round(0.0) preserves +0
    arktest.assertPositiveZero(Math.round(0.0))

    // round(-0.0) preserves -0
    arktest.assertNegativeZero(Math.round(-0.0))
}

function testPrecisionBoundaries() {
    // Values just above and below 0.5
    arktest.assertEQ(Math.round(0.5 - Number.EPSILON / 4), 0.0)
    arktest.assertEQ(Math.round(0.5 + Number.EPSILON / 4), 1.0)
    arktest.assertEQ(Math.round(-0.5 - Number.EPSILON / 4), 0.0)
    arktest.assertEQ(Math.round(-0.5 + Number.EPSILON / 4), 0.0)

    // Very close to .5 from above
    arktest.assertEQ(Math.round(0.5000001), 1.0)
    arktest.assertEQ(Math.round(2.4999999), 2.0)

    // Large values where .5 matters
    arktest.assertEQ(Math.round(3.500000000000001), 4.0)
    arktest.assertEQ(Math.round(-3.500000000000001), -4.0)
}

function testSpecialValues() {
    arktest.assertTrue(isNaN(Math.round(NaN)), 'round(NaN) => NaN')
    arktest.assertEQ(Math.round(Infinity), Infinity, 'round(Infinity)')
    arktest.assertEQ(Math.round(-Infinity), -Infinity, 'round(-Infinity)')
}

function testExtremeValues() {
    arktest.assertEQ(Math.round(Double.MAX_VALUE), Double.MAX_VALUE)
    arktest.assertEQ(Math.round(-Double.MAX_VALUE), -Double.MAX_VALUE)
    arktest.assertEQ(Math.round(Double.MIN_VALUE), 0.0)
    arktest.assertEQ(Math.round(-Double.MIN_VALUE), 0.0)
    arktest.assertEQ(Math.round(Double.EPSILON), 0.0)
    arktest.assertEQ(Math.round(-Double.EPSILON), 0.0)
}

function testIntegerInputs() {
    arktest.assertEQ(Math.round(0.0), 0.0)
    arktest.assertEQ(Math.round(42.0), 42.0)
    arktest.assertEQ(Math.round(-42.0), -42.0)
    arktest.assertEQ(Math.round(1e15), 1e15)
    arktest.assertEQ(Math.round(-1e15), -1e15)
}

function testRoundTc39ComplianceProbe() {
    arktest.assertNegativeZero(Math.round(-0.5), 'round(-0.5) => -0 (TC39)')
    arktest.assertNegativeZero(Math.round(-0.4), 'round(-0.4) => -0 (TC39)')
    arktest.assertNegativeZero(Math.round(-Double.EPSILON), 'round(-EPS) => -0 (TC39)')

    // Negative inputs below -0.5 round toward +Infinity (not away from zero)
    arktest.assertEQ(Math.round(-1.5), -1.0, 'round(-1.5) => -1 (toward +Inf)')
    arktest.assertEQ(Math.round(-2.5), -2.0, 'round(-2.5) => -2 (toward +Inf)')

    // Positive 0.5 rounds up to 1
    arktest.assertEQ(Math.round(0.5), 1.0, 'round(0.5) => 1')

    // ±0 sign is preserved
    arktest.assertPositiveZero(Math.round(0.0), 'round(+0) => +0')
    arktest.assertNegativeZero(Math.round(-0.0), 'round(-0) => -0')
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.round tests')
    suite.addTest('Test Normal Rounding', testNormalRounding)
    suite.addTest('Test Half Rounding', testHalfRounding)
    // AOT Issue: #35165
    suite.addTest('Test Zero Sign Preservation', testZeroSign)
    suite.addTest('Test Precision Boundaries', testPrecisionBoundaries)
    suite.addTest('Test Special Values', testSpecialValues)
    suite.addTest('Test Extreme Values', testExtremeValues)
    suite.addTest('Test Integer Inputs', testIntegerInputs)
    // AOT Issue: #35165
    suite.addTest('Test TC39 Compliance Probe', testRoundTc39ComplianceProbe)
    return suite.run()
}
