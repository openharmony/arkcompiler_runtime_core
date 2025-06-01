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
    const suite = new ArkTestsuite("LightWeightMap Set Get API tests")

    suite.addTest("LightWeightMap set valid data", testSetValidData)
    suite.addTest("LightWeightMap set larger number value", testSetLargeNumberValue)
    suite.addTest("LightWeightMap get valid key", testGetValidKey)
    suite.addTest("LightWeightMap get not set key", testGetNonExistingKey)
    suite.addTest("LightWeightMap get key from the null lightWeightMap", testGetKeyFromEmptyMap)
    suite.addTest("LightWeightMap get blank key", testGetBlankKey)
    suite.addTest("LightWeightMap get key when contains complex value", testGetKeyWithComplexValue)
    suite.addTest("LightWeightMap get complex key", testGetComplexKey)
    suite.addTest("LightWeightMap get special key", testGetSpecialKey)
    suite.addTest("LightWeightMap get same key when value is different", testGetSameKeyWithDifferentValue)
    suite.addTest("LightWeightMap get empty string", testGetEmptyStringKey)

    return suite.run()
}

function testSetValidData() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    let result = lightWeightMap.get("squirrel");
    assertEQ(result, 123, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testSetLargeNumberValue() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", Number.MAX_VALUE);
    let result = lightWeightMap.get("squirrel");
    assertEQ(result, Number.MAX_VALUE, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetValidKey() {
    let lightWeightMap: LightWeightMap<number, number> = new LightWeightMap<number, number>();
    lightWeightMap.set(123, 356);
    let result = lightWeightMap.get(123);
    assertEQ(result, 356, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetNonExistingKey() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 356);
    let result = lightWeightMap.get("squirrel");
    assertEQ(result, undefined, "The lightWeightMap should return undefined for the key that is not set");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetKeyFromEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    let result = lightWeightMap.get("sparrow");
    assertEQ(result, undefined, "The lightWeightMap should return undefined for the key in an empty map");
    assertEQ(lightWeightMap.length, 0, "The lightWeightMap length should be 0");
}

function testGetBlankKey() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 356);
    let result = lightWeightMap.get(" sparrow ");
    assertEQ(result, undefined, "The lightWeightMap should return undefined for the key that is not set");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetKeyWithComplexValue() {
    interface ComplexValue {
        name: string;
        gender: string;
        age: number;
    }

    const value: ComplexValue = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap: LightWeightMap<string, ComplexValue> = new LightWeightMap<string, ComplexValue>();
    lightWeightMap.set("squirrel", value);
    let result = lightWeightMap.get("squirrel");
    assertEQ(result, value, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetComplexKey() {
    interface ComplexKey {
        name: string;
        gender: string;
        age: number;
    }

    const key: ComplexKey = {
        name: 'squirrel',
        gender: 'male',
        age: 30,
    };
    let lightWeightMap: LightWeightMap<ComplexKey, number> = new LightWeightMap<ComplexKey, number>();
    lightWeightMap.set(key, 123);
    let result = lightWeightMap.get(key);
    assertEQ(result, 123, "The lightWeightMap should return the correct value for the complex key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetSpecialKey() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set('鸿蒙', 123);
    let result = lightWeightMap.get('鸿蒙');
    assertEQ(result, 123, "The lightWeightMap should return the correct value for the special key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetSameKeyWithDifferentValue() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("squirrel", 123);
    lightWeightMap.set("squirrel", 456);
    let result = lightWeightMap.get("squirrel");
    assertEQ(result, 456, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}

function testGetEmptyStringKey() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set('', 123);
    let result = lightWeightMap.get('');
    assertEQ(result, 123, "The lightWeightMap should return the correct value for the key");
    assertEQ(lightWeightMap.length, 1, "The lightWeightMap length should be 1");
}
