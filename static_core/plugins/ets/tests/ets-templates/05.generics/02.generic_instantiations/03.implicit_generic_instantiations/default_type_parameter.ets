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
    If a method of a generic class or interface G <T1, ..., Tn> has its own type parameter U
    with default type that equals Ti, and an implicit generic instantiation of this method
    provides no information to infer a type argument, then the type argument correspondent
    to Ti is used as the type argument for U.
---*/

// X1 and X2 are two distinct types
class X1 {}
class X2 {}

class A <T> { // T is the class type parameter
    foo<U = T> (p: U) { return p instanceof X1 || p instanceof X2 } // U is own type parameter with default T
    foo1<U = T> (p: U): U { return p }
    bar<V = T> () { return true }
    bar1<V = T> (p: int) { return true }
}

function spec_examples() {
    // this is based on examples from spec
    let a = new A<X1>

    // implicit instantiation:
    arktest.assertTrue(a.foo(new X1), "foo - type argument is inferred from ``new X1``")
    arktest.assertTrue(a.foo(new X2), "foo - type argument is inferred from ``new X2``")
    arktest.assertTrue(a.foo1(new X1) instanceof X1, "foo1 - compile time: U=X1, return type X1")
    arktest.assertTrue(a.foo1(new X2) instanceof X2, "foo1 - compile time: U=X2, return type X2")
    arktest.assertTrue(a.bar(), "bar - class type argument X1 is used as no other information is provided")
    arktest.assertTrue(a.bar1(0), "bar1 - class type argument X1 is used as no other information is provided")

    // explicit instantiation:
    arktest.assertTrue(a.foo<X1> (new X1), "foo - explicit type argument X1 is used")
    arktest.assertTrue(a.foo<X2> (new X2), "foo - explicit type argument X2 is used")
    arktest.assertTrue(a.bar<X1> (), "bar - explicit type argument X1 is used")
    arktest.assertTrue(a.bar<X2> (), "bar - explicit type argument X2 is used")
}

class B<T> { // T is the class type parameter
    field: T
    constructor (p: T) {
        this.field = p
    }
    bar<V = T> () { return this.field instanceof X1 } // V is own type parameter with default T
}

function test_through_constructor() {
    let b1 = new B<X1>(new X1)
    let b2 = new B<X2>(new X2)

    // implicit instantiation:
    arktest.assertTrue(b1.bar(), "b1 - type argument X1 is used as no other information is provided")
    arktest.assertTrue(!b2.bar(), "b2 - class type argument X2 is used as no other information is provided")

    // explicit instantiation:
    arktest.assertTrue(b1.bar<X1>(), "b1 - explicit type argument X1 is used")
    arktest.assertTrue(b1.bar<X2>(), "b1 - explicit type argument X2 is used")
    arktest.assertTrue(!b2.bar<X1>(), "b2 - explicit type argument X1 is used")
    arktest.assertTrue(!b2.bar<X2>(), "b2 - explicit type argument X2 is used")
}

// X3 and X4 are two distinct types implementing same interface
interface Test {
    test(): int
}
class X3 implements Test {
    test(): int { return 3 }
}
class X4 implements Test {
    test(): int { return 4 }
}

class C <T extends Test> { // T is the class type parameter
    foo<U extends Test = T> (p: U) { // U is own type parameter with default T
        if (p.test() == 3) {
            return p instanceof X3
        }
        if (p.test() == 4) {
            return p instanceof X4
        }
        return false
    }
}

function test_through_interface() {
    let c = new C<X3>

    // implicit instantiation:
    arktest.assertTrue(c.foo(new X3), "foo - type argument is inferred from ``new X3``")
    arktest.assertTrue(c.foo(new X4), "foo - type argument is inferred from ``new X4``")

    // explicit instantiation:
    arktest.assertTrue(c.foo<X3> (new X3), "foo - explicit type argument X3 is used")
    arktest.assertTrue(c.foo<X4> (new X4), "foo - explicit type argument X4 is used")
}

function main() {
    spec_examples()
    test_through_constructor()
    test_through_interface()
}
