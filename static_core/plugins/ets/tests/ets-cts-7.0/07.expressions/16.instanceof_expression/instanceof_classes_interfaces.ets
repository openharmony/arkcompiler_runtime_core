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
    The result of an instanceof expression is true
    if the actual type of evaluated expr is a subtype of T (see Subtyping).
    Otherwise, the result is false.
---*/

interface I {}

interface J {}

class A implements I {}

class B extends A implements J {}

class C {}

class D extends C {}

let a: A
let b: B
let i: I
let j: J
let c: C
let d: D

function main() {
    a = new A()
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a instanceof I)
    arktest.assertFalse(a instanceof B)
    arktest.assertFalse(a instanceof J)

    a = new B()
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a instanceof I)
    arktest.assertTrue(a instanceof B)
    arktest.assertTrue(a instanceof J)

    i = new A()
    arktest.assertTrue(i instanceof A)
    arktest.assertTrue(i instanceof I)
    arktest.assertFalse(i instanceof B)
    arktest.assertFalse(i instanceof J)

    i = new B()
    arktest.assertTrue(i instanceof A)
    arktest.assertTrue(i instanceof I)
    arktest.assertTrue(i instanceof B)
    arktest.assertTrue(i instanceof J)

    b = new B()
    arktest.assertTrue(b instanceof A)
    arktest.assertTrue(b instanceof I)
    arktest.assertTrue(b instanceof B)
    arktest.assertTrue(b instanceof J)

    j = new B()
    arktest.assertTrue(j instanceof A)
    arktest.assertTrue(j instanceof I)
    arktest.assertTrue(j instanceof B)
    arktest.assertTrue(j instanceof J)

    c = new C()
    d = new D()
    arktest.assertTrue(c instanceof C)
    arktest.assertTrue(d instanceof D)
    arktest.assertTrue(d instanceof C)
    arktest.assertFalse(c instanceof D)

    // Local variable tests: same assertions as above, but using local-scope variables.
    // Local variables (unlike globals) go through smart-cast condition checking in the
    // type checker, exercising CheckTestObjectCondition for ETSObjectType-typed expressions.
    let la: A = new A()
    arktest.assertTrue(la instanceof A)
    arktest.assertTrue(la instanceof I)
    arktest.assertFalse(la instanceof B)
    arktest.assertFalse(la instanceof J)

    la = new B()
    arktest.assertTrue(la instanceof A)
    arktest.assertTrue(la instanceof I)
    arktest.assertTrue(la instanceof B)
    arktest.assertTrue(la instanceof J)

    let li: I = new A()
    arktest.assertTrue(li instanceof A)
    arktest.assertTrue(li instanceof I)
    arktest.assertFalse(li instanceof B)
    arktest.assertFalse(li instanceof J)

    li = new B()
    arktest.assertTrue(li instanceof A)
    arktest.assertTrue(li instanceof I)
    arktest.assertTrue(li instanceof B)
    arktest.assertTrue(li instanceof J)

    let lc: C = new C()
    let ld: D = new D()
    arktest.assertTrue(lc instanceof C)
    arktest.assertFalse(lc instanceof D)
    arktest.assertTrue(ld instanceof C)
    arktest.assertTrue(ld instanceof D)
}
