/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    Call to a superconstructor if a class has an extension clause.
---*/

abstract class A {
    x: int;

    protected constructor(a: int) {
        this.x = a + 1;
    }
}

class B extends A {
    constructor() {
        super(42)
    }

    constructor(a: int) {
        super(a + 1);
    }
}

function main() {
    arktest.assertEQ(new B().x, 43)
    arktest.assertEQ(new B(5).x, 7)

    let b: B = {}
    arktest.assertEQ(b.x, 43)

    b = { x: -1 }
    arktest.assertEQ(b.x, -1)
}
