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

function main() : int {
    let tmp : string = "hello world";
    let index : int = 0;
    let initialArray : string[] = ["1.0","2.0","3.0","4.0","5.0","6.0"];
    let expectedArray : string[] = ["1.0","2.0","3.0","4.0","5.0","6.0"];
    let result : int = 0;

    for (tmp of initialArray) {
        if (tmp != expectedArray[index]) {
            console.log("Incorrect element at index : " + index);
            result++;
        }
        index++;
    }


    assertEQ(index, expectedArray.length)
    assertEQ(result, 0)
    return 0;
}

