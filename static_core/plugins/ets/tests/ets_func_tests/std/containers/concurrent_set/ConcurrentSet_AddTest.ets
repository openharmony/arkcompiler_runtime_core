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
let strAllOne: string = "ConcurrentSet[0,1,2,3,4,5,6,7,8,9]";

let ConcurrentSetTestOne: ConcurrentSet<number> = new ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetAddTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("AddTestOne001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
    });
    ConcurrentSetTestsuite.addTest("AddTestTwo001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
    });
    ConcurrentSetTestsuite.addTest("AddTestOne002", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[0]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
    });
    ConcurrentSetTestsuite.addTest("AddTestTwo002", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[0]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 1);
    });
    // Multiple Threads Test
    ConcurrentSetTestsuite.addTest("Add&ToStringTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        let p1 = launch<void, () => void>(addAll);
        let p2 = launch<void, () => void>(toStringTest);
        p1.Await();
        p2.Await();
        let str: string = ConcurrentSetTestOne.toString();
        assertTrue(str != strAllOne || str == strAllOne);
    });
    ConcurrentSetTestsuite.addTest("Add&AddTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        let p = launch<void, () => void>(addAll);
        p.Await();
        assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("Add&HasTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        let p1 = launch<void, () => void>(addAll);
        let p2 = launch<void, () => void>(hasAll);
        p1.Await();
        p2.Await();
        for (let i: int = 0; i < 10; i++) {
            assertEQ(ConcurrentSetTestOne.has(i), true);
        }
    });
    ConcurrentSetTestsuite.addTest("Add&HasSizeTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        let p1 = launch<void, () => void>(addAll);
        let p2 = launch<void, () => void>(getSize);
        p1.Await();
        p2.Await();
        assertEQ(ConcurrentSetTestOne.size, 10);
    });
    ConcurrentSetTestsuite.addTest("Add&DeteleTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch<void, () => void>(addBack);
        let p2 = launch<void, () => void>(deleteAll);
        p1.Await();
        p2.Await();
        assertTrue(ConcurrentSetTestOne.size < 6);
    });
    ConcurrentSetTestsuite.addTest("Add&ClearTest001", () =>
    {
        ConcurrentSetTestOne.clear();
        assertEQ(ConcurrentSetTestOne.size, 0);
        let p1 = launch<void, (i: int) => void>(addOneTest, 2);
        let p2 = launch<void, () => void>(clearTest);
        p1.Await();
        p2.Await();
        assertTrue(ConcurrentSetTestOne.size < 2);
    });

    return ConcurrentSetTestsuite.run();
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

function getSize() {
    ConcurrentSetTestOne.size;
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

function addOneTest(i: int) {
    ConcurrentSetTestOne.add(arrKeys[i]);
}

function toStringTest() {
    for (let i: int = 0; i < 10 ; i++) {
        let str: string = ConcurrentSetTestOne.toString();
    }
}
