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
    const suite = new ArkTestsuite("LightWeightSet GetValueAt API tests")

    suite.addTest("LightWeightSet getValueAt value by index", testGetValueAtValidIndex)
    suite.addTest("LightWeightSet getValueAt value by boundary index", testGetValueAtBoundaryIndex)
    suite.addTest("LightWeightSet getValueAt value by left exceeded index", testGetValueAtLeftExceededIndex)
    suite.addTest("LightWeightSet getValueAt value by right exceeded index", testGetValueAtRightExceededIndex)
    suite.addTest("LightWeightSet getValueAt value by index of null lightWeightSet", testGetValueAtIndexOfEmptySet)
    suite.addTest("LightWeightSet getValueAt value by left float index", testGetValueAtLeftFloatIndex)
    suite.addTest("LightWeightSet getValueAt value by right float index", testGetValueAtRightFloatIndex)
    suite.addTest("LightWeightSet getValueAt long string value", testGetValueAtLongString)

    return suite.run()
}

function testGetValueAtValidIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.getValueAt(1);
    assertEQ(result, "bb", "The lightWeightSet should return the value at the specified index");
    assertEQ(lightWeightSet.length, 3, "The lightWeightSet length should be 3");
}

function testGetValueAtBoundaryIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    lightWeightSet.add("cc");
    let result = lightWeightSet.getValueAt(0);
    assertEQ(result, "aa", "The lightWeightSet should return the value at the boundary index");
    assertEQ(lightWeightSet.length, 3, "LightWeightSet length should be 3");
}

function testGetValueAtLeftExceededIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    try {
        lightWeightSet.getValueAt(-2);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testGetValueAtRightExceededIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    try {
        lightWeightSet.getValueAt(2);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testGetValueAtIndexOfEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    try {
        lightWeightSet.getValueAt(4);
    } catch (err: Error) {
        assertEQ(err.message, "The value of index is out of range.");
    }
}

function testGetValueAtLeftFloatIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getValueAt(Math.floor(1.2));
    assertEQ(result, "bb", "The lightWeightSet should return the value at the floored index");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testGetValueAtRightFloatIndex() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    lightWeightSet.add("bb");
    let result = lightWeightSet.getValueAt(Math.floor(1.8));
    assertEQ(result, "bb", "The lightWeightSet should return the value at the floored index");
    assertEQ(lightWeightSet.length, 2, "The lightWeightSet length should be 2");
}

function testGetValueAtLongString() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let str1 = "sparrow"
    let str2 = "squirrel"
    let str3 = "s"
    lightWeightSet.add(str1);
    lightWeightSet.add(str2);
    lightWeightSet.add(str3);
    let result = lightWeightSet.getValueAt(1);
    assertEQ(result, "sparrow", "The lightWeightSet should return the value at the specified index");
}