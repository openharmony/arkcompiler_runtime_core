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

function main(): int {
    const suite = new ArkTestsuite("Array API tests for SDK");
    suite.addTest("Test reduceRight with array length reducing", testreduceRight);
    suite.addTest("Test reduceRight with array length increasing", testreduceRight1);
    return suite.run(); 
}

type FuncType = (prevVal: number, curVal: number, idx: number, obj: Array<number>) => number;

class TestData {
    static arr = new Array<number>(1, 2, 3, 4, 5);
    static readonly callbackfn: FuncType = (prevVal: number, curVal: number, idx: number, obj: Array<number>): number => {
        TestData.arr.length = 2;
        return prevVal + curVal;
    }
    static readonly expected = 12;
}

class TestData1 {
    static arr = new Array<number>(1, 2, 3, 4, 5);
    static readonly callbackfn: FuncType = (prevVal: number, curVal: number, idx: number, obj: Array<number>): number => {
        TestData1.arr.push(6,7,8);
        return prevVal + curVal;
    }
    static readonly expected = 15;
}

function testreduceRight() {
    TestData.arr = new Array<number>(1, 2, 3, 4, 5);
    let res = TestData.arr.reduceRight(TestData.callbackfn);
    assertEQ(TestData.expected, res, `Expected: "${TestData.expected}", Actual: "${res}"`);
}

function testreduceRight1() {
    TestData1.arr = new Array<number>(1, 2, 3, 4, 5); 
    let res = TestData1.arr.reduceRight(TestData1.callbackfn);
    assertEQ(TestData1.expected, res, `Expected: "${TestData1.expected}", Actual: "${res}"`);
}
