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

{% for extends_decl in ["", "extends AParent"] %}

/*---
desc: Accessor to class member that has static method with same name as instance one.
---*/

class AParent { // for testing child class overriding instance methods
    str_instance_parent: string = "parent"

    toString(): string { return "instance_AParent" } // override Object method

    foo_get() { return this.str_instance_parent }

    foo_set(val: string) { this.str_instance_parent = val }
}

class A {{extends_decl}}  {
    static str_static: string = "static"
    str_instance: string = "instance"

    toString(): string { return "instance_A" } // override Object method
    static toString(): string { return "static_A" }

    foo_get() { return this.str_instance }
    static foo_get() { return A.str_static }

    foo_set(val: string) { this.str_instance = val }
    static foo_set(val: string) { A.str_static = val }
}

class B {
    private a_ = new A()
    aa: A = new A() // for testing access through field (baseline)
    private items: A[] = [new A()]  // for testing access through array

    not_accessor() { return this.a_ } // access through regular method (baseline)
    get a() { return this.a_ }
    set a(val: string) { this.a_.foo_set(val) }

    get first(): A { return this.items[0] }
    set first(val: string) { this.items[0].foo_set(val) }
}

class C { // for chained getters and setters
    private b_: B = new B();
    get b(): B { return this.b_ }
    set b(val: string) { this.b_.a = val }
}

function test1() {
    arktest.assertEQ(new A().foo_get(), "instance") // direct instantiation baseline

    let b = new B
    arktest.assertEQ(b.not_accessor().foo_get(), "instance", "assert 1")
    arktest.assertEQ(b.a.foo_get(), "instance", "assert 2")

    let x = b.a
    arktest.assertEQ(x.foo_get(), "instance", "assert 2 - through variable assignment")

    arktest.assertEQ(b.aa.foo_get(), "instance", "assert field access")

    b.not_accessor().foo_set("instance 1")
    arktest.assertEQ(b.not_accessor().foo_get(), "instance 1", "assert 3")

    b.a = "instance 2"
    arktest.assertEQ(b.not_accessor().foo_get(), "instance 2", "assert 4")

    arktest.assertEQ(A.foo_get(), "static", "static get")
    A.foo_set("static 1")
    arktest.assertEQ(A.foo_get(), "static 1", "static set")
}

function test2() {
    let b = new B
    arktest.assertEQ(b.first.foo_get(), "instance", "assert 1 - array element")

    let x = b.first
    arktest.assertEQ(x.foo_get(), "instance", "assert 1 - array element through variable assignment")

    b.first = "instance 3"
    arktest.assertEQ(b.first.foo_get(), "instance 3", "assert 2 - array element")
}

function test3() {
    arktest.assertEQ(new B().a.toString(), "instance_A", "assert toString")
}

function test4() {
    let c = new C
    arktest.assertEQ(c.b.a.foo_get(), "instance", "assert 1 chained")

    c.b.a = "instance 4"
    arktest.assertEQ(c.b.not_accessor().foo_get(), "instance 4", "assert 2 chained")
}

function main() {
    test1()
    test2()
    test3()
    test4()
}
{%- endfor %}
