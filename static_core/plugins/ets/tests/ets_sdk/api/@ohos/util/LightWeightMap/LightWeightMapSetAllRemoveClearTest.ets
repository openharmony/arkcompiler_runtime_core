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

import LightWeightMap from "@ohos.util.LightWeightMap";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("LightWeightMap SetAll Remove Clear API tests")

    suite.addTest("LightWeightMap setAll data from map2 to map1", testSetAllFromMap2ToMap1)
    suite.addTest("LightWeightMap setAll data from empty map2 to map1", testSetAllFromEmptyMap2ToMap1)
    suite.addTest("LightWeightMap remove key", testRemoveExistingKey)
    suite.addTest("LightWeightMap remove not set key", testRemoveNonExistingKey)
    suite.addTest("LightWeightMap remove key from the null LightWeightMap", testRemoveKeyFromEmptyMap)
    suite.addTest("LightWeightMap remove large number key", testRemoveLargeNumberKey)
    suite.addTest("LightWeightMap clear data", testClearData)
    suite.addTest("LightWeightMap clear the null LightWeightMap", testClearEmptyMap)
    suite.addTest("LightWeightMap clear data and set data again", testClearAndSetDataAgain)

    return suite.run()
}

function testSetAllFromMap2ToMap1() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    newLightWeightMap.setAll(lightWeightMap);
    let result = newLightWeightMap.get("sparrow");
    assertEQ(result, 356, "The newLightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    assertEQ(newLightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testSetAllFromEmptyMap2ToMap1() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("sparrow", 356);
    let newLightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.setAll(newLightWeightMap);
    let result = lightWeightMap.get("sparrow");
    assertEQ(result, 356, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
    assertEQ(newLightWeightMap.length, 0, "The newLightWeightMap length should be 0");
}

function testRemoveExistingKey() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.remove("squirrel");
    assertEQ(result, 123, "The lightWeightMap should remove the key and return its value");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testRemoveNonExistingKey() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.remove("test");
    assertEQ(result, undefined, "The lightWeightMap should return undefined for the key that is not set");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testRemoveKeyFromEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    let result = lightWeightMap.remove("squirrel");
    assertEQ(result, undefined, "The lightWeightMap should return undefined for the key in an empty map");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testRemoveLargeNumberKey() {
    let lightWeightMap: LightWeightMap<number, string> = new LightWeightMap<number, string>();
    lightWeightMap.set(Number.MAX_VALUE, "squirrel");
    let result = lightWeightMap.remove(Number.MAX_VALUE);
    assertEQ(result, "squirrel", "The lightWeightMap should remove the key and return its value");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testClearData() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    let result1 = lightWeightMap.isEmpty();
    lightWeightMap.clear();
    let result2 = lightWeightMap.get("squirrel");
    assertEQ(result1, false, "The lightWeightMap should not be empty");
    assertEQ(result2, undefined, "The lightWeightMap should be cleared");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testClearEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.clear();
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testClearAndSetDataAgain() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.clear();
    let result = lightWeightMap.isEmpty();
    lightWeightMap.set("a", 123);
    lightWeightMap.set("b", 356);
    assertEQ(result, true, "The lightWeightMap should be empty");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}
