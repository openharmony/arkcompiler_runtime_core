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

    class A implements I { }

    class B extends A {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I).m(), "B");
    }
}

namespace test2 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    class A implements I2 { }

    class B extends A {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I2).m(), "B");
        arktest.assertEQ((b as I1).m(), "B");
    }
}

namespace test3 {
    class A1 { }

    interface I {
        m(x: A1): string { return "I" }
    }

    class A implements I { }

    class B extends A {
        m(x: Object): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(new A1()), "B");
        arktest.assertEQ((b as I).m(new A1()), "B");
    }
}

namespace test4 {
    interface I {
        m(): string { return "I" }
    }

    class A implements I { }

    class B extends A {
        m(): string { return "B" }
    }

    class C extends B { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "B");
        arktest.assertEQ((c as I).m(), "B");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
