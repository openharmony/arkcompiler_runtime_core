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

let flag: int = 0;

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapIsEmptyTestsuite = new ArkTestsuite("concurrentHashMapIsEmptyTest");

    concurrentHashMapIsEmptyTestsuite.addTest("isEmptyTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapIsEmptyTestsuite.addTest("isEmptyTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertFalse(concurrentHashMap.isEmpty());
    });
    concurrentHashMapIsEmptyTestsuite.addTest("isEmptyTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertFalse(concurrentHashMap.isEmpty());
        concurrentHashMap.clear();
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapIsEmptyTestsuite.addTest("isEmptyTest_004", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        concurrentHashMap.set("zero", 0);
        assertFalse(concurrentHashMap.isEmpty());
        concurrentHashMap.clear();
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapIsEmptyTestsuite.addTest("isEmptyAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        assertFalse(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(isEmptyFun);
        let p2 = launch<void, () => void>(clearAll);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapIsEmptyTestsuite.addTest("isEmptyAndTest_002", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        assertFalse(concurrentHashMapOne.isEmpty());
        let p1 = launch<void, () => void>(isEmptyFun);
        let p2 = launch<void, () => void>(removeAll);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
        flag = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValues[i]) {
                flag++;
            }
        }
        assertEQ(flag, 10);
    });

    return concurrentHashMapIsEmptyTestsuite.run();
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

function isEmptyFun() {
    concurrentHashMapOne.isEmpty();
}

function clearAll() {
    concurrentHashMapOne.clear();
}