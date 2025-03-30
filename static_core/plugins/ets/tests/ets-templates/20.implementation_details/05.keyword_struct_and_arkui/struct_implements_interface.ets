/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Keyword 'struct' can be used as a replacement for the keyword 'class'. Check stuct as class implements an interface.
tags: [compile-only, negative]
---*/

interface Test {
    amount: double
    getHalfAmount(): double
}

struct StructClass implements Test{
    amount: double
    field: int
    constructor(f: int, a: double) {
        this.field = f
        this.amount = a
    }

    getHalfAmount(): double {
        return this.amount / 2
    }

    method(arg: int): int {
        return this.field + arg
    }
}

function main() {
    let structClass = new StructClass(100, 50.5)
    assertEQ(structClass.field, 100)
    assertEQ(structClass.amount, 50.5)
    assertEQ(structClass.method(11), 111)
    assertEQ(structClass.getHalfAmount(), 25.25)
}
