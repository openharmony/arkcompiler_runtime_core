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

let buf = new ArrayBuffer(4);
let flag: Int8Array = new Int8Array(buf, 0, 4);
let del: int = 0;

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrKeysTest: number[] = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let strAll: string = "0,zero,1,one,2,two,3,three,4,four,5,five,6,six,7,seven,8,eight,9,nine";

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapForEachTestsuite = new ArkTestsuite("concurrentHashMapForEachTest");

    concurrentHashMapForEachTestsuite.addTest("forEachTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let flagCounter: int = 0;
        concurrentHashMap.forEach((): void => {
            flagCounter++
        });
        assertEQ(flagCounter, 10);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let flagCounter: int = 0;
        concurrentHashMap.forEach((val: string) => {
            assertEQ(val, arrValues[flagCounter]);
            assertEQ(val, concurrentHashMap.get(flagCounter++));
        });
    });
    concurrentHashMapForEachTestsuite.addTest("forEachTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let flagCounter: int = 0;
        concurrentHashMap.forEach((val: string, key: number) => {
            assertEQ(key, arrKeys[flagCounter]);
            assertEQ(val, arrValues[flagCounter]);
            assertEQ(val, concurrentHashMap.get(flagCounter++));
            val += "-1";
            concurrentHashMap.replace(key, val);
        });
        let add: string = concurrentHashMap.toString();
        let addstr = "0,zero-1,1,one-1,2,two-1,3,three-1,4,four-1,5,five-1,6,six-1,7,seven-1,8,eight-1,9,nine-1";
        assertEQ(add, addstr);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachTest_004", () =>
    {
        let mapp: Map<number, string> = new Map<number, string>();
        for (let i: int = 0; i < 10 ; i++) {
            mapp.set(arrKeys[i], arrValues[i]);
        }
        let flagCounter: int = 0;
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(mapp);
        concurrentHashMap.forEach((val: string, key: number, map: ConcurrentHashMap<number, string>) => {
            assertEQ(val, map.get(key));
            assertEQ(key, arrKeys[flagCounter]);
            assertEQ(val, arrValues[flagCounter]);
            assertEQ(val, concurrentHashMap.get(flagCounter++));
        });
        let add: string = concurrentHashMap.toString();
        assertEQ(add, strAll);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachTest_005", () =>
    {
        let flagCounter: int = 0;
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        for (let i = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        assertEQ(concurrentHashMap.size, 10);
        concurrentHashMap.forEach((val: number, key: string, map: ConcurrentHashMap<string, number>) => {
            assertEQ(val, map.get(key));
            assertEQ(key, arrValues[flagCounter]);
            assertEQ(val, arrKeys[flagCounter]);
            assertEQ(val, concurrentHashMap.get(arrValues[flagCounter]));
            flagCounter++;
        });
        let strTo: string = concurrentHashMap.toString();
        let strSN = "zero,0,one,1,two,2,three,3,four,4,five,5,six,6,seven,7,eight,8,nine,9";
        assertEQ(strTo, strSN);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithoutAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(setAll);
        p1.Await();
        p2.Await();
        assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithoutAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        del = 0;
        setAll();
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(deleteAllBack);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithoutAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(forEachZero);
        let p2 = launch<void, () => void>(getAll);
        p1.Await();
        p2.Await();
        assertEQ(Atomics.load(flag, 0), 10);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithoutAndTest_004", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>(forEachZero);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithOneAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(forEachOne);
        p1.Await();
        p2.Await();
        assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithOneAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        del = 0;
        setAll();
        let p1 = launch<void, () => void>(deleteAllBack);
        let p2 = launch<void, () => void>(forEachOne);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithOneAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(forEachOne);
        p1.Await();
        p2.Await();
        for (let i: int = 0; i < 10; i++) {
            assertEQ(concurrentHashMapOne.get(i), arrValuesTest[i]);
        }
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithOneAndTest_004", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>(forEachOne);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithTwoAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(forEachTwo);
        p1.Await();
        p2.Await();
        assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithTwoAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        del = 0;
        setAll();
        let p1 = launch<void, () => void>(forEachTwo);
        let p2 = launch<void, () => void>(deleteAllBack);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithTwoAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        arrKeysTest = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10];
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(forEachTwo);
        p1.Await();
        p2.Await();
        for (let i: int = 0; i < 10; i++) {
            assertEQ(concurrentHashMapOne.get(i), arrValuesTest[i]);
            if (concurrentHashMapOne.has(i)) {
                assertEQ(i, arrKeysTest[i]);
            }
        }
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithTwoAndTest_004", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>(forEachTwo);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithThreeAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        let p1 = launch<void, () => void>(setAll);
        let p2 = launch<void, () => void>(forEachThree);
        p1.Await();
        p2.Await();
        assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithThreeAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        del = 0;
        setAll();
        let p1 = launch<void, () => void>(forEachThree);
        let p2 = launch<void, () => void>(deleteAllBack);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithThreeAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(getAll);
        let p2 = launch<void, () => void>(forEachThree);
        p1.Await();
        p2.Await();
        for (let i: int = 0; i < 10; i++) {
            assertEQ(concurrentHashMapOne.get(i), arrValuesTest[i]);
            if (concurrentHashMapOne.has(i)) {
                assertEQ(i, arrKeysTest[i]);
            }
        }
    });
    concurrentHashMapForEachTestsuite.addTest("forEachWithThreeAndTest_004", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch<void, () => void>(clearAll);
        let p2 = launch<void, () => void>(forEachThree);
        p1.Await();
        p2.Await();
        assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapForEachTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function getAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.has(i)) {
            let getVal = concurrentHashMapOne.get(i) as string;
            arrValuesTest[i] = getVal;
        }
    }
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.delete(i)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function deleteAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        if (concurrentHashMapOne.delete(i)) {
            del++;
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
}

function forEachZero() {
    concurrentHashMapOne.forEach(() => {
        Atomics.add(flag, 0, 1);
    });
}

function forEachOne() {
    concurrentHashMapOne.forEach((val: string) => {
        let index = Atomics.load(flag, 0) as int;
        arrValuesTest[index] = val;
        Atomics.add(flag, 0, 1);
    });
}

function forEachTwo() {
    concurrentHashMapOne.forEach((val: string, key: number) => {
        let index = Atomics.load(flag, 0) as int;
        arrKeysTest[index] = key;
        arrValuesTest[index] = val;
        Atomics.add(flag, 0, 1);
    });
}

function forEachThree() {
    concurrentHashMapOne.forEach((val: string, key: number, map: ConcurrentHashMap<number, string>) => {
        let index = Atomics.load(flag, 0) as int;
        arrKeysTest[index] = key;
        arrValuesTest[index] = val;
        Atomics.add(flag, 0, 1);
    });
}