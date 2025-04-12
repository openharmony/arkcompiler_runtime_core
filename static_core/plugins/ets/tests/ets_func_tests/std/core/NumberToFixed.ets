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
   suite.addTest("number.toFixed(num), the parameter is too large", testLargeParameter);
   suite.addTest("number.toFixed(num), the parameter is negative", testNegativeParameter);
   return suite.run()
}

class TestData {
    static a: Number = new Number('a');
    static big_digits: Number = Number.POSITIVE_INFINITY;
    static negative_digits: Number = -Number.POSITIVE_INFINITY;
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