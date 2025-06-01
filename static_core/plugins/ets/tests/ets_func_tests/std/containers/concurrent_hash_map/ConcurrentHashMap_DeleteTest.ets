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
let reFlag = 0;

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];
let arrValuesTest: string[] = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];

let concurrentHashMapOne: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapOneTestsuite = new ArkTestsuite("concurrentHashMapDeleteTest");

    concurrentHashMapOneTestsuite.addTest("deleteTest_001", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let value: string | undefined = concurrentHashMap.get(1);
        let del: boolean = concurrentHashMap.delete(1);
        let valueDel: string | undefined = concurrentHashMap.get(1);
        assertEQ(value, "one");
        assertTrue(del);
        assertEQ(valueDel, undefined);
    });
    concurrentHashMapOneTestsuite.addTest("deleteTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let value: string | undefined = concurrentHashMap.get(10);
        let del: boolean = concurrentHashMap.delete(10);
        let valueDel: string | undefined = concurrentHashMap.get(10);
        assertEQ(value, undefined);
        assertFalse(del);
        assertEQ(valueDel, undefined);
    });
    concurrentHashMapOneTestsuite.addTest("deleteTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        let value: string | undefined = concurrentHashMap.get(1);
        let del: boolean = concurrentHashMap.delete(1);
        let valueDel: string | undefined = concurrentHashMap.get(1);
        assertEQ(value, undefined);
        assertFalse(del);
        assertEQ(valueDel, undefined);
    });
    concurrentHashMapOneTestsuite.addTest("deleteTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<string, number>();
        concurrentHashMap.set("zero", 0);
        let value: number | undefined = concurrentHashMap.get("zero");
        let del: boolean = concurrentHashMap.delete("zero");
        let valueDel: number | undefined = concurrentHashMap.get("zero");
        assertEQ(value, 0);
        assertTrue(del);
        assertEQ(valueDel, undefined);
    });
    concurrentHashMapOneTestsuite.addTest("twiceTest_004", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch deleteAll();
        let p2 = launch deleteAllBack();
        await p1;
        await p2;
        assertEQ(concurrentHashMapOne.size, 0);
    });
    concurrentHashMapOneTestsuite.addTest("deleteAndTest_001", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch deleteAll();
        let p2 = launch replaceAll();
        await p1;
        await p2;
        assertTrue(Atomics.load(flag, 0) >= 10);
    });
    concurrentHashMapOneTestsuite.addTest("deleteAndTest_002", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        Atomics.store(flag, 0, 0);
        setAll();
        let p1 = launch deleteAll();
        let p2 = launch isEmptyFun();
        await p1;
        await p2;
        assertEQ(Atomics.load(flag, 0), 10);
    });
    concurrentHashMapOneTestsuite.addTest("deleteAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch deleteAll();
        let p2 = launch clearAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.isEmpty());
    });
    concurrentHashMapOneTestsuite.addTest("deleteAndTest_004", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setAll();
        let p1 = launch removeAll();
        let p2 = launch deleteAllBack();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.isEmpty());
    });

    return concurrentHashMapOneTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
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
            Atomics.add(flag, 0, 1);
        }
    }
}

function removeAll() {
    for (let i = 0; i < 10; i++) {
        if (!concurrentHashMapOne.isEmpty()) {
            let val = concurrentHashMapOne.remove(i);
            if (val !== undefined) {
                reFlag++;
            }
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
}

function replaceAll() {
    for (let i: int = 0; i < 10; i++) {
        let newVal = arrValues[i] + "-re";
        if (concurrentHashMapOne.replace(i, newVal)) {
            Atomics.add(flag, 0, 1);
        }
    }
}

function isEmptyFun() {
    let ie = concurrentHashMapOne.isEmpty();
}
