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
desc: Implicit super() picks the parameterless constructor when both zero-arg and rest-parameter overloads exist
assert: >-
    If constructor body has no explicit super() call, compiler inserts super() as the first statement.
    The parameterless constructor is preferred over the rest-parameter overload.
---*/

class A {
    tag: string

    constructor() {
        this.tag = 'no-arg'
    }

    constructor(...x: string[]) {
        this.tag = 'rest:' + x.length
    }
}

class B extends A {
    constructor() {
        // super() should be added implicitly and pick A.constructor()
    }
}

function main(): void {
    let b = new B()
    arktest.assertEQ(b.tag, 'no-arg')
}
