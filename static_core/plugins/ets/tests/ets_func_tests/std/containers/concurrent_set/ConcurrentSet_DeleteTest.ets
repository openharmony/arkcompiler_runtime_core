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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"];

let ConcurrentSetTestOne: ConcurrentSet<number> = new ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetDeteleTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("DeteleTestOne001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        ConcurrentSetTest.add(arrKeys[0]);
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrKeys[0]);
        assertEQ(del, true);
        res = ConcurrentSetTest.size;
        assertEQ(res, 0);
    });
    ConcurrentSetTestsuite.addTest("DeteleTestTwo001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        ConcurrentSetTest.add(arrValues[0]);
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrValues[0]);
        assertEQ(del, true);
        res = ConcurrentSetTest.size;
        assertEQ(res, 0);
    });
    ConcurrentSetTestsuite.addTest("DeteleTestOne002", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        ConcurrentSetTest.add(arrKeys[0]);
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrKeys[1]);
        assertEQ(del, false);
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
    });
    ConcurrentSetTestsuite.addTest("DeteleTestTwo002", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        ConcurrentSetTest.add(arrValues[0]);
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
        let del = ConcurrentSetTest.delete(arrValues[1]);
        assertEQ(del, false);
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("Detele&HasSizeTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, (i: int) => void>(deleteOne, 0);
        let p2 = launch<void, (i: int) => void>(deleteOne, 1);
        let p3 = launch<void, (i: int) => void>(deleteOne, 2);
        let p4 = launch<void, () => void>(getSize);
        p1.Await();
        p2.Await();
        p3.Await();
        p4.Await();
        assertTrue(ConcurrentSetTestOne.size > 6 && ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("Detele&HasTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, (i: int) => void>(deleteOne, 0);
        let p2 = launch<void, (i: int) => void>(deleteOne, 1);
        let p3 = launch<void, (i: int) => void>(hasOne, 3);
        p1.Await();
        p2.Await();
        p3.Await();
        assertTrue(ConcurrentSetTestOne.size > 7 && ConcurrentSetTestOne.size < 11);
    });
    ConcurrentSetTestsuite.addTest("Detele&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch<void, (i: int) => void>(deleteOne, 0);
        let p2 = launch<void, () => void>(clearTest);
        p1.Await();
        p2.Await();
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

function getSize() {
    ConcurrentSetTestOne.size;
}

function deleteOne(i: int) {
    ConcurrentSetTestOne.delete(arrKeys[i]);
}

function clearTest() {
    ConcurrentSetTestOne.clear();
}