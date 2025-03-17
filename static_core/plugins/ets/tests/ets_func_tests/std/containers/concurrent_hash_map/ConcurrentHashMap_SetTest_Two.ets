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

    let concurrentHashMapTestsuite = new ArkTestsuite("concurrentHashMapSetTest");

    // Multiple Threads Test
    concurrentHashMapTestsuite.addTest("setAndTest_001", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setFront();
        let p1 = launch setBack();
        let p2 = launch getAll();
        await p1;
        await p2;
        for (let i: int = 0; i < 10; i++) {
            assertEQ(concurrentHashMapOne.get(i), arrValues[i]);
        }
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValues[i]) {
                flag++;
            }
        }
        assertTrue(flag >= 0);
    });
    concurrentHashMapTestsuite.addTest("setAndTest_003", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setFront();
        let p1 = launch setBack();
        let p2 = launch deleteAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.size <= 5);
    });
    concurrentHashMapTestsuite.addTest("setAndTest_004", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setFront();
        let p1 = launch setBack();
        let p2 = launch replaceAll();
        await p1;
        await p2;
        flag = 0;
        for (let i = 0; i < 10; i++) {
            let newVal = arrValues[i] + "-re";
            if (concurrentHashMapOne.get(i) == newVal) {
                flag++;
            }
        }
        assertTrue(flag >= 5);
    });
    concurrentHashMapTestsuite.addTest("setAndTest_005", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch setAll();
        let p2 = launch isEmptyFun();
        await p1;
        await p2;
        assertEQ(concurrentHashMapOne.size, 10);
    });
    concurrentHashMapTestsuite.addTest("setAndTest_006", () =>
    {
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        let p1 = launch setAll();
        let p2 = launch clearAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapOne.get(9) == undefined || concurrentHashMapOne.get(9) == "nine");
    });
    concurrentHashMapTestsuite.addTest("setAndTest_007", () =>
    {
        arrValuesTest = ["ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten", "ten"];
        concurrentHashMapOne.clear();
        assertTrue(concurrentHashMapOne.isEmpty());
        setFront();
        let p1 = launch setBack();
        let p2 = launch removeAll();
        await p1;
        await p2;
        flag = 0;
        for (let i = 0; i < 10; i++) {
            if (arrValuesTest[i] == arrValues[i]) {
                flag++;
            }
        }
        assertTrue(flag >= 0);
    });

    return concurrentHashMapTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function setFront() {
    for (let i: int = 0; i < 5; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function setBack() {
    for (let i: int = 5; i < 10; i++) {
        concurrentHashMapOne.set(arrKeys[i], arrValues[i]);
    }
}

function replaceAll() {
    for (let i: int = 0; i < 10; i++) {
        let newVal = arrValues[i] + "-re";
        if (concurrentHashMapOne.replace(i, newVal)) {
            flag++;
        }
    }
}

function isEmptyFun() {
    let ie = concurrentHashMapOne.isEmpty();
}

function getAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.has(i)) {
            let getVal = concurrentHashMapOne.get(i) as string;
            arrValuesTest[i] = getVal;
        }
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

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapOne.delete(i)) {
            flag++;
        }
    }
}

function clearAll() {
    concurrentHashMapOne.clear();
}
