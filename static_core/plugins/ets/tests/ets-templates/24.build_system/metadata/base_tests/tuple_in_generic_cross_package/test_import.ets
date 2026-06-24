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
desc: >-
    Two call sites bind the type parameters T, U of pair / flip / first
    to different concrete combinations.
files: [lib_pair.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "(1,a)"
        - "(a,1)"
        - "1"
        - "(true,3.14)"
---*/

import { pair, flip, first } from '@panda/lib/lib_pair'

let p: [int, string] = pair<int, string>(1, "a")
console.log("(" + p[0] + "," + p[1] + ")")

let s: [string, int] = flip<int, string>(p)
console.log("(" + s[0] + "," + s[1] + ")")

console.log(first<int, string>(p))

let q: [boolean, double] = pair<boolean, double>(true, 3.14)
console.log("(" + q[0] + "," + q[1] + ")")
