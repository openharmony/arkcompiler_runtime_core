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
    Mandatory call to another same-class constructor that uses the keyword this.
    A compile-time error occurs if: An argument of the call uses this or super.
expected_out: |-
    Semantic error ESE0235: Using this is not allowed in constructor
tags: [compile-only, negative]
---*/

class A {
    _ref: A
    constructor() {
        this._ref = this
    }
    constructor(ref: A) {
        this._ref = ref
    }
    get ref(): A { return this._ref }
    set ref(p: A) { this._ref = p }
}

class B extends A {
    // primary
    constructor(a: A) {
        super(a)
    }
    // secondary
    constructor() {
        this(this.ref)  // CTE, cannot use super or this in arguments
    }
}
