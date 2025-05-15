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
    const suite = new ArkTestsuite("LightWeightMap Entries API tests")

    suite.addTest("LightWeightMap entries return iterator by key and value with LightWeightMap",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("LightWeightMap entries return iterator by key and value with null LightWeightMap",
        testEntriesReturnIteratorByKeyValueWithEmptyMap)
    suite.addTest("LightWeightMap entries return iterator by key and value with LightWeightMap when key is long string",
        testEntriesReturnIteratorByKeyValue)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let lightWeightMap: LightWeightMap<string, string> = new LightWeightMap<string, string>();
    lightWeightMap.set("sparrow", "123");
    lightWeightMap.set("gull", "357");
    lightWeightMap.set("test", "111");
    lightWeightMap.set("apply", "222");
    let iterator = lightWeightMap.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "gull,357,test,111,apply,222,sparrow,123";
    assertEQ(str, strAll, "The lightWeightMap entries iterator should return the correct key-value pairs");
}

function testEntriesReturnIteratorByKeyValueWithEmptyMap() {
    let lightWeightMap: LightWeightMap<string, number> = new LightWeightMap<string, number>();
    let iteratorSetValues = lightWeightMap.entries();
    assertEQ(iteratorSetValues.next().value, undefined,
        "The lightWeightMap entries iterator should return undefined for an empty map");
}

function testEntriesReturnIteratorByLongStrKey() {
    let lightWeightMap: LightWeightMap<string, string> = new LightWeightMap<string, string>();
    lightWeightMap.set("sparrow", "123");
    lightWeightMap.set("squirrel", "357");
    let iterator = lightWeightMap.entries();
    let str = "";
    for (let i: int = 0; i < 2; i++) {
        str += iterator.next().value!.toString();
    }
    let strAll = "sparrow123squirrel357";
    assertEQ(str, strAll, "The lightWeightMap entries iterator should return the correct key-value pairs");
}