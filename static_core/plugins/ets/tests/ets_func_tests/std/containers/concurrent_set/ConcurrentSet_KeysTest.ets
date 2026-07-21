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
    let concurrentSetTestsuite = new arktest.ArkTestsuite('ConcurrentSetKeysTest');
    // Single Thread Test
    concurrentSetTestsuite.addTest('KeysTest001', () => {
        let concurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = concurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10; i++) {
            concurrentSetTest.add(arrValues[i]);
        }
        res = concurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let iterKeys: IterableIterator<string> = concurrentSetTest.keys();
        flag = 0;
        for (let i: int = 0; i < 10; i++) {
            if (iterKeys.next().value != arrValues[i]) {
                flag++;
            }
        }
        arktest.assertEQ(flag, 0);
        arktest.assertEQ(iterKeys.next().value, undefined);
    });
    concurrentSetTestsuite.addTest('KeysTest002', () => {
        let concurrentSetTestNull: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = concurrentSetTestNull.size;
        arktest.assertEQ(res, 0);
        let iterKey: IterableIterator<string> = concurrentSetTestNull.keys();
        arktest.assertEQ(iterKey.next().value, undefined);
    });
    // Multiple Threads Test
    concurrentSetTestsuite.addTest('Keys&AddTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch(addBack);
        let p2 = launch(keysAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 10);
    });
    concurrentSetTestsuite.addTest('Key&DeteleTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch(deleteAll);
        let p2 = launch(keysAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
    });
    concurrentSetTestsuite.addTest('Key&DeteleTest002', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        deleteAll();
        keysAll();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
    });
    concurrentSetTestsuite.addTest('Key&ClearTest001', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch(keysAll);
        let p2 = launch(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size < 11);
    });
    concurrentSetTestsuite.addTest('Key&ClearTest002', () => {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p = launch(keysAll);
        clearTest();
        p.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
    });

    return concurrentSetTestsuite.run();
}

function keysAll() {
    let iterKeys = concurrentSetTestOne.keys();
    let j = 0;
    for (let i = 0; i < 10; i++) {
        if (iterKeys.next().value == arrKeys[i]) {
            j++;
        }
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
