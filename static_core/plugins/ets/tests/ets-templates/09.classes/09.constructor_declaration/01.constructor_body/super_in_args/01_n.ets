/**
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
    Mandatory call to a superconstructor if a class has an extension clause.
    A compile-time error occurs if: An argument of the call uses this or super.
expected_out: |-
    Syntax error ESY0061: Unexpected super keyword.
tags: [compile-only, negative]
---*/

class A {
    ref: A
    constructor() {
        this.ref = this
    }
    constructor(ref: A) {
        this.ref = ref
    }
}

class B extends A {
    constructor() {
        super(super)  // CTE, cannot use super or this in arguments
    }
}
