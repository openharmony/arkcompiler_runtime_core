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
desc: Implicit super() works in an abstract class constructor
assert: >-
    If an abstract class constructor has no explicit super() call, compiler inserts
    super() as the first statement and checks the lowered call in abstract class context.
---*/

class Base {
    x: int

    constructor(x: int) {
        this.x = x
    }

    constructor() {
        this.x = 0
    }
}

abstract class Mid extends Base {
    y: int

    constructor(y: int) {
        this.y = y
    }
}

class Leaf extends Mid {
    constructor() {
        super(42)
    }
}

function main(): void {
    let c = new Leaf()
    arktest.assertEQ(c.x, 0)
    arktest.assertEQ(c.y, 42)
}
