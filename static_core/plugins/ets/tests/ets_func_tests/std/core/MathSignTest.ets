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
    let result = Math.sign(0.0)
    arktest.assertEQ(result, 0.0)
    arktest.assertEQ(1.0 / result, Infinity)
}

function testNegativeZero() {
    let result = Math.sign(-0.0)
    arktest.assertEQ(result, 0.0)
    arktest.assertEQ(1.0 / result, -Infinity)
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

function main() {
    const suite = new arktest.ArkTestsuite('Math.sign tests')
    suite.addTest('Test NaN', testNaN)
    suite.addTest('Test Positive Zero', testPositiveZero)
    suite.addTest('Test Negative Zero', testNegativeZero)
    suite.addTest('Test Positive', testPositive)
    suite.addTest('Test Negative', testNegative)
    suite.addTest('Test Epsilon', testEpsilon)
    suite.addTest('Test Infinity', testInfinity)
    return suite.run()
}
