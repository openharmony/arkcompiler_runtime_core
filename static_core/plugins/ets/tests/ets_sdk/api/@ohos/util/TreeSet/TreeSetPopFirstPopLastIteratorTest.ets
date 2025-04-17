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

import {TreeSet} from "@ohos.util.TreeSet";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeSet PopFirst PopLast Iterator API tests")

    suite.addTest("TreeSet popFirst popLast first and last value",
        testPopFirstPopLastValidValues)
    suite.addTest("TreeSet popFirst popLast first and last value from the null treeSet",
        testPopFirstPopLastEmptyTreeSet)
    suite.addTest("TreeSet popFirst popLast first and last value from the TreeSet with a set of data",
        testPopFirstPopLastMultipleValues)
    suite.addTest("TreeSet popFirst popLast first and last value by descending",
        testPopFirstPopLastDescendingOrder)
    suite.addTest("TreeSet popFirst popLast first and last value after clear data",
        testPopFirstPopLastAfterClear)
    suite.addTest("TreeSet $_iterator return iterator",
        testIterator)
    suite.addTest("TreeSet $_iterator return iterator with null treeSet",
        testIteratorEmptyTreeSet)

    return suite.run()
}

function testPopFirstPopLastValidValues() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    treeSet.add("cc");
    treeSet.add("dd")
    let result_popFirst = treeSet.popFirst();
    let result_has_first = treeSet.has("aa");
    let result_popLast = treeSet.popLast();
    let result_has_last = treeSet.has("dd");
    assertEQ(result_popFirst, "aa", "The treeSet should pop the first value");
    assertEQ(result_has_first, false, "The treeSet should not contain the popped first value");
    assertEQ(result_popLast, "dd", "The treeSet should pop the last value");
    assertEQ(result_has_last, false, "The treeSet should not contain the popped last value");
    assertEQ(treeSet.length, 2, "The treeSet length should be 1");
}

function testPopFirstPopLastEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let result_popFirst = treeSet.popFirst();
    let result_popLast = treeSet.popLast();
    assertEQ(result_popFirst, undefined, "The treeSet should return undefined for popping first value in an empty set");
    assertEQ(result_popLast, undefined, "The treeSet should return undefined for popping last value in an empty set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0")
}

function testPopFirstPopLastMultipleValues() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    let result_popFirst = treeSet.popFirst();
    let result_popLast = treeSet.popLast();
    assertEQ(result_popFirst, "aa", "The treeSet should pop the first value");
    assertEQ(result_popLast, undefined, "The treeSet should pop the last value");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testPopFirstPopLastDescendingOrder() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return firstValue.compareTo(secondValue) });
    treeSet.add("b");
    treeSet.add("c");
    treeSet.add("a");
    treeSet.add("d");
    let result_popFirst = treeSet.popFirst();
    let result_popLast = treeSet.popLast();
    assertEQ(result_popFirst, "d", "The treeSet should pop the first value");
    assertEQ(result_popLast, "a", "The treeSet should pop the last value");
    assertEQ(treeSet.length, 2, "The treeSet length should be 2");
}

function testPopFirstPopLastAfterClear() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return firstValue.compareTo(secondValue) });
    treeSet.add("squirrel");
    treeSet.add("sparrow");
    treeSet.clear();
    let result1 = treeSet.isEmpty();
    let result_popFirst = treeSet.popFirst();
    let result_popLast = treeSet.popLast();
    let result2 = treeSet.has("squirrel");
    let result3 = treeSet.has("sparrow");
    assertEQ(result_popFirst, undefined, "The treeSet should pop the first value");
    assertEQ(result_popLast, undefined, "The treeSet should pop the last value");
    assertEQ(result1, true, "The treeSet should be empty");
    assertEQ(result2, false, "The treeSet should not contain the value that is not set");
    assertEQ(result3, false, "The treeSet should not contain the value that is not set");
    assertEQ(treeSet.length, 0, "The treeSet length should be 0");
}

function testIterator() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let it = treeSet.$_iterator();
    assertEQ(it.next().value, "aa", "The treeSet $_iterator should return the first value");
    assertEQ(it.next().value, "bb", "The treeSet $_iterator should return the second value");
    assertEQ(it.next().value, undefined, "The treeSet $_iterator should return undefined for no more values");
}

function testIteratorEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let it = treeSet.$_iterator();
    assertEQ(it.next().value, undefined, "The treeSet $_iterator should return undefined when treeSet is empty");
}
