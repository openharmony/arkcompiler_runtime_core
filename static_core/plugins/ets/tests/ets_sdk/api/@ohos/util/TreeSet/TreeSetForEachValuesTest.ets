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

function main(): int {
<<<<<<< HEAD
    const suite = new ArkTestsuite("TreeSet ForEach Values API tests")
=======
    const suite = new arktest.ArkTestsuite("TreeSet ForEach Values API tests")
>>>>>>> OpenHarmony_feature_20250328

    suite.addTest("TreeSet forEach the treeSet with no reference",
        testForEachNoReference)
    suite.addTest("TreeSet forEach the treeSet with reference is value",
        testForEachValueReference)
    suite.addTest("TreeSet forEach the treeSet with reference are value and key",
        testForEachValueKeyReference)
    suite.addTest("TreeSet forEach the treeSet with three references",
        testForEachThreeReferences)
    suite.addTest("TreeSet values return iterator by value",
        testValuesReturnIteratorByValue)
    suite.addTest("TreeSet values return iterator by value with null treeSet",
        testValuesReturnIteratorByValueWithEmptyTreeSet)

    return suite.run()
}

function testForEachNoReference() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let count: number = 0;
    treeSet.forEach(() => {
        count++;
    });
<<<<<<< HEAD
    assertEQ(count, 2, "The treeSet forEach should iterate over all elements");
=======
    arktest.assertEQ(count, 2, "The treeSet forEach should iterate over all elements");
>>>>>>> OpenHarmony_feature_20250328
}

function testForEachValueReference() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let flag = false;
    treeSet.forEach((value: string) => {
        flag = treeSet.has(value)
<<<<<<< HEAD
        assertEQ(flag, true, "The treeSet forEach should iterate over all values");
=======
        arktest.assertEQ(flag, true, "The treeSet forEach should iterate over all values");
>>>>>>> OpenHarmony_feature_20250328
    });
}

function testForEachValueKeyReference() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let flag = false;
    treeSet.forEach((value: string, key: string) => {
        flag = treeSet.has(value)
<<<<<<< HEAD
        assertEQ(flag, true, "The treeSet forEach should iterate over all values and keys");
=======
        arktest.assertEQ(flag, true, "The treeSet forEach should iterate over all values and keys");
>>>>>>> OpenHarmony_feature_20250328
    });
}

function testForEachThreeReferences() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let flag = false;
    treeSet.forEach((value: string, key: string, treeSet: TreeSet<string>) => {
        flag = treeSet.has(value)
<<<<<<< HEAD
        assertEQ(flag, true, "The treeSet forEach should iterate over all values, keys, and the set itself");
=======
        arktest.assertEQ(flag, true, "The treeSet forEach should iterate over all values, keys, and the set itself");
>>>>>>> OpenHarmony_feature_20250328
    });
}

function testValuesReturnIteratorByValue() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    treeSet.add("aa");
    treeSet.add("bb");
    let it = treeSet.values();
<<<<<<< HEAD
    assertEQ(it.next().value, "aa", "The treeSet values iterator should return the first value");
    assertEQ(it.next().value, "bb", "The treeSet values iterator should return the second value");
    assertEQ(it.next().value, undefined, "The treeSet values iterator should return undefined for no more values");
=======
    arktest.assertEQ(it.next().value, "aa", "The treeSet values iterator should return the first value");
    arktest.assertEQ(it.next().value, "bb", "The treeSet values iterator should return the second value");
    arktest.assertTrue(it.next().done, "The treeSet values iterator.done should true for no more values");
>>>>>>> OpenHarmony_feature_20250328
}

function testValuesReturnIteratorByValueWithEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let it = treeSet.values();
<<<<<<< HEAD
    assertEQ(it.next().value, undefined, "The treeSet values iterator should return undefined when treeSet is empty");
=======
    arktest.assertTrue(it.next().done, "The treeSet values iterator.done should true when treeSet is empty");
>>>>>>> OpenHarmony_feature_20250328
}
