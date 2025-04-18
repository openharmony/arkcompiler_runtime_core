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

import { launch } from "std/concurrency"

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapClearTestsuite = new ArkTestsuite("concurrentHashMapClearTest");

    concurrentHashMapClearTestsuite.addTest("clearTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        assertTrue(concurrentHashMap.isEmpty());
        concurrentHashMap.clear();
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapClearTestsuite.addTest("clearTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertFalse(concurrentHashMap.isEmpty());
        concurrentHashMap.clear();
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapClearTestsuite.addTest("clearTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        for (let i = 0; i < 10 ; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        assertFalse(concurrentHashMap.isEmpty());
        concurrentHashMap.clear();
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapClearTestsuite.addTest("clearAndTest_001", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch<void, () => void>(removeAll);
        let p2 = launch<void, () => void>(clearAll);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapClearTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function removeAll() {
    for (let i = 0; i < 10; i++) {
        if (!concurrentHashMapOne.isEmpty()) {
            let val = concurrentHashMapOne.remove(i);
            if (val !== undefined) {
                arrValuesTest[i] = val as string;
            }
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
}
