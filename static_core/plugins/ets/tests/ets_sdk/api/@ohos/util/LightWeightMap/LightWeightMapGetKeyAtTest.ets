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
    const suite = new ArkTestsuite("LightWeightMap GetKeyAt API tests")

    suite.addTest("LightWeightMap getKeyAt key by index", testGetKeyAtValidIndex)
    suite.addTest("LightWeightMap getKeyAt key by boundary index", testGetKeyAtBoundaryIndex)
    suite.addTest("LightWeightMap getKeyAt key by left exceeded index", testGetKeyAtLeftExceededIndex)
    suite.addTest("LightWeightMap getKeyAt key by right exceeded index", testGetKeyAtRightExceededIndex)
    suite.addTest("LightWeightMap getKeyAt key by index of null lightWeightMap", testGetKeyAtIndexOfEmptyMap)
    suite.addTest("LightWeightMap getKeyAt key by left float index", testGetKeyAtLeftFloatIndex)
    suite.addTest("LightWeightMap getKeyAt key by right float index", testGetKeyAtRightFloatIndex)


    return suite.run()
}

function testGetKeyAtValidIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(1);
    assertEQ(result, "test", "The lightWeightMap should return the correct key at the specified index");
}

function testGetKeyAtBoundaryIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(2);
    assertEQ(result, "sparrow", "The lightWeightMap should return the correct key at the boundary index");
}

function testGetKeyAtLeftExceededIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    try {
        lightWeightMap.getKeyAt(-1);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testGetKeyAtRightExceededIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    try {
        lightWeightMap.getKeyAt(5);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testGetKeyAtIndexOfEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    try {
        lightWeightMap.getKeyAt(5);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testGetKeyAtLeftFloatIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(Math.floor(1.2));
    assertEQ(result, "test", "The lightWeightMap should return the correct key at the floored left float index");
}

function testGetKeyAtRightFloatIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result = lightWeightMap.getKeyAt(Math.floor(1.8));
    assertEQ(result, "test", "The lightWeightMap should return the correct key at the floored right float index");
}
