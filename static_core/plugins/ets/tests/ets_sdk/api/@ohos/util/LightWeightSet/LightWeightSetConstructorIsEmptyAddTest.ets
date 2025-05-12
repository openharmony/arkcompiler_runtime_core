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
    const suite = new ArkTestsuite("LightWeightSet Constructor IsEmpty Add API tests")

    suite.addTest("LightWeightSet construct default empty lightWeightSet", testConstructorCreatesEmptySet)
    suite.addTest("LightWeightSet isEmpty is true", testIsEmptyTrue)
    suite.addTest("LightWeightSet isEmpty is false", testIsEmptyFalse)
    suite.addTest("LightWeightSet isEmpty is true after remove data", testIsEmptyTrueAfterRemove)
    suite.addTest("LightWeightSet isEmpty is true after clear data", testIsEmptyTrueAfterClear)
    suite.addTest("LightWeightSet add data", testAddData)
    suite.addTest("LightWeightSet add Max data", testAddMaxData)
    suite.addTest("LightWeightSet add exist data", testAddExistData)

    return suite.run()
}

function testConstructorCreatesEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    assertTrue(lightWeightSet instanceof LightWeightSet,
        "The lightWeightSet should be of the same type as the LightWeightSet");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testIsEmptyTrue() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let result = lightWeightSet.isEmpty();
    assertEQ(result, true, "The lightWeightSet should be empty");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testIsEmptyFalse() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.isEmpty();
    assertEQ(result, false, "The lightWeightSet should not be empty");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testIsEmptyTrueAfterRemove() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add(3);
    lightWeightSet.remove(3)
    let result = lightWeightSet.isEmpty();
    assertEQ(result, true, "The lightWeightSet should be empty");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testIsEmptyTrueAfterClear() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa")
    lightWeightSet.add("bb")
    lightWeightSet.add("cc")
    lightWeightSet.clear()
    let result = lightWeightSet.isEmpty();
    assertEQ(result, true, "The lightWeightSet should  be empty");
    assertEQ(lightWeightSet.length, 0, "The lightWeightSet length should be 0");
}

function testAddData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");
    let result = lightWeightSet.has("aa");
    assertEQ(result, true, "The lightWeightSet should contain the added value");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testAddMaxData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    const MY_MAX = 10240;
    let result = lightWeightSet.add(MY_MAX);
    assertEQ(result, true, "The lightWeightSet should contain the added value");
    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}

function testAddExistData() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    lightWeightSet.add("aa");

    let result = lightWeightSet.has("aa");
    assertEQ(result, true, "The lightWeightSet should contain the added value");

    result = lightWeightSet.add("aa");
    assertEQ(result, false, "The lightWeightSet should not add exist value");

    assertEQ(lightWeightSet.length, 1, "The lightWeightSet length should be 1");
}