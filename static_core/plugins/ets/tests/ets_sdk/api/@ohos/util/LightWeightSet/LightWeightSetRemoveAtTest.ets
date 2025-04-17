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
    const suite = new ArkTestsuite("LightWeightSet RemoveAt API tests")

    suite.addTest("LightWeightSet removeAt data by index", testRemoveAtValidIndex)
    suite.addTest("LightWeightSet removeAt data by boundary index", testRemoveAtBoundaryIndex)
    suite.addTest("LightWeightSet removeAt data by left exceeded index", testRemoveAtLeftExceededIndex)
    suite.addTest("LightWeightSet removeAt data by right exceeded index", testRemoveAtRightExceededIndex)
    suite.addTest("LightWeightSet removeAt data by index of null lightWeightSet", testRemoveAtIndexOfEmptySet)
    suite.addTest("LightWeightSet removeAt data by left float index", testRemoveAtLeftFloatIndex)
    suite.addTest("LightWeightSet removeAt data by right float index", testRemoveAtRightFloatIndex)

    return suite.run()
}

function testRemoveAtValidIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.removeAt(1);
    assertEQ(result, true, "The lightWeightSet should remove the value at the specified index");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtBoundaryIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.removeAt(0);
    assertEQ(result, true, "The lightWeightSet should remove the value at the boundary index");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtLeftExceededIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(-2);
    assertEQ(result, false, "The lightWeightSet should not remove the value at an out-of-range index");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtRightExceededIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(2);
    assertEQ(result, false, "The lightWeightSet should not remove the value at an out-of-range index");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testRemoveAtIndexOfEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    try {
        lightWeightSet.removeAt(4);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testRemoveAtLeftFloatIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(Math.floor(1.2));
    assertEQ(result, true, "The lightWeightSet should remove the value at the floored index");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testRemoveAtRightFloatIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.removeAt(Math.floor(1.8));
    assertEQ(result, true, "The lightWeightSet should remove the value at the floored index");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}
