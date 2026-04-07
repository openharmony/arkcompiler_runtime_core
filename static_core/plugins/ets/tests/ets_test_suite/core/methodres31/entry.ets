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


namespace test1 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A {
        m(): string { return "B" }
    }

    class C extends B implements I { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "B");
        arktest.assertEQ((c as I).m(), "B");
    }
}

namespace test2 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A {
        m(): string { return "B" }
    }

    class C extends B {
        m(): string { return "C" }
    }

    class D extends C implements I { }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(), "C");
        arktest.assertEQ((d as I).m(), "C");
    }
}

namespace test3 {
    class A1 { }

    interface I {
        m(x: A1): string { return "I" }
    }

    class A implements I {
        m(x: A1): string { return "A" }
    }

    export function run(): void {
        const a = new A();
        arktest.assertEQ((a as I).m(new A1()), "A");
    }
}

namespace test4 {
    class A1 { }

    interface I {
        m(x: Object): string { return "I" }
    }

    class A implements I {
        m(x: Object): string { return "A" }
    }

    class B extends A {
        m(x: A1): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I).m(new Object()), "A");
        arktest.assertEQ((b as I).m(new A1()), "A");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I1 {
        m(): string { return "I3" }
    }

    class C1 implements I2, I3 { }

    class D extends C1 {
        m(): string { return "D" }
    }

    class E extends D {
        m(): string { return "E" }
    }

    export function run(): void {
        const e = new E();
        arktest.assertEQ(e.m(), "E");
        arktest.assertEQ((e as I1).m(), "E");
        arktest.assertEQ((e as I2).m(), "E");
        arktest.assertEQ((e as I3).m(), "E");
    }
}

namespace test6 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A implements I { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "A");
        arktest.assertEQ((b as I).m(), "A");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
