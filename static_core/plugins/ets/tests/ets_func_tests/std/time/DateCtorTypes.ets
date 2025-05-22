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

const doubleYears: double[] = [-300, -72.0, 1.0, 1900.5, 1969.9, 1970.0, 1971.1, 1980.0, 2000.0, 2020.20, 2023.9, 2024.6, 2030.3, 2038.5, 2039.9];
const doubleMonths: double[] = [0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10, 11.11, 0.2, 1.1, 2.2];
const doubleHours: double[] = [0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10, 11.11, 15.5, 17.7, 22.22];
const doubleMinutes: double[] = [0.0, 4.4, 8.8, 12.12, 16.16, 20.20, 24.24, 28.28, 32.32, 36.36, 40.40, 44.44, 48.48, 52.52, 56.56];

function tests_long(): void {

    for(let i: int = 0; i < intYears.length; i++) {
        let cd = new Date(intYears[i] as long);
        let cd1 = new Date(intYears[i]);
    }

    for(let i: int = 0; i < intMonths.length; i++) {
        let cd = new Date(intMonths[i] as long);
        let cd1 = new Date(intMonths[i]);
    }

    for(let i: int = 0; i < intHours.length; i++) {
        let cd = new Date(intHours[i] as long);
        let cd1 = new Date(intHours[i]);
    }

    for(let i: int = 0; i < intMinutes.length; i++) {
        let cd = new Date(intMinutes[i] as long);
        let cd1 = new Date(intMinutes[i]);
    }
}

function tests_double(): void {

    for(let i: int = 0; i < doubleYears.length; i++) {
        let cd = new Date(doubleYears[i] as double);
        let cd1 = new Date(doubleYears[i]);
    }

    for(let i: int = 0; i < doubleMonths.length; i++) {
        let cd = new Date(doubleMonths[i] as double);
        let cd1 = new Date(doubleMonths[i]);
    }

    for(let i: int = 0; i < doubleHours.length; i++) {
        let cd = new Date(doubleHours[i] as double);
        let cd1 = new Date(doubleHours[i]);
    }

    for(let i: int = 0; i < doubleMinutes.length; i++) {
        let cd = new Date(doubleMinutes[i] as double);
        let cd1 = new Date(doubleMinutes[i]);
    }
}

function main(): int {
    tests_long()
    tests_double()

    return 0;
}
