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
    If signatures are override-compatible, the method of subinterface
    overrides the method of superinterface in the subinterface.
    Otherwise, if the superinterface is direct, the superinterface method
    is inherited by the subinterface.
---*/

class A {}
class B extends A {}

interface Base {
    m(p: B): A           // overridden method
    m(p: string): string // inherited method
}

interface Derived extends Base {
    m(p: A): B           // overriding method
}

class Impl implements Derived {
    m(p: A): B {
        return new B()
    }
    m(p: string): string {
        return p
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Overriding in Interfaces - method inheritance")

    suite.addTest("override-compatible: Derived.m(A) overrides Base.m(B)", () => {
        let d: Derived = new Impl()
        let r: B = d.m(new A())
        arktest.assertTrue(r instanceof B)
    })

    suite.addTest("inherited: Base.m(string) is inherited in Derived", () => {
        let d: Derived = new Impl()
        let r: string = d.m("hello")
        arktest.assertEQ(r, "hello")
    })

    suite.addTest("both methods accessible via Base interface", () => {
        let b: Base = new Impl()
        arktest.assertTrue(b.m(new B()) instanceof A)
        arktest.assertEQ(b.m("test"), "test")
    })

    return suite.run()
}
