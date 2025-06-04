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
    const suite = new ArkTestsuite("LightWeightSet Entries API tests")

    suite.addTest("LightWeightSet entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("LightWeightSet entries return iterator by key and value with null lightWeightSet",
        testEntriesReturnIteratorByKeyValueWithEmptySet)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let lightWeightSet: LightWeightSet<number | undefined> = new LightWeightSet<number | undefined>();
    lightWeightSet.add(123);
    lightWeightSet.add(456);
    lightWeightSet.add(333);
    lightWeightSet.add(222);
    let iterator = lightWeightSet.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "123,123,222,222,333,333,456,456";
    assertEQ(str, strAll, "The lightWeightSet entries iterator should return all key-value pairs");
}

function testEntriesReturnIteratorByKeyValueWithEmptySet() {
    let lightWeightSet: LightWeightSet<number | string> = new LightWeightSet<number | string>();
    let it = lightWeightSet.entries();
    assertEQ(it.next().value, undefined,
        "The lightWeightSet entries iterator should return undefined for an empty set");
}
