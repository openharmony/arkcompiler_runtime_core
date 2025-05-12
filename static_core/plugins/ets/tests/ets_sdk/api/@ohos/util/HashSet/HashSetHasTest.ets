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
    const suite = new ArkTestsuite("HashSet Has API tests")

    suite.addTest("HashSet has value of the hashSet", testHasValue)
    suite.addTest("HashSet has not set value of the hashSet", testHasNotSetValue)
    suite.addTest("HashSet has value of the null hashSet", testHasValueOfEmptyHashSet)
    suite.addTest("HashSet has null value of the hashSet", testHasNullValue)
    suite.addTest("HashSet has not set null value of the hashSet", testHasNotSetNullValue)
    suite.addTest("HashSet has value after clear data", testHasValueAfterClear)
    suite.addTest("HashSet has empty string", testHasEmptyString)

    return suite.run()
}

function testHasValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has("squirrel");
    assertEQ(result, true, "The hashSet should contain the specified value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasNotSetValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has("test");
    assertEQ(result, false, "The hashSet should not contain the specified value that is not set");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasValueOfEmptyHashSet() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    let result = hashSet.has("test");
    assertEQ(result, false, "The hashSet should not contain the specified value when the hashSet is empty");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testHasNullValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add(null);
    let result = hashSet.has(null);
    assertEQ(result, true, "The hashSet should contain the null value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasNotSetNullValue() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result = hashSet.has(null);
    assertEQ(result, false, "The hashSet should not contain the specified value that is not set");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}

function testHasValueAfterClear() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("squirrel");
    let result1 = hashSet.isEmpty();
    hashSet.clear();
    let result2 = hashSet.has("squirrel");
    assertEQ(result1, false, "The hashSet should not be empty");
    assertEQ(result2, false, "The hashSet should contain the specified value that is not set");
    assertEQ(hashSet.length, 0, "The hashSet length should be 0");
}

function testHasEmptyString() {
    let hashSet: HashSet<string | null> = new HashSet<string | null>();
    hashSet.add("");
    let result = hashSet.has("");
    assertEQ(result, true, "The hashSet should contain the specified value");
    assertEQ(hashSet.length, 1, "The hashSet length should be 1");
}
