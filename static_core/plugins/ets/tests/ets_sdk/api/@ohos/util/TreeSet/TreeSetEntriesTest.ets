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

import {TreeSet} from "@ohos.util.TreeSet";

const success = 0;
const fail = 1;

function main(): int {
    const suite = new ArkTestsuite("TreeSet Entries API tests")

    suite.addTest("TreeSet entries return iterator by key and value",
        testEntriesReturnIteratorByKeyValue)
    suite.addTest("TreeSet entries return iterator by key and value with null treeSet",
        testEntriesReturnIteratorByKeyValueWithEmptyTreeSet)

    return suite.run()
}

function testEntriesReturnIteratorByKeyValue() {
    let treeSet: TreeSet<number | undefined> = new TreeSet<number | undefined>(
        (firstValue: number | undefined, secondValue: number | undefined): number => {
            return secondValue! - firstValue!
        });
    treeSet.add(123);
    treeSet.add(456);
    treeSet.add(222);
    treeSet.add(333);
    let iterator = treeSet.entries();
    let str = "";
    for (let i: int = 0; i < 4; i++) {
        str += iterator.next().value!.toString();
        if (i < 3) {
            str += ",";
        }
    }
    let strAll = "123,123,222,222,333,333,456,456";
    assertEQ(str, strAll, "The treeSet entries iterator should return the first value");
}

function testEntriesReturnIteratorByKeyValueWithEmptyTreeSet() {
    let treeSet: TreeSet<string> = new TreeSet<string>((firstValue: string,
        secondValue: string): number => { return secondValue.compareTo(firstValue) });
    let it = treeSet.entries();
    assertEQ(it.next().value, undefined, "The treeSet entries iterator should return undefined when treeSet is empty");
}
