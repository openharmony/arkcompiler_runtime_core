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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrKeysTest: number[] = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new ArkTestsuite("concurrentHashMapRemoveTest");

    concurrentHashMapTestsuite.addTest("removeTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        assertEQ(concurrentHashMap.remove(0), undefined);
        assertEQ(concurrentHashMap.remove(1), undefined);
    });
    concurrentHashMapTestsuite.addTest("removeTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertEQ(concurrentHashMap.remove(0), "zero");
        assertEQ(concurrentHashMap.remove(0), undefined);
        assertEQ(concurrentHashMap.remove(1), "one");
        assertEQ(concurrentHashMap.remove(1), undefined);
    });
    concurrentHashMapTestsuite.addTest("removeTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertEQ(concurrentHashMap.remove(10), undefined);
        assertEQ(concurrentHashMap.remove(11), undefined);
    });
    concurrentHashMapTestsuite.addTest("removeTest_004", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        assertEQ(concurrentHashMap.size, 0);
        for (let i = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        assertEQ(concurrentHashMap.size, 10);
        for (let i = 0; i < 10; i++) {
            assertEQ(concurrentHashMap.remove(arrValues[i]), arrKeys[i]);
        }
        assertTrue(concurrentHashMap.isEmpty());
    });
    concurrentHashMapTestsuite.addTest("twiceTest_006", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch removeAll();
        let p2 = launch removeAllBack();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapTestsuite.run();
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

function removeAllBack() {
    for (let i = 9; i >= 0; i--) {
        if (!concurrentHashMapOne.isEmpty()) {
            let val = concurrentHashMapOne.remove(i);
            if (val !== undefined) {
                arrValuesTest[i] = val as string;
            }
        }
    }
}