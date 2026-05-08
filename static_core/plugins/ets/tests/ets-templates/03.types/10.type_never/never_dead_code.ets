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
    dead code and never terminator for functions execution (no CTE expected)
tags: [compile-only]
---*/

function foo(): never {
    throw new Error('Never')
}

//dead code for never returned function after never returned function call
function bar(): never {
    foo();
    //unreacheble code
    arktest.assertTrue(false, 'dead code execution')
}

//both statments with never
function bar2(): never {
    if (false) return 1 as never
    foo()
}

//return before never terminator
function bar3(): number {
    if (true) return 1
    foo()
}

function bar4(): string {
    foo()
}
