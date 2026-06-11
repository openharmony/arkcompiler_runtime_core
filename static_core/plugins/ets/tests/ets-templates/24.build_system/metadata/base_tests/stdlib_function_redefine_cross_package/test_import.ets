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
    A consumer imports the redefined 'swap' from the library and calls it.
    The imported (user) swap must be the one resolved, yielding the tuple
    reversal below.
files: [lib_swap.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "(a,1)"
---*/

import { swap } from '@panda/lib/lib_swap'

let p: [int, string] = [1, "a"]
let s: [string, int] = swap<int, string>(p)
console.log("(" + s[0] + "," + s[1] + ")")
