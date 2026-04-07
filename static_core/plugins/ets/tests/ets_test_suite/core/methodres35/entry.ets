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

    interface J {
        m(): string { return "J" }
    }

    class A implements I {
        m(): string { return "A" }
    }

    class B extends A implements J { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "A");
        arktest.assertEQ((b as I).m(), "A");
        arktest.assertEQ((b as J).m(), "A");
    }
}

namespace test2 {
    class A1 { }

    interface I {
        m(x: A1): string { return "I" }
    }

    class A implements I {
        m(x: Object): string { return "A" }
    }

    export function run(): void {
        const a = new A();
        arktest.assertEQ((a as I).m(new A1()), "A");
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class A implements I1, I2 {
        m(): string { return "A" }
    }

    export function run(): void {
        const a = new A();
        arktest.assertEQ(a.m(), "A");
        arktest.assertEQ((a as I1).m(), "A");
        arktest.assertEQ((a as I2).m(), "A");
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
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

namespace test5 {
    interface I {
        m(): string { return "I" }
    }

    class A implements I {
        m(): string { return "A" }
    }

    class B extends A {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I).m(), "B");
    }
}

namespace test6 {
    class A1 { }

    interface I {
        m(x: A1): string { return "I" }
    }

    class A implements I {
        m(x: Object): string { return "A" }
    }

    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I).m(new A1()), "A");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
