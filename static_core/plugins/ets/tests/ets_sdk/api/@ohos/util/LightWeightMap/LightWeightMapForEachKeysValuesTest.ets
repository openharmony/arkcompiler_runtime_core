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
    const suite = new ArkTestsuite("LightWeightMap ForEach Keys Values API tests")

    suite.addTest("LightWeightMap forEach the lightWeightMap with no reference",
        testForEachNoReference)
    suite.addTest("LightWeightMap forEach the lightWeightMap with reference is value",
        testForEachValueReference)
    suite.addTest("LightWeightMap forEach the lightWeightMap with reference are value and key",
        testForEachKeyValueReference)
    suite.addTest("LightWeightMap forEach the lightWeightMap with three references",
        testForEachThreeReferences)
    suite.addTest("LightWeightMap keys return iterator by key",
        testKeysIterator)
    suite.addTest("LightWeightMap keys return iterator by key with null lightWeightMap",
        testKeysIteratorWithEmptyMap)
    suite.addTest("LightWeightMap values return iterator by value",
        testValuesIterator)
    suite.addTest("LightWeightMap values return iterator by value with null lightWeightMap",
        testValuesIteratorWithEmptyMap)

    return suite.run()
}

function testForEachNoReference() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let count: number = 0;
    lightWeightMap.forEach(() => {
        count++;
    });
    assertEQ(count, 2, "The lightWeightMap forEach should iterate over all elements");
}

function testForEachValueReference() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let flag: boolean = false;
    lightWeightMap.forEach((value: number) => {
        flag = lightWeightMap.hasValue(value);
        assertEQ(flag, true, "The lightWeightMap forEach should iterate over all values");
    });
}

function testForEachKeyValueReference() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let flag1: boolean = false;
    let flag2: boolean = false;
    lightWeightMap.forEach((value: number, key: string) => {
        flag1 = lightWeightMap.hasValue(value);
        flag2 = lightWeightMap.hasKey(key);
        assertEQ(flag1, true, "The lightWeightMap forEach should iterate over all values");
        assertEQ(flag2, true, "The lightWeightMap forEach should iterate over all keys");
    });
}

function testForEachThreeReferences() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let flag1: boolean = false;
    let flag2: boolean = false;
    lightWeightMap.forEach((value: number, key: string, lightWeightMap: LightWeightMap<string, number>) => {
        flag1 = lightWeightMap.hasValue(value);
        flag2 = lightWeightMap.hasKey(key);
        assertEQ(flag1, true, "The lightWeightMap forEach should iterate over all elements with three references");
        assertEQ(flag2, true, "The lightWeightMap forEach should iterate over all elements with three references");
    });
}

function testKeysIterator() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let iteratorSetValues = lightWeightMap.keys();
    assertEQ(iteratorSetValues.next().value, "gull", "The lightWeightMap keys iterator should return the correct key");
    assertEQ(iteratorSetValues.next().value, "sparrow",
        "The lightWeightMap keys iterator should return the correct key");
    assertEQ(iteratorSetValues.next().value, undefined,
        "The lightWeightMap keys iterator should return undefined at the end");
}

function testKeysIteratorWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    let iteratorSetValues = lightWeightMap.keys();
    assertEQ(iteratorSetValues.next().value, undefined,
        "The lightWeightMap keys iterator should return undefined for an empty map");
}

function testValuesIterator() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    lightWeightMap.set("sparrow", 123);
    lightWeightMap.set("gull", 357);
    let iteratorSetValues = lightWeightMap.values();
    assertEQ(iteratorSetValues.next().value, 357, "The lightWeightMap values iterator should return the correct value");
    assertEQ(iteratorSetValues.next().value, 123, "The lightWeightMap values iterator should return the correct value");
    assertEQ(iteratorSetValues.next().value, undefined,
        "The lightWeightMap values iterator should return undefined at the end");
}

function testValuesIteratorWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    let iteratorSetValues = lightWeightMap.values();
    assertEQ(iteratorSetValues.next().value, undefined,
        "The lightWeightMap values iterator should return undefined for an empty map");
}
