/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

const dates: long[] = [ -271821, -271820, 1, 1900, 1971, 275759, 275760, 275761];
const check: boolean[] = [false, true, true, true, true, true, true, false];

function main(): int {
    for (let i: int; i< dates.length; i++) {
        let date: Date = new Date(dates[i], 0);
        let dateComplex = new Date(dates[i], 0);
        let vd = date.isDateValid();
        let vdComplex = dateComplex.isDateValid();
        if ((vd || vdComplex) != check[i]) return 1;
    }
    return 0;
}
