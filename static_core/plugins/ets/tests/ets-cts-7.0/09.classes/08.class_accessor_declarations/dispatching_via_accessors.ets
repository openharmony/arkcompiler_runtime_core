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
desc: Dispatching via accessor methods and fields of a class member.
---*/

interface I { // for testing class implementing interface methods
    foo_get(): string

    foo_set(val: string): void
}

class AI implements I {
    static str_static: string = "static"
    str_instance: string = "instance"

    foo_get() { return this.str_instance }
    static foo_get() { return AI.str_static }

    foo_set(val: string) { this.str_instance = val }
    static foo_set(val: string) { AI.str_static = val }
}

class BI {
    private a_ = new AI()

    get a() { return this.a_ }
    set a(val: string) { this.a_.foo_set(val) }
}

class A { // most basic case of dispatching
    str_instance: string = "instance"

    foo_get() { return this.str_instance }

    foo_set(val: string) { this.str_instance = val }
}

class B {
    private a_ = new A()

    get a() { return this.a_ }
    set a(val: string) { this.a_.foo_set(val) }
}

class A1 { // instance foo(int) selected by overload resolution, not static foo()
    foo(x: int): int { return x }
    static foo(): string { return "static" }
}

class B1 {
    private a_ = new A1()
    get a() { return this.a_ }
}

class A2 { // same name instance field and static one
    n: int = 1
    static n: int = 99
}

class B2 {
    private a_ = new A2
    aa = new A2 // baseline - regular field access

    private aaa = new A2 // baseline - method access
    bar(): A2 { return this.aaa }

    get a() { return this.a_ }
    set a(val: int) { this.a_.n = val }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("dispatching via accessors");
    {% for case in cases %}
    suite.addTest("test_{{loop.index}}", () => {
        {{case.use|indent}}
    });
    {% endfor %}
    return suite.run()
}
