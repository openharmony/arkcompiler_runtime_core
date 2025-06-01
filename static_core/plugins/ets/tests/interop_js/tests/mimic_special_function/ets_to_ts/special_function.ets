/**
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

let numArr: Array<number> = new Array<number>(1, 3, 5);
let strArr: Array<string> = new Array<string>('3', '2', '1');
let booArr: Array<boolean> = new Array<boolean>(true, false);

const INSERT_NUMBER1: number = 9999;
const INSERT_NUMBER2: number = 888;
const INSERT_NUMBER3: number = 77;

const INSERT_STRING1: string = '1111';
const INSERT_STRING2: string = '2222';
const INSERT_STRING3: string = '3333';

const INSERT_BOOLEAN1: boolean = true;
const INSERT_BOOLEAN2: boolean = false;

function GetNumArr(idx: number): number {
    return numArr[idx];
}
function GetStrArr(idx: number): string {
    return strArr[idx];
}
function GetBooArr(idx: number): boolean {
    return booArr[idx];
}

