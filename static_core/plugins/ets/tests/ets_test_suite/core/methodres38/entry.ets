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
        m(x: int): int { return 42 }
    }

    class C extends B implements I { }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).m(), "A")
        arktest.assertEQ(c.m(), "A")
        arktest.assertEQ(c.m(1), 42)
    }
}

namespace test2 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A0" }
        m(x: int): string { return "A1" }
    }

    class B extends A implements I { }

    export function run(): void {
        const b = new B()
        arktest.assertEQ((b as I).m(), "A0")
        arktest.assertEQ(b.m(), "A0")
        arktest.assertEQ(b.m(1), "A1")
    }
}

namespace test3 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A0" }
        m(x: int): string { return "A1" }
    }

    class B extends A {
        m(x: int): string { return "B1" }
    }

    class C extends B implements I { }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).m(), "A0")
        arktest.assertEQ(c.m(), "A0")
        arktest.assertEQ(c.m(1), "B1")
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: int): string { return "I2" }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I1).m(), "I1")
        arktest.assertEQ((c as I2).m(), "I1")
        arktest.assertEQ((c as I2).m(1), "I2")
    }
}

namespace test5 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A {
        m(x: int): string { return "B" }
    }

    class C extends B {
        m(): string { return "C" }
    }

    class D extends C implements I { }

    export function run(): void {
        const d = new D()
        arktest.assertEQ((d as I).m(), "C")
        arktest.assertEQ(d.m(), "C")
        arktest.assertEQ(d.m(1), "B")
    }
}

namespace test6 {
    interface I {
        m(): string { return "I0" }
        m(x: int): string { return "I1" }
    }

    class C implements I {
        m(): string { return "C" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).m(), "C")
        arktest.assertEQ((c as I).m(1), "I1")
    }
}

namespace test7 {
    class A1 { }
    class A2 extends A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: A2): string { return "I2" }
    }

    class C implements I2 {
        m(x: A1): string { return "C" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I1).m(new A1()), "C")
        arktest.assertEQ((c as I2).m(new A2()), "C")
        arktest.assertEQ((c as I1).m(new A2()), "C")
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
