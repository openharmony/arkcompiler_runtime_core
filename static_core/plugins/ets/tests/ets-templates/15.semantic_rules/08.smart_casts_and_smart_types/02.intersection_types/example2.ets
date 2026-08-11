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
    The values of intersection type D & I are all values that belong to both D and I.
---*/

interface D {
    foo() {
        return 30.0
    }
}

interface I {
    foo(): number
}

class C implements D, I {}

function main() {
    let c: C = new C()
    let d: D = c as D

    if (d instanceof I) {
        // smart type of 'd' here is of some subtype of 'I' that implements 'D'
        // type expression for this type is D & subtype of I
        arktest.assertEQ(d.foo(), 30.0)
    } else {
        arktest.assertTrue(false, 'Unexpected instanceof expression result')
    }
}
