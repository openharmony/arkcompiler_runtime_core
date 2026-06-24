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

import { min as minChar } from 'std/math'

function testNoArguments() {
    arktest.assertEQ(Math.min(), Infinity)
}

function testSingleArgument() {
    arktest.assertEQ(Math.min(5), 5)
    arktest.assertEQ(Math.min(-5), -5)
    arktest.assertEQ(Math.min(0), 0)
    arktest.assertEQ(Math.min(Infinity), Infinity)
    arktest.assertEQ(Math.min(-Infinity), -Infinity)
    arktest.assertEQ(Math.min(3.14), 3.14)
    arktest.assertEQ(Math.min(-0), -0)
}

function testTwoArguments() {
    arktest.assertEQ(Math.min(1, 2), 1)
    arktest.assertEQ(Math.min(2, 1), 1)
    arktest.assertEQ(Math.min(-1, -2), -2)
    arktest.assertEQ(Math.min(-2, -1), -2)
    arktest.assertEQ(Math.min(0, 1), 0)
    arktest.assertEQ(Math.min(1, 0), 0)
}

function testMultipleArguments() {
    arktest.assertEQ(Math.min(1, 2, 3, 4, 5), 1)
    arktest.assertEQ(Math.min(5, 4, 3, 2, 1), 1)
    arktest.assertEQ(Math.min(3, 1, 4, 1, 5), 1)
    arktest.assertEQ(Math.min(-1, -5, -3, -2, -4), -5)
}

function testWithNaN() {
    arktest.assertTrue(Number.isNaN(Math.min(1, NaN, 3)))
    arktest.assertTrue(Number.isNaN(Math.min(NaN, 2)))
    arktest.assertTrue(Number.isNaN(Math.min(NaN)))
    arktest.assertTrue(Number.isNaN(Math.min(1, 2, NaN, 4)))
}

function testWithInfinity() {
    arktest.assertEQ(Math.min(Infinity, 1), 1)
    arktest.assertEQ(Math.min(1, Infinity), 1)
    arktest.assertEQ(Math.min(Infinity, Infinity), Infinity)
    arktest.assertEQ(Math.min(-Infinity, 1), -Infinity)
    arktest.assertEQ(Math.min(1, -Infinity), -Infinity)
    arktest.assertEQ(Math.min(Infinity, -Infinity), -Infinity)
}

function testWithZero() {
    arktest.assertEQ(Math.min(0, 0), 0)
    arktest.assertEQ(Math.min(0, -0), -0)
    arktest.assertEQ(Math.min(-0, 0), -0)
    arktest.assertEQ(Math.min(-0, -0), -0)
}

function testZeroSign() {
    // Math.min must return -0 (not +0) when args include both signs.
    arktest.assertNegativeZero(Math.min(0.0, -0.0))
    arktest.assertNegativeZero(Math.min(-0.0, 0.0))
    arktest.assertNegativeZero(Math.min(0.0, 0.0, -0.0))
    arktest.assertNegativeZero(Math.min(-0.0, 0.0, -0.0))
}

function testEqualNonZero() {
    // Equal non-zero values must return that value unchanged (not doubled).
    // Guards against treating tiny values as zero via an epsilon check:
    // if `val1 == 0.0` were replaced by `abs(val1) < epsilon`, these would break.
    arktest.assertEQ(Math.min(5.0, 5.0), 5.0)
    arktest.assertEQ(Math.min(5.0, 5.0, 5.0), 5.0)
    // Tiny but non-zero values must NOT be treated as zero
    arktest.assertEQ(Math.min(1e-300, 1e-300), 1e-300)
    arktest.assertEQ(Math.min(-1e-300, -1e-300), -1e-300)
    // Smallest positive subnormal must be preserved
    arktest.assertEQ(Math.min(Number.MIN_VALUE, Number.MIN_VALUE), Number.MIN_VALUE)
    arktest.assertEQ(Math.min(-Number.MIN_VALUE, -Number.MIN_VALUE), -Number.MIN_VALUE)
}

function testSameSignZero() {
    // Same-sign zero pairs must preserve that sign (the mixed-sign branch must not flip it).
    // 2-arg overload
    arktest.assertPositiveZero(Math.min(0.0, 0.0))
    arktest.assertNegativeZero(Math.min(-0.0, -0.0))
    // Variadic overload (3+ args route here)
    arktest.assertPositiveZero(Math.min(0.0, 0.0, 0.0))
    arktest.assertNegativeZero(Math.min(-0.0, -0.0, -0.0))
}

function testSingleArgZeroSign() {
    // Single-argument overload must return the argument's zero sign unchanged.
    arktest.assertNegativeZero(Math.min(-0.0))
    arktest.assertPositiveZero(Math.min(0.0))
}

