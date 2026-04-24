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
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    interface I3 {
        m(): string { return "I3" }
    }

    class A implements I1 { }

    class B extends A implements I2 { }

    class C extends B implements I3 { }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I1).m(), "I1")
        arktest.assertEQ((c as I2).m(), "I2")
        arktest.assertEQ((c as I3).m(), "I3")
    }
}

namespace test2 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class A implements I1 { }

    class B extends A implements I2 { }

    class C extends B {
        m(): string { return "C" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I1).m(), "C")
        arktest.assertEQ((c as I2).m(), "C")
        arktest.assertEQ(c.m(), "C")
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class A implements I1 { }

    class B extends A implements I2 {
        m(): string { return "B" }
    }

    class C extends B { }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I1).m(), "B")
        arktest.assertEQ((c as I2).m(), "B")
        arktest.assertEQ(c.m(), "B")
    }
}

namespace test4 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class A implements I1 { }

    class B extends A {
        m(x: Object): string { return "B" }
    }

    export function run(): void {
        const b = new B()
        arktest.assertEQ((b as I1).m(new A1()), "B")
        arktest.assertEQ(b.m(new A1()), "B")
    }
}

namespace test5 {
    interface I1 {
        m(): string
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class A implements I1, I2 { }

    class B extends A {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B()
        arktest.assertEQ((b as I1).m(), "B")
        arktest.assertEQ((b as I2).m(), "B")
        arktest.assertEQ(b.m(), "B")
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
