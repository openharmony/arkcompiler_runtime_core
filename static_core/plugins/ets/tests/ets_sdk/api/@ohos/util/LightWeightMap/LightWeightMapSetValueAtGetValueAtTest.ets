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
    const suite = new ArkTestsuite("LightWeightMap SetValueAt GetValueAt API tests")

    suite.addTest("LightWeightMap setValueAt getValueAt by index",
        testSetValueAtValidIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by boundary index",
        testSetValueAtBoundaryIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by left exceeded index",
        testSetValueAtLeftExceededIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by right exceeded index",
        testSetValueAtRightExceededIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by index of null lightWeightMap",
        testSetValueAtIndexOfEmptyMap)
    suite.addTest("LightWeightMap setValueAt getValueAt by left float index",
        testSetValueAtLeftFloatIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt by right float index",
        testSetValueAtRightFloatIndex)
    suite.addTest("LightWeightMap setValueAt getValueAt set large value and get by large index",
        testSetValueAtLargeValueAndIndex)

    return suite.run()
}

function testSetValueAtValidIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(1, 777);
    let result2 = lightWeightMap.getValueAt(1);
    assertEQ(result1, true, "The lightWeightMap should set the value at the specified index");
    assertEQ(result2, 777, "The lightWeightMap should return the correct value at the specified index");
}

function testSetValueAtBoundaryIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(2, 777);
    let result2 = lightWeightMap.getValueAt(2);
    assertEQ(result1, true, "The lightWeightMap should set the value at the boundary index");
    assertEQ(result2, 777, "The lightWeightMap should return the correct value at the boundary index");
}

function testSetValueAtLeftExceededIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    try {
        lightWeightMap.setValueAt(-1, 777);
        lightWeightMap.getValueAt(-1);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testSetValueAtRightExceededIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    try {
        lightWeightMap.setValueAt(6, 777);
        lightWeightMap.getValueAt(6);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testSetValueAtIndexOfEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    try {
        lightWeightMap.setValueAt(6, 777);
        lightWeightMap.getValueAt(6);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testSetValueAtLeftFloatIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(Math.floor(1.2), 777);
    let result2 = lightWeightMap.getValueAt(Math.floor(1.2));
    assertEQ(result1, true, "The lightWeightMap should set the value at the floored left float index");
    assertEQ(result2, 777, "The lightWeightMap should return the correct value at the floored left float index");
}

function testSetValueAtRightFloatIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(Math.floor(1.8), 777);
    let result2 = lightWeightMap.getValueAt(Math.floor(1.8));
    assertEQ(result1, true, "The lightWeightMap should set the value at the floored right float index");
    assertEQ(result2, 777, "The lightWeightMap should return the correct value at the floored right float index");
}

function testSetValueAtLargeValueAndIndex() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 111);
    lightWeightMap.set("test", 456);
    let result1 = lightWeightMap.setValueAt(0, Number.MAX_VALUE);
    let result2 = lightWeightMap.getValueAt(0);
    assertEQ(result1, true, "The lightWeightMap should set the value at the floored right float index");
    assertEQ(result2, Number.MAX_VALUE, "The lightWeightMap should get the value at the key index");
    try {
        lightWeightMap.getValueAt(Number.MAX_VALUE);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.")
    }
}
