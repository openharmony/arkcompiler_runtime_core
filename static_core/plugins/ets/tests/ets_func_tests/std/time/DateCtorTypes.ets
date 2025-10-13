/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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


/*---
desc: Data ctor tests
tags: [compile-only]
---*/


const intYears: int[] = [-300, -72, 1, 1900, 1969, 1970, 1971, 1980, 2000, 2020, 2023, 2024, 2030, 2038, 2039];
const intMonths: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2];
const intHours: int[] =  [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 17, 22];
const intMinutes: int[] = [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56];


function tests_long(): void {

    for(let i: int = 0; i < intYears.length; i++) {
        let cd = new Date(intYears[i].toLong());
        let cd1 = new Date(intYears[i]);
    }

    for(let i: int = 0; i < intMonths.length; i++) {
        let cd = new Date(intMonths[i].toLong());
        let cd1 = new Date(intMonths[i]);
    }

    for(let i: int = 0; i < intHours.length; i++) {
        let cd = new Date(intHours[i].toLong());
        let cd1 = new Date(intHours[i]);
    }

    for(let i: int = 0; i < intMinutes.length; i++) {
        let cd = new Date(intMinutes[i].toLong());
        let cd1 = new Date(intMinutes[i]);
    }
}

function main(): int {
    tests_long()

    return 0;
}
