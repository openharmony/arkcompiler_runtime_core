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

function testPositiveValues() {
    arktest.assertEQ(Math.floor(3.7), 3.0)
    arktest.assertEQ(Math.floor(3.0), 3.0)
    arktest.assertEQ(Math.floor(3.999), 3.0)
    arktest.assertEQ(Math.floor(0.999), 0.0)
    arktest.assertEQ(Math.floor(100.5), 100.0)
    arktest.assertEQ(Math.floor(1e10 + 0.5), 1e10)
}

function testNegativeValues() {
    arktest.assertEQ(Math.floor(-3.7), -4.0)
    arktest.assertEQ(Math.floor(-3.0), -3.0)
    arktest.assertEQ(Math.floor(-0.1), -1.0)
    arktest.assertEQ(Math.floor(-0.001), -1.0)
    arktest.assertEQ(Math.floor(-100.5), -101.0)
}

function testIntegerInputs() {
    arktest.assertEQ(Math.floor(0.0), 0.0)
    arktest.assertEQ(Math.floor(5.0), 5.0)
    arktest.assertEQ(Math.floor(-5.0), -5.0)
    arktest.assertEQ(Math.floor(1.0), 1.0)
    arktest.assertEQ(Math.floor(-1.0), -1.0)
}

function testZeroSign() {
    // floor(-0.0) must preserve negative zero sign
    arktest.assertNegativeZero(Math.floor(-0.0), 'floor(-0.0) => -0')

    // floor(+0.0) must preserve positive zero sign
    arktest.assertPositiveZero(Math.floor(0.0), 'floor(0.0) => +0')

    // floor of a tiny negative fraction is -1.0, not -0.0
    arktest.assertEQ(Math.floor(-Double.MIN_VALUE), -1.0)
}

function testSpecialValues() {
    arktest.assertTrue(isNaN(Math.floor(NaN)), 'floor(NaN) => NaN')
    arktest.assertEQ(Math.floor(Infinity), Infinity, 'floor(Infinity) => Infinity')
    arktest.assertEQ(Math.floor(-Infinity), -Infinity, 'floor(-Infinity) => -Infinity')
}

function testExtremeValues() {
    arktest.assertEQ(Math.floor(Double.MAX_VALUE), Double.MAX_VALUE)
    arktest.assertEQ(Math.floor(-Double.MAX_VALUE), -Double.MAX_VALUE)
    arktest.assertEQ(Math.floor(Double.MIN_VALUE), 0.0)
    arktest.assertEQ(Math.floor(-Double.MIN_VALUE), -1.0)
    arktest.assertEQ(Math.floor(Double.EPSILON), 0.0)
    arktest.assertEQ(Math.floor(-Double.EPSILON), -1.0)
}

function testLargeFractions() {
    arktest.assertEQ(Math.floor(4503599627370495.5), 4503599627370495.0)
    arktest.assertEQ(Math.floor(9007199254740991.0), 9007199254740991.0)
    arktest.assertEQ(Math.floor(1e308), 1e308)
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.floor tests')
    suite.addTest('Test Positive Values', testPositiveValues)
    suite.addTest('Test Negative Values', testNegativeValues)
    suite.addTest('Test Integer Inputs', testIntegerInputs)
    suite.addTest('Test Zero Sign Preservation', testZeroSign)
    suite.addTest('Test Special Values', testSpecialValues)
    suite.addTest('Test Extreme Values', testExtremeValues)
    suite.addTest('Test Large Fractions', testLargeFractions)
    return suite.run()
}
