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

function getArray(): char[] {
    let a : char[] = [c'a', c'b', c'c', c'd', c'e']
    return a
}

function checkArray(a: char[]): boolean {
    let b : char[] = [c'a', c'b', c'x', c'y', c'z']
    return compareCharArrays(a, b) == 0
}

function changeArray(arr: FixedArray<char>): FixedArray<char> {
    arr[0] = c'a';
    arr[1] = c'b';
    arr[2] = c'x';
    arr[3] = c'y';
    arr[4] = c'z';
    return arr;
}

function compareCharArrays(a: char[], b: char[]): int {
    for (let i: int = 0; i < a.length; ++i) {
        if (a[i] != b[i]) {
            return 1;
        }
    }
    return 0;
}

function GetSpecialArray(): char[] {
    let aChar : char[] = [c'\0', c'\0', c'\n', c'\r', c'\t', c'\b', c'\a', c'\v', c'\f', c'\0']
    return aChar
}
