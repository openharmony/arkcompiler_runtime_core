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

import HashMap from "@ohos.util.HashMap";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("HashMap API HasKey HasValue tests")

    suite.addTest("HashMap hasKey and hasValue designate key and value", testHasKeyHasValueWithExistingKeyAndValue)
    suite.addTest("HashMap hasKey and hasValue not set key and value", testHasKeyHasValueWithNonExistingKeyAndValue)
    suite.addTest("HashMap hasKey and hasValue key and value from the null hashMap", testHasKeyHasValueWithEmptyHashMap)
    suite.addTest("HashMap hasKey and hasValue empty string", testHasKeyHasValueWithEmptyStringKey)
    suite.addTest("HashMap hasKey and hasValue data when data is exchange", testHasKeyHasValueAfterDataExchange)
    suite.addTest("HashMap hasKey and hasValue when clear data", testHasKeyHasValueAfterClearingData)

    return suite.run()
}

function testHasKeyHasValueWithExistingKeyAndValue() {
    let hashMap: HashMap<string, number> = new HashMap<string, number>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.hasKey("squirrel");
    let result2 = hashMap.hasValue(123);
    assertEQ(result1, true, "The hashMap contains the specified key");
    assertEQ(result2, true, "The hashMap contains the specified value");
    assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueWithNonExistingKeyAndValue() {
    let hashMap: HashMap<string, number> = new HashMap<string, number>();
    hashMap.set("squirrel", 123);
    let result1 = hashMap.hasKey("sparrow");
    let result2 = hashMap.hasValue(456);
    assertEQ(result1, false, "The hashMap not contains the specified key that is not set");
    assertEQ(result2, false, "The hashMap not contains the specified value that is not set");
    assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueWithEmptyHashMap() {
    let hashMap: HashMap<string, number> = new HashMap<string, number>();
    let result1 = hashMap.hasKey("squirrel");
    let result2 = hashMap.hasValue(123);
    assertEQ(result1, false, "The hashMap not contains the specified key when the hashMap is empty");
    assertEQ(result2, false, "The hashMap not contains the specified value when the hashMap is empty");
    assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}

function testHasKeyHasValueWithEmptyStringKey() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("", "");
    let result1 = hashMap.hasKey("");
    let result2 = hashMap.hasValue("");
    assertEQ(result1, true, "The hashMap contains the empty string key");
    assertEQ(result2, true, "The hashMap contains the empty string value");
    assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueAfterDataExchange() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    let result1 = hashMap.hasKey("hello");
    let result2 = hashMap.hasValue("ji");
    assertEQ(result1, false, "The hashMap not contains the specified key that is not set");
    assertEQ(result2, false, "The hashMap not contains the specified value that is not set");
    assertEQ(hashMap.length, 1, "The hashMap length should be 1");
}

function testHasKeyHasValueAfterClearingData() {
    let hashMap: HashMap<string, string> = new HashMap<string, string>();
    hashMap.set("ji", "hello");
    let result1 = hashMap.isEmpty();
    hashMap.clear();
    let result2 = hashMap.hasKey("ji");
    let result3 = hashMap.hasValue("hello");
    assertEQ(result1, false, "The hashMap should not be empty");
    assertEQ(result2, false, "The hashMap not contains the specified key when the hashMap is empty");
    assertEQ(result3, false, "The hashMap not contains the specified value when the hashMap is empty");
    assertEQ(hashMap.length, 0, "The hashMap length should be 0");
}
