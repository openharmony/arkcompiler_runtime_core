/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

name: issue-13330_7.ets
tags: []
---*/

class Base {

    set base(value: int) {
        accessCounter++;
    }
}

class Derive extends Base {

    set base(value: int) {
        super.base = value;
    }
}

let accessCounter = 0;

function main(): void {
    accessCounter = 0;
    let vv = new Derive();
    vv.base = 42;
    assertEQ( accessCounter, 1)
}
