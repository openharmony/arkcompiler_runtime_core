/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Generic interface main
files: [test_lib.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "20"
        - "20"
---*/

import {Rankable, maxOf} from '@panda/lib/test_lib'

class Score implements Rankable<Score> {
    value: int
    constructor(v: int) {
        this.value = v
    }
    compareTo(other: Score): int {
        return this.value - other.value
    }
}

let s1 = new Score(10)
let s2 = new Score(20)
let s3 = new Score(5)
console.log(maxOf(s1, s2).value)
console.log(maxOf(s2, s3).value)
