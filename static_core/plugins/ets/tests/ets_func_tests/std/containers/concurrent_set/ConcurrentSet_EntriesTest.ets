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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine'];
let strAll: string = 'zero,zero,one,one,two,two,three,three,four,four,five,five,six,six,seven,seven,eight,eight,nine,nine';

let concurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let concurrentSetTestsuite = new arktest.ArkTestsuite('ConcurrentSetEntriesTest');
    // Single Thread Test
    concurrentSetTestsuite.addTest('EntriesTest001', () => {
        let concurrentSetTestNull: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = concurrentSetTestNull.size;
        arktest.assertEQ(res, 0);
        let iterValue: IterableIterator<[string, string]> = concurrentSetTestNull.entries();
        arktest.assertEQ(iterValue.next().value, undefined);
    });
    // Multiple Threads Test
    // Unexpected error: ClassCastError: [Lstd/core/Object; cannot be cast to [Double, Double]|undefined
    /* FIXME:
    concurrentSetTestsuite.addTest("Entries&AddTest001", () =>
    {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addFront();
        let p1 = launch(addBack);
        let p2 = launch(entriesAll);
        p1.Await();
        p2.Await();
        arktest.assertEQ(concurrentSetTestOne.size, 10);
    });
    concurrentSetTestsuite.addTest("Entries&DeteleTest001", () =>
    {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch(deleteAll);
        let p2 = launch(entriesAll);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size < 10);
    });
    concurrentSetTestsuite.addTest("Entries&ClearTest001", () =>
    {
        concurrentSetTestOne.clear();
        arktest.assertEQ(concurrentSetTestOne.size, 0);
        addAll();
        let p1 = launch(entriesAll);
        let p2 = launch(clearTest);
        p1.Await();
        p2.Await();
        arktest.assertTrue(concurrentSetTestOne.size < 10);
    });
    */

    // Unexpected error: ClassCastError: [Lstd/core/Object; cannot be cast to [string, string]|undefined
    /* FIXME:
    concurrentSetTestsuite.addTest("EntriesTest001", () =>
    {
        let ConcurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        arktest.assertEQ(res, 10);
        let iterValues = ConcurrentSetTest.entries();
        let str = "";
        for (let i: int = 0; i < 10; i++) {
            str += iterValues.next().value!.toString();
            if (i < 9){
                str += ",";
            }
        }
        arktest.assertEQ(str, strAll);
        arktest.assertEQ(iterValues.next().value, undefined);
    });
    */

    return concurrentSetTestsuite.run();
}

function entriesAll() {
    let iterEntries = concurrentSetTestOne.entries();
    let i = 0;
    while (iterEntries.next().value != undefined) {
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
