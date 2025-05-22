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
    let ConcurrentSetTestsuite = new ArkTestsuite("ConcurrentSetClearTest");
    // Single Thread Test
    ConcurrentSetTestsuite.addTest("ClearTestOne001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<number> = new ConcurrentSet<number>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrKeys[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
        ConcurrentSetTest.clear();
        res = ConcurrentSetTest.size;
        assertEQ(res, 0);
    });
    ConcurrentSetTestsuite.addTest("ClearTestTwo001", () =>
    {
        let ConcurrentSetTest: ConcurrentSet<string> = new ConcurrentSet<string>();
        let res = ConcurrentSetTest.size;
        assertEQ(res, 0);
        for (let i: int = 0; i < 10 ; i++) {
            ConcurrentSetTest.add(arrValues[i]);
        }
        res = ConcurrentSetTest.size;
        assertEQ(res, 10);
        ConcurrentSetTest.clear();
        res = ConcurrentSetTest.size;
        assertEQ(res, 0);
    });

    return ConcurrentSetTestsuite.run();
}
