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
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetForEachTest");

    ConcurrentSetTestsuite.addTest("TestForEach001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        ConcurrentSetTest.add(arrValues[0]);
        ConcurrentSetTest.add(arrValues[1]);
        flag = 0;
        ConcurrentSetTest.forEach((): void => {
            flag++;
        });
        assertEQ(flag, 2);
    });
    ConcurrentSetTestsuite.addTest("ForEach&AddTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        let p1 = launch forEachZero();
        let p2 = launch addAll();
        await p1;
        await p2;
        assertTrue(ConcurrentSetTestOne.size >= 0);
    });
    ConcurrentSetTestsuite.addTest("ForEach&AddTest002", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        forEachZero();
        addAll();
        assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("ForEach&DeteleTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch forEachZero();
        let p2 = launch deleteAllBack();
        await p1;
        await p2;
        assertTrue(ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("ForEach&HasTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch forEachZero();
        let p2 = launch hasAll();
        await p1;
        await p2;
        assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("ForEach&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0); 
        flag = 0;
        addAll();
        let p1 = launch forEachZero();
        let p2 = launch clearTest();
        await p1;
        await p2;
        assertTrue(ConcurrentSetTestOne.size < 11);
    });

    return ConcurrentSetTestsuite.run();
}

function forEachZero() {
    ConcurrentSetTestOne.forEach(() => {
        flag++;
    });
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}

function deleteAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        ConcurrentSetTestOne.delete(i);
    }
}

function hasAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.has(i);
    }
}

function addAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}
