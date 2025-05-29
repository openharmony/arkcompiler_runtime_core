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

let concurrentHashMapKeys: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();

function main(): int {
    let map: Map<number, string> = new Map<number, string>();
    for (let i: int = 0; i < 10 ; i++) {
        map.set(arrKeys[i], arrValues[i]);
    }

    let concurrentHashMapKeysTestsuite = new ArkTestsuite("concurrentHashMapKeysTest");

    concurrentHashMapKeysTestsuite.addTest("keysTest_001", () =>
    {
        let concurrentHashMapNull: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        let iterKey: IterableIterator<number> = concurrentHashMapNull.keys();
        assertEQ(iterKey.next().value, undefined);
        assertEQ(iterKey.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysTest_002", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>(map);
        let iterKeys: IterableIterator<number> = concurrentHashMap.keys();
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value == arrKeys[i]) {
                flag++;
            }
        }
        assertEQ(flag, 10);
        assertEQ(iterKeys.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysTest_003", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<number, string> = new ConcurrentHashMap<number, string>();
        let iterKeys: IterableIterator<number> = concurrentHashMap.keys();
        assertEQ(iterKeys.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrKeys[i], arrValues[i]);
        }
        flag = 0;
        iterKeys = concurrentHashMap.keys();
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value == arrKeys[i]) {
                flag++;
            }
        }
        assertEQ(flag, 10);
        assertEQ(iterKeys.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysTest_004", () =>
    {
        let concurrentHashMap: ConcurrentHashMap<string, number> = new ConcurrentHashMap<String, number>();
        let iterKeys: IterableIterator<string> = concurrentHashMap.keys();
        assertEQ(iterKeys.next().value, undefined);
        for (let i: int = 0; i < 10; i++) {
            concurrentHashMap.set(arrValues[i], arrKeys[i]);
        }
        flag = 0;
        iterKeys = concurrentHashMap.keys();
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value == arrValues[i]) {
                flag++;
            }
        }
        assertEQ(flag, 10);
        assertEQ(iterKeys.next().value, undefined);
    });
    concurrentHashMapKeysTestsuite.addTest("keysAndTest_001", () =>
    {
        concurrentHashMapKeys.clear();
        assertTrue(concurrentHashMapKeys.isEmpty());
        let p1 = launch setAll();
        let p2 = launch keysAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapKeys.size >= 0);
    });
    concurrentHashMapKeysTestsuite.addTest("keysAndTest_002", () =>
    {
        concurrentHashMapKeys.clear();
        assertTrue(concurrentHashMapKeys.isEmpty());
        setAll();
        let p1 = launch deleteAll();
        let p2 = launch keysAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapKeys.isEmpty());
    });
    concurrentHashMapKeysTestsuite.addTest("keysAndTest_003", () =>
    {
        concurrentHashMapKeys.clear();
        assertTrue(concurrentHashMapKeys.isEmpty());
        setAll();
        let p1 = launch clearAll();
        let p2 = launch keysAll();
        await p1;
        await p2;
        assertTrue(concurrentHashMapKeys.isEmpty());
    });

    return concurrentHashMapKeysTestsuite.run();
}

function setAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentHashMapKeys.set(arrKeys[i], arrValues[i]);
    }
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        if (concurrentHashMapKeys.delete(i)) {
            flag++;
        }
    }
}

function clearAll() {
    concurrentHashMapKeys.clear();
}

function keysAll() {
    let iterKeys = concurrentHashMapKeys.keys();
    for (let i = 0; iterKeys.next().value == arrKeys[i]; i++) {
        if (iterKeys.next() == undefined) {
            break;
        }
    }
}