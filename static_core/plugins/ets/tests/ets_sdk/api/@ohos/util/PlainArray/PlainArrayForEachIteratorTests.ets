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

import { PlainArray } from "@ohos.util.PlainArray";

function main(): int {
    const suite = new ArkTestsuite("PlainArray API tests");

    suite.addTest("PlainArray forEach with no parameter function", testForEachNoParameterFunction);
    suite.addTest("PlainArray forEach with function with value", testForEachWithOneParameter);
    suite.addTest("PlainArray forEach with function with value and index", testForEachWithTwoParameters);
    suite.addTest("PlainArray forEach with function with value, index and tempPlainArray",
        testForEachWithThreeParameters);
    suite.addTest("PlainArray forEach with empty container", testForEachWithEmpty);
    suite.addTest("PlainArray forEach with change the value", testForEachChangeValue);
    suite.addTest("PlainArray iterator with empty container", testIteratorWithEmpty);
    suite.addTest("PlainArray iterator with non-empty container", testIteratorWithNonEmpty);

    return suite.run();
}

class TestData {
    static readonly testNumber1: number = 1;
    static readonly testInt1: int = 1;
    static readonly testNumber6: number = 6;
    static readonly testString1: string = "0:0,2:2,4:4,5:5,8:8";
    static readonly forNumber1: Number[] = [0, 2, 4, 5, 8];
    static readonly forNumber2: Number[] = [0, 8, 2, 5, 4];
    static readonly forString1: String[] = ["0", "s", "88", ",", "4"];
}

function testForEachNoParameterFunction() {
    let plainArray = new PlainArray<number>();
    for (let i = 0; i < TestData.forNumber2.length; i++) {
        plainArray.add(TestData.forNumber2[i], TestData.forNumber2[i]);
    }
    let forEachCount: number = 0;
    plainArray.forEach(() => {
        forEachCount++;
    });
    assertEQ(forEachCount, plainArray.length, "ForEach count should match plainArray length");
}

function testForEachWithOneParameter() {
    let plainArray = new PlainArray<number>();
    for (let i = 0; i < TestData.forNumber2.length; i++) {
        plainArray.add(TestData.forNumber2[i], TestData.forNumber2[i]);
    }
    let forEachCount: number = 0;
    plainArray.forEach((value: number) => {
        assertEQ(value, plainArray.getValueAt(forEachCount++), "ForEach should iterate and modify elements correctly");
    });
    assertEQ(forEachCount, plainArray.length, "ForEach count should match plainArray length");
}

function testForEachWithTwoParameters() {
    let plainArray = new PlainArray<number>();
    for (let i = 0; i < TestData.forNumber2.length; i++) {
        plainArray.add(TestData.forNumber2[i] as int, TestData.forNumber2[i]);
    }
    let forEachCount: number = 0;
    plainArray.forEach((value: number, index: number) => {
        assertEQ(index, plainArray.getKeyAt(forEachCount), "ForEach should iterate and modify elements correctly");
        assertEQ(value, plainArray.getValueAt(forEachCount++), "ForEach should iterate and modify elements correctly");
    });
    assertEQ(forEachCount, plainArray.length, "ForEach count should match plainArray length");
}

function testForEachWithThreeParameters() {
    let plainArray = new PlainArray<number>();
    for (let i = 0; i < TestData.forNumber2.length; i++) {
        plainArray.add(TestData.forNumber2[i], TestData.forNumber2[i]);
    }
    let forEachCount: number = 0;
    plainArray.forEach((value: number, index: number, tempPlainArray: PlainArray<number>) => {
        assertEQ(plainArray, tempPlainArray, "TempPlainArray should be this");
        assertEQ(index, plainArray.getKeyAt(forEachCount), "ForEach should iterate and modify elements correctly");
        assertEQ(value, plainArray.getValueAt(forEachCount), "ForEach should iterate and modify elements correctly");
        assertEQ(tempPlainArray.getValueAt(forEachCount), plainArray.getValueAt(forEachCount),
            "The value at same index should be same");
        assertEQ(tempPlainArray.getKeyAt(forEachCount), plainArray.getKeyAt(forEachCount++),
            "The key at same index should be same");
    });
    assertEQ(forEachCount, plainArray.length, "ForEach count should match plainArray length");
}

function testForEachWithEmpty() {
    let plainArray = new PlainArray<number>();
    let forEachCount: number = TestData.testNumber1;
    plainArray.forEach(() => {
        forEachCount++;
    });
    assertEQ(forEachCount, TestData.testNumber1, "ForEach should not execute the callback function");
}

function testForEachChangeValue() {
    let plainArray = new PlainArray<number>();
    for (let i = 0; i < TestData.forNumber2.length; i++) {
        plainArray.add(TestData.forNumber2[i], TestData.forNumber2[i]);
    }
    plainArray.forEach((value: number) => {
        value = TestData.testNumber6;
    });
    assertEQ(plainArray.toString(), TestData.testString1, "The elements should not change");
}

function testIteratorWithEmpty() {
    let plainArray = new PlainArray<string>();
    let elem: number = 0;
    for (let item of plainArray) {
        elem = TestData.testNumber1;
    }
    let iter = plainArray.$_iterator();
    let temp: IteratorResult<[Number, string]> = iter.next();
    while (!temp.done) {
        elem = TestData.testNumber1;
    }
    assertEQ(elem, 0, "Iterator over empty PlainArray should not iterate any elements");
}

function testIteratorWithNonEmpty() {
    let plainArray = new PlainArray<string>();
    for (let i = 0; i < TestData.forNumber2.length; i++) {
        plainArray.add(TestData.forNumber2[i], TestData.forString1[i]);
    }
    let elem1: number = 0;
    for (let item of plainArray) {
        assertEQ(item[TestData.testInt1], plainArray.getValueAt(elem1), "The value should be same");
        assertEQ(item[0], plainArray.getKeyAt(elem1++), "The key should be same");
    }
    let iter = plainArray.$_iterator();
    let temp: IteratorResult<[Number, string]> = iter.next();
    let elem2: number = 0;
    while (!temp.done) {
        assertEQ(temp.value![TestData.testInt1], plainArray.getValueAt(elem2), "The value should be same");
        assertEQ(temp.value![0], plainArray.getKeyAt(elem2++), "The key should be same");
        temp = iter.next();
    }
    assertEQ(elem1, plainArray.length, "Iterator should iterate all elements correctly");
    assertEQ(elem2, plainArray.length, "Iterator should iterate all elements correctly");
}
