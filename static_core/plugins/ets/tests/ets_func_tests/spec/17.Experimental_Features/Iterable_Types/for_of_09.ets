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
desc: 8.9 for-of Statements. 17.4, 17.5.1 - iterable
tags: []
---*/

function main() : int {
    let tmp : int = 10000;
    let initialArray : int[] = [1,2,3,4,5,6];
    let strangeArray : int[] = [2,4,6];
    for (tmp of initialArray) {
        if (tmp == 2 ) {
            initialArray = strangeArray;
        }
    }

    for (let index = 0; index < strangeArray.length; i++) {
        assertEQ(initialArray[index], strangeArray[index])
    }

    return 0;
}

