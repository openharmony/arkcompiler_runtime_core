/**
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

 /*---
desc: Number toFixed tests
---*/

function main(): int {
    const suite = new ArkTestsuite("Number API test for SDK");
    suite.addTest("number.toFixed(num), treatment of Scientific notation", testNumberToFixed);
    suite.addTest("number.toFixed(num), the parameter is too large", testLargeParameter);
    suite.addTest("number.toFixed(num), the parameter is negative", testNegativeParameter);
    return suite.run()
}

interface TestCase {
    numb: number,
    fractionDigits: number;
    expected: string
}

class TestData {
    static testCases: TestCase[] = [
        {
            numb: 1e21,
            fractionDigits: 3,
            expected: "1e+21"
        },
        {
            numb: 1e20,
            fractionDigits: 0,
            expected: "100000000000000000000"
        },
        {
            numb: 1e20/1e21,
            fractionDigits: 3,
            expected: "0.100"
        },
        {
            numb: 1e26,
            fractionDigits: 1,
            expected: "1e+26"
        },
        {
            numb: 1e-8,
            fractionDigits: 0,
            expected: "0"
        },
        {
            numb: -1e22,
            fractionDigits: 2,
            expected: "-1e+22"
        },
        {
            numb: -1e-6,
            fractionDigits: 0,
            expected: "-0"
        },
        {
            numb: 6e-32,
            fractionDigits: 1,
            expected: "0.0"
        },
        {
            numb: 9.999e20,
            fractionDigits: 0,
            expected: "999900000000000000000"
        },
    ]
    static a: Number = new Number('a');
    static big_digits: Number = Number.POSITIVE_INFINITY;
    static negative_digits: Number = -Number.POSITIVE_INFINITY;
}

function testNumberToFixed() {
    for (let i = 0; i < TestData.testCases.length; i++) {
        let testCase: TestCase = TestData.testCases[i];
        let num: number = testCase.numb;
        let digits: number = testCase.fractionDigits;
        let expected: string = testCase.expected;
        let actual: string = new Number(num).toFixed(digits);

        assertEQ(actual, expected, "failed: Expected" + expected + "got: " + actual);
    }
}

function testLargeParameter() {
    let a: Number = TestData.a;
    let big_digits: Number = TestData.big_digits;

    const errorcheck = (e: Error | Exception) => { return (e instanceof RangeError) }
    expectThrow((): void throws => { a.toFixed(big_digits) }, errorcheck)
}

function testNegativeParameter() {
    let a: Number = TestData.a;
    let negative_digits: Number = TestData.negative_digits;

    const errorcheck = (e: Error | Exception) => { return (e instanceof RangeError) }
    expectThrow((): void throws => { a.toFixed(negative_digits) }, errorcheck)
}
