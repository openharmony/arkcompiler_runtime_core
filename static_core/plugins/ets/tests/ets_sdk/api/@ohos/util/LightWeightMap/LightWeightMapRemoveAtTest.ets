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
    const suite = new ArkTestsuite("LightWeightMap RemoveAt API tests")

    suite.addTest("LightWeightMap removeAt data by index", testRemoveAtValidIndex)
    suite.addTest("LightWeightMap removeAt data by boundary index", testRemoveAtBoundaryIndex)
    suite.addTest("LightWeightMap removeAt data by left exceeded index", testRemoveAtLeftExceededIndex)
    suite.addTest("LightWeightMap removeAt data by right exceeded index", testRemoveAtRightExceededIndex)
    suite.addTest("LightWeightMap removeAt data by index of null lightWeightMap", testRemoveAtIndexOfEmptyMap)
    suite.addTest("LightWeightMap removeAt data by left float index", testRemoveAtLeftFloatIndex)
    suite.addTest("LightWeightMap removeAt data by right float index", testRemoveAtRightFloatIndex)

    return suite.run()
}

function testRemoveAtValidIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(1);
    assertEQ(result, true, "The lightWeightMap should remove the element at the specified index");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testRemoveAtBoundaryIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(2);
    assertEQ(result, true, "The lightWeightMap should remove the element at the boundary index");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testRemoveAtLeftExceededIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(-1);
    assertEQ(result, false, "The lightWeightMap should not remove the element at the left exceeded index");
    assertEQ(lightWeightMap.length, 3, "The lightWeightMap length should be 3");
}

function testRemoveAtRightExceededIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(5);
    assertEQ(result, false, "The lightWeightMap should not remove the element at the right exceeded index");
    assertEQ(lightWeightMap.length, 3, "The lightWeightMap length should be 3");
}

function testRemoveAtIndexOfEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    try {
        lightWeightMap.removeAt(5);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testRemoveAtLeftFloatIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(Math.floor(1.2));
    assertEQ(result, true, "The lightWeightMap should remove the element at the floored left float index");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}

function testRemoveAtRightFloatIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.removeAt(Math.floor(1.8));
    assertEQ(result, true, "The lightWeightMap should remove the element at the floored right float index");
    assertEQ(lightWeightMap.length, 2, "The lightWeightMap length should be 2");
}
