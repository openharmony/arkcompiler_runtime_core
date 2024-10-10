/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
desc: 05 Generics
tags: []
---*/

function main() : int {
    let x : Array<String> = new Array<String>();
    for (let i = 0; i < 10; i++) {
        x.push("Uups")
    }

    assert x.length == 10
    x.length = 5
    try {
        x[7] = "Bye";
    } catch (e) {
        return 0;
    }

    return 1;
}
