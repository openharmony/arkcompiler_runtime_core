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
    const suite = new ArkTestsuite("HashSet Add Remove API tests")

    suite.addTest("HashSet add data", testAddData)
    suite.addTest("HashSet add null data", testAddNullData)
    suite.addTest("HashSet add empty string", testAddEmptyString)
    suite.addTest("HashSet add data string length is 2048", testAddDataStringLength2048)
    suite.addTest("HashSet add data loop 1000", testAddDataLoop1000)
    suite.addTest("HashSet remove value", testRemoveValue)
    suite.addTest("HashSet remove not set value", testRemoveNotSetValue)
    suite.addTest("HashSet remove value from the null hashSet", testRemoveValueFromEmptyHashSet)
    suite.addTest("HashSet remove null value", testRemoveNullValue)
    suite.addTest("HashSet remove not set null value", testRemoveNotSetNullValue)
    suite.addTest("HashSet remove empty string", testRemoveEmptyString)

    return suite.run()
}

function testAddData() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has("squirrel");
    assertEQ(result, true, "The hashSet should contain the added value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testAddNullData() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add(null);
    let result = hashSet.has(null);
    assertEQ(result, true, "The hashSet should contain the added null value");
    assertEQ(hashSet.length, 1, "HashSet length should be 1");
}

function testAddEmptyString() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("");
    let result = hashSet.has("");
    assertEQ(result, true, "The hashSet should contain the added value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testAddDataStringLength2048() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let param = 'a'.repeat(2048);
    hashSet.add(param);
    let result = hashSet.has(param);
    assertEQ(result, true, "The hashSet should contain the added value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testAddDataLoop1000() {
    let hashSet: HashSet<number | null> = new HashSet<number | null>();
    for (let i = 0; i < 1000; i++) {
        hashSet.add(i);
    }
    for (let i = 0; i < hashSet.length; i++) {
        let result = hashSet.has(i);
        assertEQ(result, true, "The hashSet should contain the added value");
    }
    assertEQ(hashSet.length, 1000, "The hashSet length should be 1000");
}

function testRemoveValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.remove("squirrel");
    assertEQ(result, true, "The hashSet should remove the value");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testRemoveNotSetValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.remove("test");
    assertEQ(result, false, "The hashSet should return false when the value is not set");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testRemoveValueFromEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let result = hashSet.remove("test");
    assertEQ(result, false, "The hashSet should return false when the hashSet is empty");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testRemoveNullValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add(null);
    let result = hashSet.remove(null);
    assertEQ(result, true, "The hashSet should remove the null value");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testRemoveNotSetNullValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.remove(null);
    assertEQ(result, false, "The hashSet should return false when the value is not set");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testRemoveEmptyString() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("");
    let result = hashSet.remove("");
    assertEQ(result, true, "The hashSet should remove the value");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}