function testTinyNonZeroMixedWithZero() {
    // Tiny non-zero values must NOT be treated as zero: strict < decides the result,
    // and the zero branch must not fire for them.
    // Tiny negative beats +0 -> returns the tiny value itself
    arktest.assertEQ(Math.min(0.0, -1e-300), -1e-300)
    arktest.assertEQ(Math.min(-1e-300, 0.0), -1e-300)
    arktest.assertEQ(Math.min(0.0, -Number.MIN_VALUE), -Number.MIN_VALUE)
    // Tiny negative beats both +0 and -0 in a 3-arg call
    arktest.assertEQ(Math.min(0.0, -Double.MIN_VALUE, -0.0), -Double.MIN_VALUE)
    // -0 beats tiny positive -> returns -0 (sign matters)
    arktest.assertNegativeZero(Math.min(-0.0, 1e-300))
    arktest.assertNegativeZero(Math.min(1e-300, -0.0))
}

function testWithFloats() {
    arktest.assertEQ(Math.min(1.5, 2.5), 1.5)
    arktest.assertEQ(Math.min(2.5, 1.5), 1.5)
    arktest.assertEQ(Math.min(1.1, 1.2, 1.3), 1.1)
    arktest.assertEQ(Math.min(-1.5, -2.5), -2.5)
    arktest.assertEQ(Math.min(0.1, 0.2, 0.3), 0.1)
}

function testNegativeAndPositive() {
    arktest.assertEQ(Math.min(-5, 5), -5)
    arktest.assertEQ(Math.min(5, -5), -5)
    arktest.assertEQ(Math.min(-10, 0, 10), -10)
    arktest.assertEQ(Math.min(10, 0, -10), -10)
}

function testLargeNumbers() {
    arktest.assertEQ(Math.min(1e100, 1e101), 1e100)
    arktest.assertEQ(Math.min(Number.MAX_VALUE, Number.MAX_VALUE - 1), Number.MAX_VALUE - 1)
    arktest.assertEQ(Math.min(-Number.MAX_VALUE, -Number.MAX_VALUE + 1), -Number.MAX_VALUE)
}

function testSmallNumbers() {
    arktest.assertEQ(Math.min(1e-100, 1e-101), 1e-101)
    arktest.assertEQ(Math.min(Number.MIN_VALUE, Number.MIN_VALUE * 2), Number.MIN_VALUE)
}

function testMixedValues() {
    arktest.assertEQ(Math.min(100, -50, 0, 25, -75), -75)
    arktest.assertEQ(Math.min(1.5, -2.7, 0.5, -0.5), -2.7)
    arktest.assertEQ(Math.min(1000, 1, 100, 10), 1)
}

function testCharOverload() {
    let a: char = c'a'
    let b: char = c'z'
    let c: char = c'A'
    let d: char = c'Z'

    arktest.assertEQ(minChar(a, b), a)
    arktest.assertEQ(minChar(b, a), a)
    arktest.assertEQ(minChar(c, d), c)
    arktest.assertEQ(minChar(a, c), c)  // 'A' < 'a' in ASCII
    arktest.assertEQ(minChar(c'0', c'9'), c'0')
}

function testIntOverload() {
    let a: int = 10
    let b: int = 20
    let c: int = -5
    let d: int = 0

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
    arktest.assertEQ(Math.min(Int.MAX_VALUE, Int.MIN_VALUE), Int.MIN_VALUE)
}

function testLongOverload() {
    let a: long = 100000000000
    let b: long = 200000000000
    let c: long = -50000000000
    let d: long = 0

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
    arktest.assertEQ(Math.min(Long.MAX_VALUE, Long.MIN_VALUE), Long.MIN_VALUE)
}

function testFloatOverload() {
    let a: float = 1.5f
    let b: float = 2.5f
    let c: float = -1.5f
    let d: float = 0.0f

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
}

function testDoubleOverload() {
    let a: double = 1.5
    let b: double = 2.5
    let c: double = -1.5
    let d: double = 0.0

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
}

function testNaNWithSpecial() {
    // NaN propagates regardless of the other operand
    arktest.assertTrue(Double.isNaN(Math.min(NaN, -0)))
    arktest.assertTrue(Double.isNaN(Math.min(-0, NaN)))
    arktest.assertTrue(Double.isNaN(Math.min(NaN, 0)))
    arktest.assertTrue(Double.isNaN(Math.min(NaN, NaN)))
    arktest.assertTrue(Double.isNaN(Math.min(NaN, Infinity)))
    arktest.assertTrue(Double.isNaN(Math.min(-Infinity, NaN)))
    arktest.assertTrue(Double.isNaN(Math.min(Infinity, -Infinity, NaN, 0)))
}

function testAllIdentical() {
    // All-same-value reduction must return that value unchanged
    arktest.assertEQ(Math.min(42, 42), 42)
    arktest.assertEQ(Math.min(42, 42, 42, 42), 42)
    arktest.assertEQ(Math.min(-7, -7, -7), -7)
    arktest.assertEQ(Math.min(3.14, 3.14, 3.14), 3.14)
}

