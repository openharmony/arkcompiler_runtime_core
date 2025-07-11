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

function doit(ch : string) : string {
    return ch + ch;
}

function main() : int {
    let tmp : string = 'z';
    let initialString : String = "abcdef";
    let workString : String = "";
    for (tmp of initialString) {
        workString = workString + new String(doit(tmp));
    }

    if (workString == "aabbccddeeff") {
        return 0;
    }
    return 1;
}

