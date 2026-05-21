/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Accessor to class member that has static method with same name as instance one.
---*/

class A {
    static str_static: string = "static"
    str_instance: string = "instance"

    foo_get() { return this.str_instance }
    static foo_get() { return A.str_static }

    foo_set(val: string) { this.str_instance = val }
    static foo_set(val: string) { A.str_static = val }
}

class B {
    private a_ = new A()
    not_accessor() { return this.a_ }
    get a() { return this.a_ }
    set a(val: string) { this.a_.foo_set(val) }
}

function main() {
    let b = new B
    arktest.assertEQ(b.not_accessor().foo_get(), "instance", "assert 1")
    arktest.assertEQ(b.a.foo_get(), "instance", "assert 2")

    b.not_accessor().foo_set("instance 1")
    arktest.assertEQ(b.not_accessor().foo_get(), "instance 1", "assert 3")

    b.a = "instance 2"
    arktest.assertEQ(b.not_accessor().foo_get(), "instance 2", "assert 4")

    arktest.assertEQ(A.foo_get(), "static", "static get")
    A.foo_set("static 1")
    arktest.assertEQ(A.foo_get(), "static 1", "static set")
}
