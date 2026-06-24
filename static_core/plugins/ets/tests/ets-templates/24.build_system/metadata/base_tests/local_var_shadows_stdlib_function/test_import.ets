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
    Positive: the spec-legal way to give an imported function the name of a
    std/core function ('swap') across a package boundary. Binding it to a
    local VARIABLE (or a lambda) — rather than a function declaration —
    legally shadows the stdlib 'swap' (spec 4.3: the innermost binding takes
    precedence) and sidesteps the cross-scope overload rule (spec 4.2.1),
    which only forbids two same-name FUNCTION declarations in different
    scopes. The library exports the function under a clean name (doSwap); the
    rename to 'swap' happens entirely in the consumer's value namespace.
    Two forms are exercised: a direct function-typed variable bound to the
    imported generic (explicit type arguments) and a lambda wrapper.
files: [lib_funcs.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "(a,1)"
        - "(9,true)"
---*/

import { doSwap } from '@panda/lib/lib_funcs'

// (1) imported generic function bound to a local variable named 'swap'
let swap: (p: [int, string]) => [string, int] = doSwap<int, string>
let p: [int, string] = [1, "a"]
let s1 = swap(p)
console.log("(" + s1[0] + "," + s1[1] + ")")

// (2) lambda wrapper bound to a local 'swapL'
let swapL: (q: [boolean, int]) => [int, boolean] =
    (q: [boolean, int]): [int, boolean] => doSwap<boolean, int>(q)
let q: [boolean, int] = [true, 9]
let s2 = swapL(q)
console.log("(" + s2[0] + "," + s2[1] + ")")
