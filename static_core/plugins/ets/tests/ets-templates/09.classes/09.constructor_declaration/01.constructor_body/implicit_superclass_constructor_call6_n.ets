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
desc: Check that superclass constructor implicitly invoked.
assert: >-
    If the body has no call to a superconstructor, then the compiler
    implicitly adds a superconstructor call with no arguments
    as the very first statement in the body.
    If the superconstructor requires a non-empty list of arguments,
    then a compile-time error occurs.
expected_out: |-
    Semantic error ESE0192: Must call super constructor
tags: [compile-only, negative]
---*/

class A {
    x: string

    constructor(x: string) {
        this.x = x
    }
}

class B extends A {
    constructor(x: string) {  // CTE: no-args superconstructor not found
        this.x = 'B'
    }
}
