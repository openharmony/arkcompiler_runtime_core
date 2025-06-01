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
let buf = new SharedArrayBuffer(4);
let flag: Int8Array = new Int8Array(buf, 0, 4);

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapTestsuite = new ArkTestsuite("concurrentHashMapReplaceTest");
    // Single Thread Test
    concurrentHashMapTestsuite.addTest("replaceTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        assertFalse(concurrentHashMap.replace(0, "zero"));
        assertFalse(concurrentHashMap.replace(1, "one"));
        assertFalse(concurrentHashMap.replace(2, "two"));
    });
    concurrentHashMapTestsuite.addTest("replaceTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertEQ(concurrentHashMap.size, 10);
        assertTrue(concurrentHashMap.replace(0, "zero_Test"));
        assertTrue(concurrentHashMap.replace(1, "one_Test"));
        assertEQ(concurrentHashMap.get(0), "zero_Test");
        assertEQ(concurrentHashMap.get(1), "one_Test");
    });
    concurrentHashMapTestsuite.addTest("replaceTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        assertFalse(concurrentHashMap.replace(10, "ten"));
        assertFalse(concurrentHashMap.replace(11, "eleven"));
    });
    concurrentHashMapTestsuite.addTest("replaceTest_004", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        for (let i = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        for (let i = 0; i < 10; i++) {
            let j = i + 10;
            concurrentHashMap.replace(arrValues[i], j);
        }
        let reItor = concurrentHashMap.values();
        let reItorVal = reItor.next().value;
        for (let i = 0; i < 10; i++) {
            let j = i + 10;
            assertEQ(reItorVal, j);
            reItorVal = reItor.next().value;
        }
    });
    concurrentHashMapTestsuite.addTest("twiceTest_005", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        Atomics.store(flag, 0, 0);
        let p1 = launch replaceAll();
        let p2 = launch replaceAll();
        await p1;
        await p2;
        for (let i = 0; i < 10; i++) {
            let newVal = arrValues[i] + "-re"
            assertEQ(concurrentHashMapOne.get(i), newVal);
        }
    });
    concurrentHashMapTestsuite.addTest("replaceAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch replaceAll();
        let p2 = launch isEmptyFun();
        await p1;
        await p2;
        assertEQ(Atomics.load(flag, 0), 10);
        for (let i = 0; i < 10; i++) {
            let reVal = arrValues[i] + "-re";
            assertEQ(concurrentHashMapOne.get(i), reVal);
        }
    });
    concurrentHashMapTestsuite.addTest("replaceAndTest_002", () => 
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch replaceAllBack();
        let p2 = launch clearAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.isEmpty());
        assertTrue(Atomics.load(flag, 0) >= 0);
    });
    concurrentHashMapTestsuite.addTest("replaceAndTest_003", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch replaceAll();
        let p2 = launch removeAllBack();
        await p1;
        await p2;
        let flagCounter: int = 0;
        for (let i = 0; i < 10; i++) {
            let newVal = arrValues[i] + "-re";
            if (arrValuesTest[i] == newVal) {
                flagCounter++;
            }
        }
        assertTrue(flagCounter >= 0);
        assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function replaceAll() {
    for (let i: int = 0; i < 10; i++) {
        let newVal = arrValues[i] + "-re";
        if (concurrentHashMapOne.replace(i, newVal)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function replaceAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        let newVal = arrValues[i] + "-re";
        if (concurrentHashMapOne.replace(i, newVal)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function isEmptyFun() {
    let ie = concurrentHashMapOne.isEmpty();
}

function removeAllBack() {
    for (let i = 9; i >= 0; i--) {
        if (concurrentHashMapOne.has(i)) {
            arrValuesTest[i] = concurrentHashMapOne.remove(i) as string;
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
}
