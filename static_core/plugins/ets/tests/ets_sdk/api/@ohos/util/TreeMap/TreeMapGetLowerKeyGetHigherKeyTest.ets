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
    const suite = new ArkTestsuite("TreeMap GetLowerKey GetHigherKey API tests")

    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher key", testGetLowerKeyAndHigherKeyValidKeys)
    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher key from the null treeMap",
        testGetLowerKeyAndHigherKeyEmptyTreeMap)
    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher key with boundary data",
        testGetLowerKeyAndHigherKeyBoundaryData)
    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher non-exist key.",
        testGetLowerKeyAndHigherKeyNonExistingKey)
    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher empty string key when treeMap is empty.",
        testGetLowerKeyAndHigherKeyEmptyStringKey)
    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher object key", testGetLowerKeyAndHigherKeyObjectKeys)
    suite.addTest("TreeMap getLowerKey getHigherKey lower and higher object key with boundary data",
        testGetLowerKeyAndHigherKeyObjectKeysBoundaryData)

    return suite.run()
}


class MyObject extends Object {
    size: number;
    constructor(size: number) {
        super();
        this.size = size;
    }
}

function equalObjects(first: MyObject, second: MyObject): boolean {
    if (first.size == second.size) {
        return true;
    }
    return false;
}

function testGetLowerKeyAndHigherKeyValidKeys() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    treeMap.set("cc", "789");
    let lower = treeMap.getLowerKey("bb");
    let lower_ = treeMap.getLowerKey("cc");
    let higher = treeMap.getHigherKey("aa");
    let higher_ = treeMap.getHigherKey("bb");
    assertEQ(lower, "aa", "The treeMap should return the correct lower key");
    assertEQ(lower_, "bb", "The treeMap should return the correct lower key");
    assertEQ(higher, "bb", "The treeMap should return the correct higher key");
    assertEQ(higher_, "cc", "The treeMap should return the correct higher key");
    assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testGetLowerKeyAndHigherKeyEmptyTreeMap() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    let lower = treeMap.getLowerKey("bb");
    let higher = treeMap.getHigherKey("aa");
    assertEQ(lower, undefined, "The treeMap should return undefined for the lower key in an empty map");
    assertEQ(higher, undefined, "The treeMap should return undefined for the higher key in an empty map");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testGetLowerKeyAndHigherKeyBoundaryData() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("aa", "123");
    treeMap.set("bb", "456");
    treeMap.set("cc", "789");
    let lower = treeMap.getLowerKey("aa");
    let higher = treeMap.getHigherKey("cc");
    assertEQ(lower, undefined, "The treeMap should return undefined for the lower key at the boundary value");
    assertEQ(higher, undefined, "The treeMap should return undefined for the higher key at the boundary value");
    assertEQ(treeMap.length, 3, "The treeMap length should be 3");
}

function testGetLowerKeyAndHigherKeyNonExistingKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    treeMap.set("bb", "456");
    treeMap.set("cc", "123");
    treeMap.set("dd", "789");
    treeMap.set("ee", "111");
    treeMap.set("gg", "777");
    treeMap.set("ii", "333");
    treeMap.set("kk", "555");
    treeMap.set("nn", "666");
    let lower_ = treeMap.getLowerKey("aa");
    let lower = treeMap.getLowerKey("ff");
    let higher_ = treeMap.getHigherKey("pp");
    let higher = treeMap.getHigherKey("ff");
    assertEQ(lower_, undefined, "The treeMap should return undefined when no data is found above this key");
    assertEQ(higher_, undefined, "The treeMap should return undefined when no data is found below this key");
    assertEQ(lower, "ee", "The treeMap should return the correct lower key");
    assertEQ(higher, "gg", "The treeMap should return the correct higher key");
    assertEQ(treeMap.length, 8, "The treeMap length should be 8");
}

function testGetLowerKeyAndHigherKeyEmptyStringKey() {
    let treeMap: TreeMap<string, string> =
        new TreeMap<string, string>((firstValue: string, secondValue: string): number => {
            return secondValue.compareTo(firstValue)
        });
    let lower = treeMap.getLowerKey("");
    let higher = treeMap.getHigherKey("");
    assertEQ(lower, undefined, "The treeMap should return undefined for the lower key at the boundary value");
    assertEQ(higher, undefined, "The treeMap should return undefined for the higher key at the boundary value");
    assertEQ(treeMap.length, 0, "The treeMap length should be 0");
}

function testGetLowerKeyAndHigherKeyObjectKeys() {
    let treeMapo: TreeMap<MyObject, MyObject> =
        new TreeMap<MyObject, MyObject>((firstValue: MyObject, secondValue: MyObject): number => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    let obj_sec: MyObject = new MyObject(2);
    let obj_trd: MyObject = new MyObject(3);
    let obj_v: MyObject = new MyObject(4);
    let obj_sec_v: MyObject = new MyObject(5);
    let obj_trd_v: MyObject = new MyObject(6);

    treeMapo.set(obj, obj_v);
    treeMapo.set(obj_sec, obj_sec_v);
    treeMapo.set(obj_trd, obj_trd_v);
    let lower = treeMapo.getLowerKey(obj_sec);
    let lower_ = treeMapo.getLowerKey(obj_trd);
    let higher = treeMapo.getHigherKey(obj);
    let higher_ = treeMapo.getHigherKey(obj_sec);
    assertTrue(equalObjects(lower!, obj!), "The treeMap should return the correct lower key");
    assertTrue(equalObjects(lower_!, obj_sec!), "The treeMap should return the correct lower key");
    assertTrue(equalObjects(higher!, obj_sec!), "The treeMap should return the correct higher key");
    assertTrue(equalObjects(higher_!, obj_trd!), "The treeMap should return the correct higher key");
    assertEQ(treeMapo.length, 3, "The treeMap length should be 3");
}

function testGetLowerKeyAndHigherKeyObjectKeysBoundaryData() {
    let treeMapo: TreeMap<MyObject, MyObject> =
        new TreeMap<MyObject, MyObject>((firstValue: MyObject, secondValue: MyObject): number => {
            return secondValue.size - firstValue.size;
        });
    let obj: MyObject = new MyObject(1);
    let obj_sec: MyObject = new MyObject(2);
    let obj_trd: MyObject = new MyObject(3);
    let obj_v: MyObject = new MyObject(4);
    let obj_sec_v: MyObject = new MyObject(5);
    let obj_trd_v: MyObject = new MyObject(6);
    treeMapo.set(obj, obj_v);
    treeMapo.set(obj_sec, obj_sec_v);
    treeMapo.set(obj_trd, obj_trd_v);
    let lower = treeMapo.getLowerKey(obj);
    let higher = treeMapo.getHigherKey(obj_trd);
    assertEQ(lower, undefined, "The treeMap should return undefined for the lower key at the boundary value");
    assertEQ(higher, undefined, "The treeMap should return undefined for the higher key at the boundary value");
    assertEQ(treeMapo.length, 3, "The treeMap length should be 3");
}
