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

let arrKeys: number[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
let arrValues: string[] = ['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine'];

let concurrentSetTestOne: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();

function main(): int {
    let concurrentSetTestsuite = new arktest.ArkTestsuite('ConcurrentSetClearTest');
    // Single Thread Test
    concurrentSetTestsuite.addTest('ClearTestOne001', () => {
        let concurrentSetTest: containers.ConcurrentSet<number> = new containers.ConcurrentSet<number>();
        let res = concurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10; i++) {
            concurrentSetTest.add(arrKeys[i]);
        }
        res = concurrentSetTest.size;
        arktest.assertEQ(res, 10);
        concurrentSetTest.clear();
        res = concurrentSetTest.size;
        arktest.assertEQ(res, 0);
    });
    concurrentSetTestsuite.addTest('ClearTestTwo001', () => {
        let concurrentSetTest: containers.ConcurrentSet<string> = new containers.ConcurrentSet<string>();
        let res = concurrentSetTest.size;
        arktest.assertEQ(res, 0);
        for (let i: int = 0; i < 10; i++) {
            concurrentSetTest.add(arrValues[i]);
        }
        res = concurrentSetTest.size;
        arktest.assertEQ(res, 10);
        concurrentSetTest.clear();
        res = concurrentSetTest.size;
        arktest.assertEQ(res, 0);
    });

    return concurrentSetTestsuite.run();
}