function testSafeIntegerBounds() {
    arktest.assertEQ(Math.min(Double.MAX_SAFE_INTEGER, Double.MIN_SAFE_INTEGER), Double.MIN_SAFE_INTEGER)
    arktest.assertEQ(Math.min(Double.MIN_SAFE_INTEGER, Double.MAX_SAFE_INTEGER), Double.MIN_SAFE_INTEGER)
    arktest.assertEQ(Math.min(Double.MAX_SAFE_INTEGER, Double.MAX_SAFE_INTEGER), Double.MAX_SAFE_INTEGER)
    // Just beyond safe integer range
    arktest.assertEQ(Math.min(Double.MAX_SAFE_INTEGER + 1, Double.MAX_SAFE_INTEGER), Double.MAX_SAFE_INTEGER)
}

function testMathConstants() {
    arktest.assertEQ(Math.min(Math.PI, Math.E), Math.E)
    arktest.assertEQ(Math.min(Math.E, Math.PI), Math.E)
    arktest.assertEQ(Math.min(Math.LN2, Math.LN10), Math.LN2)
    arktest.assertEQ(Math.min(Math.SQRT2, Math.SQRT1_2), Math.SQRT1_2)
}

function testManyArguments() {
    // Stress the variadic reduction with 15 arguments
    arktest.assertEQ(Math.min(5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9), -9)
    arktest.assertEQ(Math.min(9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5), -5)
    // Descending then ascending — min is in the middle
    arktest.assertEQ(Math.min(10, 8, 6, 4, 2, 0, 2, 4, 6, 8, 10), 0)
}

function testNegativeZeroVariadicPosition() {
    // -0 in different positions of a variadic call must still win over +0 and positives
    arktest.assertNegativeZero(Math.min(1, -0, 2))
    arktest.assertNegativeZero(Math.min(2, 1, -0))
    arktest.assertNegativeZero(Math.min(-0, 1, 2))
    // -0 in the middle among several zeros
    arktest.assertNegativeZero(Math.min(0, 0, -0, 0))
    arktest.assertNegativeZero(Math.min(5, -0, 3, -0, 1))
}

function testMixedSpecialValues() {
    // Infinity beaten by any finite value
    arktest.assertEQ(Math.min(Infinity, -1e300, 1e300), -1e300)
    arktest.assertEQ(Math.min(1e300, -1e300, Infinity), -1e300)
    // -Infinity always wins (is the smallest)
    arktest.assertEQ(Math.min(-Infinity, Double.MAX_VALUE, Double.MIN_VALUE), -Infinity)
    // NaN poisons the entire variadic call
    arktest.assertTrue(Double.isNaN(Math.min(1, 2, Infinity, -Infinity, NaN)))
    arktest.assertTrue(Double.isNaN(Math.min(NaN, 1, 2, Infinity, -Infinity)))
}

function main() {
    const suite = new arktest.ArkTestsuite('Math.min tests')
    suite.addTest('Test No Arguments', testNoArguments)
    suite.addTest('Test Single Argument', testSingleArgument)
    suite.addTest('Test Two Arguments', testTwoArguments)
    suite.addTest('Test Multiple Arguments', testMultipleArguments)
    suite.addTest('Test With NaN', testWithNaN)
    suite.addTest('Test With Infinity', testWithInfinity)
    suite.addTest('Test With Zero', testWithZero)
    suite.addTest('Test Zero Sign', testZeroSign)
    suite.addTest('Test Equal Non Zero', testEqualNonZero)
    suite.addTest('Test Same Sign Zero', testSameSignZero)
    suite.addTest('Test Single Arg Zero Sign', testSingleArgZeroSign)
    suite.addTest('Test Tiny Non Zero Mixed With Zero', testTinyNonZeroMixedWithZero)
    suite.addTest('Test With Floats', testWithFloats)
    suite.addTest('Test Negative And Positive', testNegativeAndPositive)
    suite.addTest('Test Large Numbers', testLargeNumbers)
    suite.addTest('Test Small Numbers', testSmallNumbers)
    suite.addTest('Test Mixed Values', testMixedValues)
    suite.addTest('Test Char Overload', testCharOverload)
    suite.addTest('Test Int Overload', testIntOverload)
    suite.addTest('Test Long Overload', testLongOverload)
    suite.addTest('Test Float Overload', testFloatOverload)
    suite.addTest('Test Double Overload', testDoubleOverload)
    suite.addTest('Test NaN With Special', testNaNWithSpecial)
    suite.addTest('Test All Identical', testAllIdentical)
    suite.addTest('Test Safe Integer Bounds', testSafeIntegerBounds)
    suite.addTest('Test Math Constants', testMathConstants)
    suite.addTest('Test Many Arguments', testManyArguments)
    suite.addTest('Test Negative Zero Variadic Position', testNegativeZeroVariadicPosition)
    suite.addTest('Test Mixed Special Values', testMixedSpecialValues)
    return suite.run()
}
