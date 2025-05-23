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

import {LightWeightSet} from "@ohos.util.LightWeightSet";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("LightWeightSet ToString IncreaseCapacityTo ToArray API tests")

    suite.addTest("LightWeightSet toString output data as a string",
        testToStringOutputData)
    suite.addTest("LightWeightSet toString output data as a string of the empty LightWeightSet",
        testToStringEmptySet)
    suite.addTest("LightWeightSet toString output large data as a string",
        testToStringLargeData)
    suite.addTest("LightWeightSet increaseCapacityTo the capacity",
        testIncreaseCapacityTo)
    suite.addTest("LightWeightSet increaseCapacityTo the capacity less than the length of lightWeightSet",
        testIncreaseCapacityToLessThanLength)
    suite.addTest("LightWeightSet increaseCapacityTo the left float capacity",
        testIncreaseCapacityToLeftFloat)
    suite.addTest("LightWeightSet increaseCapacityTo the right float capacity",
        testIncreaseCapacityToRightFloat)
    suite.addTest("LightWeightSet toArray lightWeightSet",
        testToArray)

    return suite.run()
}

function testToStringOutputData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.toString();
    assertEQ(result, "aa,bb", "The lightWeightSet should return a string representation of the set");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testToStringEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let result = lightWeightSet.toString();
    assertEQ(result, "", "The lightWeightSet should return a string representation of the set");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testToStringLargeData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    const MY_MAX = 1024000;
    lightWeightSet.add(MY_MAX);
    let result = lightWeightSet.toString();
    assertEQ(result, MY_MAX + '', "The lightWeightSet should return a string representation of the set");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testIncreaseCapacityTo() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    try {
        lightWeightSet.increaseCapacityTo(16);
        lightWeightSet.increaseCapacityTo(2);
    } catch (err: Error) {
        assertEQ(err.message, "The lightWeightSet should not throw an error when increasing the capacity");
    }
}

function testIncreaseCapacityToLessThanLength() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    try {
        lightWeightSet.increaseCapacityTo(1);
    } catch (err: Error) {
        assertEQ(err.message, "The lightWeightSet should throw an error when increasing the capacity");
    }
}

function testIncreaseCapacityToLeftFloat() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    try {
        lightWeightSet.increaseCapacityTo(Math.floor(1.2));
    } catch (err: Error) {
        assertEQ(err.message, "LightWeightSet should throw an error when increasing the capacity");
    }
}

function testIncreaseCapacityToRightFloat() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    try {
        lightWeightSet.increaseCapacityTo(Math.floor(3.8));
    } catch (err: Error) {
        assertEQ(err.message, "LightWeightSet should not throw an error when increasing the capacity");
    }
}

function testToArray() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let test1 = lightWeightSet.toArray()
    let test2 = new Array<string>(2)
    test2[0] = "aa"
    test2[1] = "bb"
    assertEQ(test1[0], test2[0], "The lightWeightSet fail at 1st judgement");
    assertEQ(test1[1], test2[1], "The lightWeightSet fail at 2nd judgement");
}
