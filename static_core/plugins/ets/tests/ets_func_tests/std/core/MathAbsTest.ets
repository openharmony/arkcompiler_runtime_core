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

function testDoublePositive() {
    arktest.assertEQ(Math.abs(3.7), 3.7)
    arktest.assertEQ(Math.abs(3.0), 3.0)
    arktest.assertEQ(Math.abs(0.5), 0.5)
    arktest.assertEQ(Math.abs(100.25), 100.25)
    arktest.assertEQ(Math.abs(Double.MAX_VALUE), Double.MAX_VALUE)
    arktest.assertEQ(Math.abs(Double.MIN_VALUE), Double.MIN_VALUE)
    arktest.assertEQ(Math.abs(Double.EPSILON), Double.EPSILON)
}

function testDoubleNegative() {
    arktest.assertEQ(Math.abs(-3.7), 3.7)
    arktest.assertEQ(Math.abs(-0.5), 0.5)
    arktest.assertEQ(Math.abs(-100.25), 100.25)
    arktest.assertEQ(Math.abs(-Double.MAX_VALUE), Double.MAX_VALUE)
    arktest.assertEQ(Math.abs(-Double.MIN_VALUE), Double.MIN_VALUE)
    arktest.assertEQ(Math.abs(-Double.EPSILON), Double.EPSILON)
}

function testDoubleZeroSign() {
    // abs(0.0) stays +0
    arktest.assertPositiveZero(Math.abs(0.0), 'abs(0.0) => +0')

    // abs(-0.0) must become +0 (IEEE 754: abs flips the sign bit)
    arktest.assertPositiveZero(Math.abs(-0.0), 'abs(-0.0) => +0')
}

function testDoubleSpecialValues() {
    arktest.assertTrue(isNaN(Math.abs(NaN)), 'abs(NaN) => NaN')
    arktest.assertEQ(Math.abs(Infinity), Infinity, 'abs(Infinity)')
    arktest.assertEQ(Math.abs(-Infinity), Infinity, 'abs(-Infinity) => Infinity')
}

function testIntAbs() {
    let a: int = 10
    let b: int = -10
    let c: int = 0
    let d: int = -2147483647

    arktest.assertEQ(Math.abs(a), 10)
    arktest.assertEQ(Math.abs(b), 10)
    arktest.assertEQ(Math.abs(c), 0)
    arktest.assertEQ(Math.abs(d), 2147483647)
    arktest.assertEQ(Math.abs(Int.MAX_VALUE), Int.MAX_VALUE)
}

function testLongAbs() {
    let a: long = 100000000000
    let b: long = -100000000000
    let c: long = 0

    arktest.assertEQ(Math.abs(a), 100000000000)
    arktest.assertEQ(Math.abs(b), 100000000000)
    arktest.assertEQ(Math.abs(c), 0)
    arktest.assertEQ(Math.abs(Long.MAX_VALUE), Long.MAX_VALUE)
}

function testFloatAbs() {
    let a: float = 3.5f
    let b: float = -3.5f
    let c: float = 0.0f
    let d: float = -0.0f

    arktest.assertEQ(Math.abs(a), 3.5f)
    arktest.assertEQ(Math.abs(b), 3.5f)
    arktest.assertEQ(Math.abs(c), 0.0f)
    // abs(-0.0f) should produce +0.0f
    arktest.assertPositiveZero(Math.abs(d), 'abs(-0.0f) => +0.0f')
}

function testByteAndShortAbs() {
    let sb: short = -100
    let bb: byte = 50

    arktest.assertEQ(Math.abs(sb), 100)
    arktest.assertEQ(Math.abs(bb), 50)
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.abs tests')
    suite.addTest('Test Double Positive', testDoublePositive)
    suite.addTest('Test Double Negative', testDoubleNegative)
    suite.addTest('Test Double Zero Sign', testDoubleZeroSign)
    suite.addTest('Test Double Special Values', testDoubleSpecialValues)
    suite.addTest('Test Int Abs', testIntAbs)
    suite.addTest('Test Long Abs', testLongAbs)
    suite.addTest('Test Float Abs', testFloatAbs)
    suite.addTest('Test Byte And Short Abs', testByteAndShortAbs)
    return suite.run()
}
