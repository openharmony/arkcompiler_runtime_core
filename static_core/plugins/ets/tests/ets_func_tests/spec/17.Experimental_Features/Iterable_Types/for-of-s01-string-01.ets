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
desc: 8.9 for-of Statements. 17.4,17.5.1 - iterable
tags: []
---*/

function main() {
    let ch : char = c'z';
    let index : int = 0;
    let result : int = 0;
    let testStr : String = "a string object";
    let testArray : char[] = [c'a', c' ', c's', c't', c'r', c'i', c'n', c'g', c' ', c'o', c'b', c'j', c'e', c'c', c't'];
    for (ch of testStr) {
        if (ch != testArray[index]) {
            console.log("Incorrect element at index : " + index);
            result++;
        }
        index++;
    }

    assertEQ(index, testStr.length)
    assertEQ(result, 0)
}

