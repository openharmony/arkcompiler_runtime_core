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

let ConcurrentSetTestOne: ConcurrentSet<number> = new ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetKeysTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("KeysTest001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
        let iterKeys: IterableIterator<string> = ConcurrentSetTest.keys();
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value != arrValues[i]) {
                flag++;
            }
        }
        assertEQ(flag, 0);
        assertEQ(iterKeys.next().value, undefined);
    });
    ConcurrentSetTestsuite.addTest("KeysTest002", () =>
    {
        let ConcurrentSetTestNull: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTestNull.size;
        assertEQ(res, 0);
        let iterKey: IterableIterator<string> = ConcurrentSetTestNull.keys();
        assertEQ(iterKey.next().value, undefined);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("Keys&AddTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch addBack();
        let p2 = launch keysAll();
        await p1;
        await p2;
        assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("Key&DeteleTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch deleteAll();
        let p2 = launch keysAll();
        await p1;
        await p2;
        assertEQ(ConcurrentSetTestOne.size, 0);
    });
    ConcurrentSetTestsuite.addTest("Key&DeteleTest002", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        deleteAll();
        keysAll();
        assertEQ(ConcurrentSetTestOne.size, 0);
    });
    ConcurrentSetTestsuite.addTest("Key&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch keysAll();
        let p2 = launch clearTest();
        await p1;
        await p2;
        assertTrue(ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("Key&ClearTest002", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p = launch keysAll();
        clearTest();
        await p;
        assertEQ(ConcurrentSetTestOne.size, 0);
    });

    return ConcurrentSetTestsuite.run();
}

function keysAll() {
    let iterKeys = ConcurrentSetTestOne.keys();
    let j = 0;
    for (let i = 0; i < 10; i++) {
        if (iterKeys.next().value == arrKeys[i]) {
            j++;
        }
    }
}

function addFront() {
    for (let i: int = 0; i < 5; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}

function addBack() {
    for (let i: int = 5; i < 10; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        ConcurrentSetTestOne.delete(i);
    }
}

function addAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}
