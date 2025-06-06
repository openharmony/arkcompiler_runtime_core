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

import {HashSet} from "@ohos.util.HashSet";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("HashSet Clear Foreach API tests")

    suite.addTest("HashSet clear data", testClearData)
    suite.addTest("HashSet clear the null hashSet", testClearEmptyHashSet)
    suite.addTest("HashSet clear data before add data", testClearBeforeAddData)
    suite.addTest("HashSet forEach the hashSet with no reference", testForEachNoReference)
    suite.addTest("HashSet forEach the hashSet with reference is value", testForEachValueReference)
    suite.addTest("HashSet forEach the hashSet with reference are value and key", testForEachKeyValueReference)
    suite.addTest("HashSet forEach the hashSet with three references", testForEachThreeReferences)

    return suite.run()
}

function testClearData() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result1 = hashSet.isEmpty();
    hashSet.clear();
    let result2 = hashSet.has("squirrel");
    assertEQ(result1, false, "The hashSet should not be empty");
    assertEQ(result2, false, "The hashSet should be cleared");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testClearEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.clear();
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testClearBeforeAddData() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.clear();
    let result1 = hashSet.isEmpty();
    hashSet.add("squirrel");
    let result2 = hashSet.has("squirrel");
    assertEQ(result1, true, "The hashSet should be cleared");
    assertEQ(result2, true, "The hashSet should contain the specified value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testForEachNoReference() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.add("sparrow");
    let count: number = 0;
    hashSet.forEach(() => {
        count++;
    });
    assertEQ(count, 2, "The hashSet forEach should iterate over all elements");
}

function testForEachValueReference() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.add("sparrow");
    let flag: boolean = false;
    hashSet.forEach((value: string | null) => {
        flag = hashSet.has(value);
        assertEQ(flag, true, "The hashSet forEach should iterate over all values");
    });
}

function testForEachKeyValueReference() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.add("sparrow");
    let flag: boolean = false;
    hashSet.forEach((value: string | null, key: string | null) => {
        flag = hashSet.has(value);
        assertEQ(flag, true, "The hashSet forEach should iterate over all values");
    });
}

function testForEachThreeReferences() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    hashSet.add("sparrow");
    let flag: boolean = false;
    hashSet.forEach((value: string | null, key: string | null, tempHashSet: HashSet<string | null>) => {
        flag = tempHashSet.has(value);
        assertEQ(flag, true, "The hashSet forEach should iterate over all values");
    });
}
