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

const ARRAY_SIZE = 15;
const UNSORTED: Array<Double> = [5, 3, 127, 2, 4, 255, 99, 10, 7, 0, 45, 1, 88, 6, 0xffffffffffffffff];
const SORTED = [0, 1, 2, 3, 4, 5, 6, 7, 10, 45, 88, 99, 127, 255, 0xffffffffffffffff];
const SINGLE = [88];

function checkSorted(a: BigUint64Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        assertEQ(a[i].getLong(), SORTED[i] as long);
    }
}

function sortEmpty(): void {
    let empty: BigUint64Array = new BigUint64Array();
    empty.sort();
}

function sortSingle(): void {
    let single: BigUint64Array = new BigUint64Array(SINGLE);
    single.sort();
    assertEQ(single[0].getLong(), SINGLE[0] as long);
}

function sortNormal(): void {
    let a: BigUint64Array = new BigUint64Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function main(): int {
    assertEQ(UNSORTED.length, SORTED.length);
    const suite = new ArkTestsuite('BigUint64Array.sort() tests')
    suite.addTest('sort an empty BigUint64Array', sortEmpty)
    suite.addTest('sort a single-element BigUint64Array', sortSingle)
    suite.addTest('sort a normal BigUint64Array', sortNormal)
    return suite.run();
}
