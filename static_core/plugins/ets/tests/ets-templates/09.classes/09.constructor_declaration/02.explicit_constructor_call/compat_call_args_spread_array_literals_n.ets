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
desc: A spread array literal cannot be used to cover tuple rest elements in an explicit constructor call.
tags: [negative, compile-only]
expected_out: "Semantic error ESE44192: The function or method being called needs a rest parameter to accept arguments passed via the spread operator."
---*/

class A {
    fld: [Object, Object, int, int]

    constructor(...p: [Object, Object, int, int]) {
        this.fld = [...p]
    }
}

class B extends A {
    constructor(p: int) {
        super(...['a', 'b'], ...[p, p])
    }
}

function main() {
    new B(1)
}
