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

import TreeMap from "@ohos.util.TreeMap";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeMap Keys Values Iterator API tests")

    suite.addTest("TreeMap keys return iterator by key",
        testKeysReturnIteratorByKey)
    suite.addTest("TreeMap keys return iterator by key with null treeMap",
        testKeysReturnIteratorByKeyWithEmptyTreeMap)
    suite.addTest("TreeMap values return iterator by value",
        testValuesReturnIteratorByValue)
    suite.addTest("TreeMap values return iterator by value with null treeMap",
        testValuesReturnIteratorByValueWithEmptyTreeMap)
    suite.addTest("TreeMap $_iterator return iterator",
        testIteratorReturnIterator)
    suite.addTest("TreeMap $_iterator return iterator with null TreeMap",
        testIteratorReturnIteratorWithEmptyTreeMap)

    return suite.run()
}

function testKeysReturnIteratorByKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let it = treeMap.keys();
    assertEQ(it.next().value, "aa", "The treeMap should return the correct key");
    assertEQ(it.next().value, "bb", "The treeMap should return the correct key");
    assertEQ(it.next().value, undefined, "The treeMap should return undefined for the next key in an empty map");
}

function testKeysReturnIteratorByKeyWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.keys();
    assertEQ(it.next().value, undefined, "The treeMap should return undefined for the key in an empty map");
}

function testValuesReturnIteratorByValue() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    let it = treeMap.values();
    assertEQ(it.next().value, "123", "The treeMap should return the correct value");
    assertEQ(it.next().value, "456", "The treeMap should return the correct value");
    assertEQ(it.next().value, undefined, "The treeMap should return undefined for the next value in an empty map");
}

function testValuesReturnIteratorByValueWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.values();
    assertEQ(it.next().value, undefined, "The treeMap should return undefined for the next value in an empty map");
}

function testIteratorReturnIterator() {
    let treeMap: TreeMap<string, number | undefined> =
        new TreeMap<string, number | undefined>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", 123);
    treeMap.set("bb", 456);
    treeMap.set("cc", 789);
    treeMap.set("dd", 112);
    let iterator = treeMap.$_iterator();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "aa,123,bb,456,cc,789,dd,112";
    assertEQ(str, strAll, "The treeMap should return the correct iterator");
}

function testIteratorReturnIteratorWithEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    let it = treeMap.$_iterator();
    assertEQ(it.next().value, undefined, "The treeMap should return undefined for the next iterator in an empty map");
}
