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
desc: Ensure that all fields are initialized in the specified scenario
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0679: 'this' cannot be referenced before super() call or this() call.
---*/

class A {
    fld: string

    constructor(a: string) {
        if (a == '1') {
            throw new Error('1')
        } else {
            this.fld = a
        }
    }

}

class B extends A {
    constructor(a: string, b: string) {
        if (this.fld == b) {  // CTE: cannot reference `this` before super() call
            throw new Error('2')
        }
        super(a)
    }
}
