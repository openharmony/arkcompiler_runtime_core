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
desc: Simultaneously overriding a field from a superclass and implements a property from a superinterface.
assertion: A property can be implemented as a field in different combinations.
---*/

interface I {
    num: number
}

class C {
    num: number = 1
}

class D extends C implements I {
    num: number = 2
}

function main() {
    let c: C = new C();
    let dd: D = new D();
    let cd: C = new D();
    let id: I = new D();

    arktest.assertEQ(c.num, 1)
    arktest.assertEQ(dd.num, 2)
    arktest.assertEQ(cd.num, 2)
    arktest.assertEQ(id.num, 2)
}
