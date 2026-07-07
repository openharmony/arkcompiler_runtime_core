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

import { launch } from 'std/concurrency'

let flag: int = 0;
let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine'];

let concurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let concurrentSetTestsuite = new arktest.ArkTestsuite('ConcurrentSetForEachTest');

    concurrentSetTestsuite.addTest('TestForEach001', () => {
        let concurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        concurrentSetTest.add(arrValues[0]);
        concurrentSetTest.add(arrValues[1]);
        flag = 0;
        concurrentSetTest.forEach((): void => {
            flag++;
        });
        arktest.assertEQ(flag, 2);
    });
    concurrentSetTestsuite.addTest('ForEach&AddTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        flag = 0;
        let p1 = launch(forEachZero);
        let p2 = launch(addAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size >= 0);
    });
    concurrentSetTestsuite.addTest('ForEach&AddTest002', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        flag = 0;
        forEachZero();
        addAll();
        arktest.assertEQ(concurrentSetTestOne.size, 10);
    });
    concurrentSetTestsuite.addTest('ForEach&DeteleTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch(forEachZero);
        let p2 = launch(deleteAllBack);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size < 11);
    });
    concurrentSetTestsuite.addTest('ForEach&HasTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch(forEachZero);
        let p2 = launch(hasAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 10);
    });
    concurrentSetTestsuite.addTest('ForEach&ClearTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        flag = 0;
        addAll();
        let p1 = launch(forEachZero);
        let p2 = launch(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size < 11);
    });

    return concurrentSetTestsuite.run();
}

function forEachZero() {
    concurrentSetTestOne.forEach(() => {
        flag++;
    });
}

function clearTest() {
    concurrentSetTestOne.clear();
}

function deleteAllBack() {
    for (let i: int = 9; i >= 0; i--) {
        concurrentSetTestOne.delete(i);
    }
}

function hasAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentSetTestOne.has(i);
    }
}

function addAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentSetTestOne.add(arrKeys[i]);
    }
}
