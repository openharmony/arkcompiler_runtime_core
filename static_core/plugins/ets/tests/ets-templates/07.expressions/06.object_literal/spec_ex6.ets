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
    If a class or an interface has some method implementation,
    then its object literal can either skip providing a new method implementation,
    or provide its own version of the method
---*/

abstract class Base {
    method(): number { return 1 }
}

interface I {
    method(): number { return 3 }
}

function main(): void {
    const x1: Base = {} // Valid literal of type Base
    arktest.assertEQ(x1.method(), 1)

    const x2: Base = { // Valid literal of type compatible to Base
        method(): number { return 2}
    }
    arktest.assertEQ(x2.method(), 2)

    const x3: I = {} // Valid literal of anonymous class type
    arktest.assertEQ(x3.method(), 3)

    const x4: I = { // Valid literal of anonymous class type
        method(): number { return 4 }
    }
    arktest.assertEQ(x4.method(), 4)

}
