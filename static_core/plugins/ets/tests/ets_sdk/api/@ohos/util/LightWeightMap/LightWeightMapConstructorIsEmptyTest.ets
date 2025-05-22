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
    const suite = new ArkTestsuite("LightWeightMap Constructor IsEmpty API tests")

    suite.addTest("LightWeightMap construct create default empty lightWeightMap",
        testConstructorCreatesEmptyLightWeightMap)
    suite.addTest("LightWeightMap isEmpty is true",
        testIsEmptyTrue)
    suite.addTest("LightWeightMap isEmpty is false",
        testIsEmptyFalse)
    suite.addTest("LightWeightMap isEmpty is true after remove data",
        testIsEmptyTrueAfterRemove)
    suite.addTest("LightWeightMap isEmpty is true after clear data",
        testIsEmptyTrueAfterClear)

    return suite.run()
}

function testConstructorCreatesEmptyLightWeightMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    assertTrue(lightWeightMap instanceof LightWeightMap,
        "The lightWeightMap should be of the same type as the LightWeightMap");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testIsEmptyTrue() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    let result = lightWeightMap.isEmpty();
    assertEQ(result, true, "The lightWeightMap should be empty");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testIsEmptyFalse() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.isEmpty();
    assertEQ(result, false, "The lightWeightMap should not be empty");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testIsEmptyTrueAfterRemove() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.remove("squirrel");
    let result = lightWeightMap.isEmpty();
    assertEQ(result, true, "The lightWeightMap should be empty");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testIsEmptyTrueAfterClear() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.clear();
    let result = lightWeightMap.isEmpty();
    assertEQ(result, true, "The lightWeightMap should be empty");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}
