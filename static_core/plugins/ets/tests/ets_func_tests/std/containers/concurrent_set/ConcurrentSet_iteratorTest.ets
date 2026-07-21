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
    let concurrentSetTestsuite = new arktest.ArkTestsuite('ConcurrentSet$_iteratorTest');
    // Single Thread Test
    concurrentSetTestsuite.addTest('$_iteratorTest001', () => {
        let concurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = concurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10; i++) {
            concurrentSetTest.add(arrValues[i]);
        }
        res = concurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let iterValues: IterableIterator<string> = concurrentSetTest.$_iterator();
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterValues.next().value != arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 0);
        arktest.assertEQ(iterValues.next().value, undefined);
    });
    concurrentSetTestsuite.addTest('$_iteratorTest002', () => {
        let concurrentSetTestNull: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = concurrentSetTestNull.size;
        arktest.assertEQ(res, 0);
        let iterValue: IterableIterator<string> = concurrentSetTestNull.$_iterator();
        arktest.assertEQ(iterValue.next().value, undefined);
    });
    // Multiple Threads Test
    concurrentSetTestsuite.addTest('$_iterator&AddTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch(addBack);
        let p2 = launch(iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 10);
    });
    concurrentSetTestsuite.addTest('$_iterator&DeteleTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch(deleteAll);
        let p2 = launch(iteratorAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
    });
    concurrentSetTestsuite.addTest('$_iterator&DeteleTest002', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        deleteAll();
        iteratorAll();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
    });
    concurrentSetTestsuite.addTest('$_iterator&ClearTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch(iteratorAll);
        let p2 = launch(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size < 11);
    });

    return concurrentSetTestsuite.run();
}

function iteratorAll() {
    let iterator$ = concurrentSetTestOne.$_iterator();
    let i = 0;
    while (iterator$.next().value != undefined) {
        i++;
    }
}

function addFront() {
    for (let i: int = 0; i < 5; i++) {
        concurrentSetTestOne.add(arrKeys[i]);
    }
}

function addBack() {
    for (let i: int = 5; i < 10; i++) {
        concurrentSetTestOne.add(arrKeys[i]);
    }
}

function clearTest() {
    concurrentSetTestOne.clear();
}

function deleteAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentSetTestOne.delete(i);
    }
}

function addAll() {
    for (let i: int = 0; i < 10; i++) {
        concurrentSetTestOne.add(arrKeys[i]);
    }
}
