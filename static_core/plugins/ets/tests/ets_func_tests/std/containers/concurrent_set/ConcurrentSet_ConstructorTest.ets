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
let strAllOne: string = "ConcurrentSet[0,1,2,3,4,5,6,7,8,9]";
const arr: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

let ConcurrentSetTestOne: ConcurrentSet<number> = new ConcurrentSet<number>();

function main(): int {
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetConstructorTest");

    // Single Thread Test
    ConcurrentSetTestsuite.addTest("ConstructorTest001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        assertEQ(ConcurrentSetTest.size, 0);
    });
    ConcurrentSetTestsuite.addTest("ConstructorTest002", () =>
    {
        let array = Array.from<number>(arr);
        let arrayLike = array as ArrayLike<number>;
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>(arrayLike);
        assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            assertEQ(val, flag++);
        });
    });
    ConcurrentSetTestsuite.addTest("ConstructorTest003", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>(array);
        assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            assertEQ(val, flag++);
        });
    });
    ConcurrentSetTestsuite.addTest("ConstructorTest004", () =>
    {
        let array = Array.from<number>(arr);
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>(array);
        assertEQ(ConcurrentSetTest.size, 10);
        flag = 0;
        ConcurrentSetTest.forEach((val: number) => {
            assertEQ(val, flag++);
        });
        let iter = ConcurrentSetTest.keys();
        let ConcurrentSetTestIter: ConcurrentSet<number> = new ConcurrentSet<number>(iter);
        assertEQ(ConcurrentSetTestIter.size, 10);
        flag = 0;
        ConcurrentSetTestIter.forEach((val: number) => {
            assertEQ(val, flag++);
        });
    });
    ConcurrentSetTestsuite.addTest("ConstructorTest005", () =>
    {
        let setTest = new Set<number>();
        for (let i = 0; i < 10; i++) {
            setTest.add(i);
        }
        let ConcurrentSetTestIter: ConcurrentSet<number> = new ConcurrentSet<number>(setTest);
        flag = 0;
        ConcurrentSetTestIter.forEach((val: number) => {
            assertEQ(val, flag++);
        });
    });

    return ConcurrentSetTestsuite.run();
}
