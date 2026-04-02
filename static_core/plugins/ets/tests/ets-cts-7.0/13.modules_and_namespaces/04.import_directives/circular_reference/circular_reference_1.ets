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
    Circular reference between two modules. Module 1 imports Module 2 and
    vice versa. Both contain classes that reference each other's types.
    Verifies that the class linker resolves circular dependencies correctly.
files: [./circular_reference_2.ets]
---*/

import { C2 } from "./circular_reference_2"

export class C1 {
    x: number = 0
    getC2(): C2 {
        let c2 = new C2()
        c2.y = 10
        return c2
    }
}

function main(): void {
    let c1 = new C1()
    let c2 = c1.getC2()
    arktest.assertEQ(c2.y, 10)

    // C2.getC1() constructs a C1 (from module 1) and sets x = 20 —
    // verifies that C2 correctly instantiates and initialises a type from the other module
    let c1back = c2.getC1()
    arktest.assertEQ(c1back.x, 20)

    // Two-hop round-trip C1→C2→C1→C2: exercises both link edges of the circular dependency
    arktest.assertEQ(c2.getC1().getC2().y, 10)
}
