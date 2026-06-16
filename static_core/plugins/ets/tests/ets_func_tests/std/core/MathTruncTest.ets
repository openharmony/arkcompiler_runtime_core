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
    // trunc rounds toward zero (not toward -Infinity like floor)
    arktest.assertEQ(Math.trunc(3.7), 3.0)
    arktest.assertEQ(Math.trunc(3.0), 3.0)
    arktest.assertEQ(Math.trunc(3.999), 3.0)
    arktest.assertEQ(Math.trunc(0.999), 0.0)
    arktest.assertEQ(Math.trunc(100.5), 100.0)
    arktest.assertEQ(Math.trunc(0.001), 0.0)
}

function testNegativeValues() {
    // Key difference from floor: trunc(-3.7) = -3.0, floor(-3.7) = -4.0
    arktest.assertEQ(Math.trunc(-3.7), -3.0)
    arktest.assertEQ(Math.trunc(-3.0), -3.0)
    arktest.assertEQ(Math.trunc(-3.999), -3.0)
    arktest.assertEQ(Math.trunc(-0.999), -0.0)
    arktest.assertEQ(Math.trunc(-100.5), -100.0)
    arktest.assertEQ(Math.trunc(-0.001), -0.0)
}

function testZeroSign() {
    // trunc(0.0) preserves +0
    arktest.assertPositiveZero(Math.trunc(0.0), 'trunc(0.0) => +0')

    // trunc(-0.0) preserves -0
    arktest.assertNegativeZero(Math.trunc(-0.0), 'trunc(-0.0) => -0')

    // trunc of a tiny positive fraction yields +0
    arktest.assertPositiveZero(Math.trunc(Double.MIN_VALUE), 'trunc(Double.MIN_VALUE) => +0')

    // trunc of a tiny negative fraction yields -0
    arktest.assertNegativeZero(Math.trunc(-Double.MIN_VALUE), 'trunc(-Double.MIN_VALUE) => -0')
}

function testIntegerInputs() {
    arktest.assertEQ(Math.trunc(5.0), 5.0)
    arktest.assertEQ(Math.trunc(-5.0), -5.0)
    arktest.assertEQ(Math.trunc(0.0), 0.0)
    arktest.assertEQ(Math.trunc(42.0), 42.0)
    arktest.assertEQ(Math.trunc(-42.0), -42.0)
}

function testSpecialValues() {
    arktest.assertTrue(isNaN(Math.trunc(NaN)), 'trunc(NaN) => NaN')
    arktest.assertEQ(Math.trunc(Infinity), Infinity, 'trunc(Infinity)')
    arktest.assertEQ(Math.trunc(-Infinity), -Infinity, 'trunc(-Infinity)')
}

function testExtremeValues() {
    arktest.assertEQ(Math.trunc(Double.MAX_VALUE), Double.MAX_VALUE)
    arktest.assertEQ(Math.trunc(-Double.MAX_VALUE), -Double.MAX_VALUE)
    arktest.assertEQ(Math.trunc(Double.MIN_VALUE), 0.0)
    arktest.assertEQ(Math.trunc(Double.EPSILON), 0.0)
    arktest.assertEQ(Math.trunc(1e308), 1e308)
}

function testContrastWithFloor() {
    // Explicit verification: trunc rounds toward zero, floor toward -Infinity
    arktest.assertEQ(Math.trunc(-3.7), -3.0, 'trunc(-3.7) = -3.0')
    arktest.assertEQ(Math.floor(-3.7), -4.0, 'floor(-3.7) = -4.0')

    arktest.assertEQ(Math.trunc(-0.5), -0.0, 'trunc(-0.5) = -0.0')
    arktest.assertEQ(Math.floor(-0.5), -1.0, 'floor(-0.5) = -1.0')

    arktest.assertEQ(Math.trunc(3.7), Math.floor(3.7))
    arktest.assertEQ(Math.trunc(0.5), Math.floor(0.5))
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.trunc tests')
    suite.addTest('Test Positive Values', testPositiveValues)
    suite.addTest('Test Negative Values', testNegativeValues)
    suite.addTest('Test Zero Sign Preservation', testZeroSign)
    suite.addTest('Test Integer Inputs', testIntegerInputs)
    suite.addTest('Test Special Values', testSpecialValues)
    suite.addTest('Test Extreme Values', testExtremeValues)
    suite.addTest('Test Contrast With Floor', testContrastWithFloor)
    return suite.run()
}
