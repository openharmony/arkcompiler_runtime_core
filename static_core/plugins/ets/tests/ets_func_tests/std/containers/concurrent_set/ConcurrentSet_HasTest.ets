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

let ConcurrentSetTestOne: ConcurrentSet<number> = new ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetHasTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("HasTestOne001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
        let hasFlag = ConcurrentSetTest.has(1);
        assertEQ(hasFlag, true);
    });
    ConcurrentSetTestsuite.addTest("HasTestTwo001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        assertEQ(ConcurrentSetTest.size, 10);
        let one: string = "one";
        let hasFlag = ConcurrentSetTest.has(one);
        assertEQ(hasFlag, true);
    });
    ConcurrentSetTestsuite.addTest("HasTestOne002", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
        let hasFlag = ConcurrentSetTest.has(20);
        assertEQ(hasFlag, false);
    });
    ConcurrentSetTestsuite.addTest("HasTestTwo002", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
        let hasFlag = ConcurrentSetTest.has("error");
        assertEQ(hasFlag, false);
    });
    ConcurrentSetTestsuite.addTest("HasTestOne003", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        let hasFlag = ConcurrentSetTest.has(10);
        assertEQ(hasFlag, false);
    });
    ConcurrentSetTestsuite.addTest("HasTestTwo003", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        let hasFlag = ConcurrentSetTest.has("two");
        assertEQ(hasFlag, false);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("Has&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        assertEQ(ConcurrentSetTestOne.has(3), true);
        let p1 = launch hasOne(0);
        let p2 = launch clearTest();
        await p1;
        await p2;
        assertEQ(ConcurrentSetTestOne.has(0), false);
    });

    return ConcurrentSetTestsuite.run();
}

function addAll() {
    for (let i: int = 0; i < 10 ; i++) {
        ConcurrentSetTestOne.add(arrKeys[i]);
    }
}

function hasOne(i: int) {
    ConcurrentSetTestOne.has(i);
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}
