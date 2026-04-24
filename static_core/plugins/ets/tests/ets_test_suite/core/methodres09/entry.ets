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

import * as dep from "@dep/x"

namespace test1 {
    class A1 { }
    class A2 extends A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class C1 {
        m(x: A2): string { return "C1" }
    }

    class C2 extends C1 {
        m(x: Object): string { return "C2" }
    }

    class C3 extends C2 implements I1 { }
    class C4 extends C1 implements I1 { }

    export function run(): void {
        arktest.assertEQ((new C3() as I1).m(new A1()), "C2");
        arktest.assertEQ((new C4() as I1).m(new A1()), "I1");
    }
}

namespace test2 {
    class A1 {
        m(x: A1): string { return "A1" }
    }
    class A2 extends A1 {
        m(x: Object): string { return "A2" }
    }
    interface I1 {
        m(x: A1): string { return "I1" }
    }
    class C1 extends A2 implements I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new A1()), "A2");
    }
}

namespace test3 {
    class A {
        m(): string { return "A" }
    }
    class B extends A {
        m(): string { return "B" }
    }
    class C extends B {
        m(): string { return "C" }
    }
    interface I1 {
        m(): string { return "I1" }
    }
    class C1 extends C implements I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "C");
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string
    }

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C implements I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

export namespace test5 {
    export function run(): void {
        const c = new dep.test5.C();
        arktest.expectThrow(() => { (c as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test5.I3).m(), "I3");
        arktest.assertEQ((c as dep.test5.I5).m(), "I5");
    }
}

namespace test6 {
    interface I {
        m(): string { return "I" }
    }

    interface J {
        m(): string { return "J" }
    }

    class A implements I {
        m(): string { return "A" }
    }

    class B extends A { }

    class C extends B implements J { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "A");
        arktest.assertEQ((c as I).m(), "A");
        arktest.assertEQ((c as J).m(), "A");
    }
}

namespace test7 {
    class A1 { }
    class A2 extends A1 { }

    interface I {
        m(x: A1): string { return "I" }
    }

    class A implements I {
        m(x: Object): string { return "A.obj" }
        m(x: A2): string { return "A.a2" }
    }

    export function run(): void {
        const a = new A();
        arktest.assertEQ((a as I).m(new A1()), "A.obj");
        arktest.assertEQ((a as I).m(new A2()), "A.obj");
    }
}

namespace test8 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A { }

    class C extends B { }

    class D extends C implements I { }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(), "A");
        arktest.assertEQ((d as I).m(), "A");
    }
}

namespace test9 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class A implements I1 { }

    class B extends A implements I2 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "I1");
        arktest.assertEQ((b as I2).m(), "I2");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
test8.run();
test9.run();
