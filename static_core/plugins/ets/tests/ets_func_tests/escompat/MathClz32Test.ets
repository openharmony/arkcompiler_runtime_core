/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    assertEQ(Math.clz32(NaN), 32)
}

function testInfinity() {
    assertEQ(Math.clz32(Infinity), 32)
    assertEQ(Math.clz32(-Infinity), 32)
}

function testZero() {
    assertEQ(Math.clz32(0), 32)
    assertEQ(Math.clz32(0.0), 32)
    assertEQ(Math.clz32(-0), 32)
    assertEQ(Math.clz32(-0.0), 32)
}

function testOne() {
    assertEQ(Math.clz32(1), 31)
    assertEQ(Math.clz32(-1), 0)
}

function testNegative() {
    assertEQ(Math.clz32(-333), 0)
    assertEQ(Math.clz32(-333.333), 0)
}

function testInUint32Range() {
    assertEQ(Math.clz32(123), 25)
    assertEQ(Math.clz32(123.456), 25)
}

function testLargeScaleNumber() {
    assertEQ(Math.clz32(1234567890), 1)
    assertEQ(Math.clz32(1.23e123), 32)
    assertEQ(Math.clz32(-1.23e123), 32)
    assertEQ(Math.clz32(Number.MAX_VALUE), 32)
    assertEQ(Math.clz32(-Number.MAX_VALUE), 32)
}

function testSmallScaleNumber() {
    assertEQ(Math.clz32(0.23), 32)
    assertEQ(Math.clz32(1.23), 31)
    assertEQ(Math.clz32(1.23e-123), 32)
    assertEQ(Math.clz32(-1.23e-123), 32)
    assertEQ(Math.clz32(Number.MIN_VALUE), 32)
    assertEQ(Math.clz32(-Number.MIN_VALUE), 32)

}

function testInt32CornerCase() {
    assertEQ(Math.clz32(2147483646), 1)
    assertEQ(Math.clz32(2147483647), 1)
    assertEQ(Math.clz32(2147483648), 0)

    assertEQ(Math.clz32(-2147483649), 1)
    assertEQ(Math.clz32(-2147483648), 0)
    assertEQ(Math.clz32(-2147483647), 0)
}

function testInt64CornerCase() {
    assertEQ(Math.clz32(9.223372036854775806e18), 32)
    assertEQ(Math.clz32(9.223372036854775807e18), 32)
    assertEQ(Math.clz32(9.223372036854775808e18), 32)

    assertEQ(Math.clz32(-9.223372036854775809e18), 32)
    assertEQ(Math.clz32(-9.223372036854775808e18), 32)
    assertEQ(Math.clz32(-9.223372036854775807e18), 32)
}

function testRandomCase() {
    assertEQ(Math.clz32(0.195767e-38), 32)
    assertEQ(Math.clz32(-0.624245e2), 0)
    assertEQ(Math.clz32(-0.949956e-2), 32)
    assertEQ(Math.clz32(0.878439e-18), 32)
    assertEQ(Math.clz32(-0.625398e15), 2)
    assertEQ(Math.clz32(-0.821849e-26), 32)
    assertEQ(Math.clz32(-0.180494e-20), 32)
    assertEQ(Math.clz32(-0.655483e-2), 32)
    assertEQ(Math.clz32(-0.365771e-6), 32)
    assertEQ(Math.clz32(0.995405e53), 32)
}

function main() {
    const suite = new ArkTestsuite("Math.clz32 tests")
    suite.addTest("Test NaN", testNaN)
    suite.addTest("Test Infinity", testInfinity)
    suite.addTest("Test Zero", testZero)
    suite.addTest("Test One", testOne)
    suite.addTest("Test Negative", testNegative)
    suite.addTest("Test In Uint32 Range", testInUint32Range)
    suite.addTest("Test Large Number", testLargeScaleNumber)
    suite.addTest("Test Small Number", testSmallScaleNumber)
    suite.addTest("Test Int32 Corner Case", testInt32CornerCase)
    suite.addTest("Test Int64 Corner Case", testInt64CornerCase)
    suite.addTest("Test Random Case", testRandomCase)
    return suite.run()
}
