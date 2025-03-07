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
    let a : char[] = ['a', 'b', 'c', 'd', 'e']
    return a
}

function checkArray(a: char[]): boolean {
    let b : char[] = ['a', 'b', 'x', 'y', 'z']
    return compareCharArrays(a, b) == 0
}

function changeArray(arr: char[]): char[] {
    arr[0] = 'a';
    arr[1] = 'b';
    arr[2] = 'x';
    arr[3] = 'y';
    arr[4] = 'z';
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
    let aChar : char[] = ['\0', '\0', '\n', '\r', '\t', '\b', '\a', '\v', '\f', '\0']
    return aChar
}
