/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
let strAllOne: string = 'ConcurrentSet[0,1,2,3,4,5,6,7,8,9]';
const arr: Number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

let concurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let concurrentSetTestsuite = new arktest.ArkTestsuite('ConcurrentSetConstructorTest');

    // Single Thread Test
    concurrentSetTestsuite.addTest('ConstructorTest001', () => {
        let concurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        arktest.assertEQ(concurrentSetTest.size, 0);
    });
    concurrentSetTestsuite.addTest('ConstructorTest002', () => {
        let array = Array.from<number>(arr);
        let arrayLike = array as ArrayLike<number>;
        let concurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(arrayLike);
        arktest.assertEQ(concurrentSetTest.size, 10);
        flag = 0;
        concurrentSetTest.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
    });
    concurrentSetTestsuite.addTest('ConstructorTest003', () => {
        let array = Array.from<number>(arr);
        let concurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(array);
        arktest.assertEQ(concurrentSetTest.size, 10);
        flag = 0;
        concurrentSetTest.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
    });
    concurrentSetTestsuite.addTest('ConstructorTest004', () => {
        let array = Array.from<number>(arr);
        let concurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(array);
        arktest.assertEQ(concurrentSetTest.size, 10);
        flag = 0;
        concurrentSetTest.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
        let iter = concurrentSetTest.keys();
        let concurrentSetTestIter: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(iter);
        arktest.assertEQ(concurrentSetTestIter.size, 10);
        flag = 0;
        concurrentSetTestIter.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
    });
    concurrentSetTestsuite.addTest('ConstructorTest005', () => {
        let setTest = new Set<number>();
        for (let i = 0; i < 10; i++) {
            setTest.add(i);
        }
        let concurrentSetTestIter: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>(setTest);
        flag = 0;
        concurrentSetTestIter.forEach((val: number) => {
            arktest.assertEQ(val, flag++);
        });
    });

    return concurrentSetTestsuite.run();
}
