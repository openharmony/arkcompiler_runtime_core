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
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    class C1 implements I2 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I2");
        arktest.assertEQ((c as I2).m(), "I2");
    }
}

namespace test2 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    interface I3 extends I2 {
        m(): string { return "I3" }
    }
    class A implements I3 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "I3");
        arktest.assertEQ((b as I2).m(), "I3");
        arktest.assertEQ((b as I3).m(), "I3");
    }
}

namespace test3 {
    interface I1 {
        m(x: Object): string { return "I1" }
    }
    interface I2 {
        m(x: string): string { return "I2" }
    }
    class C1 implements I1, I2 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(new Object()), "I1");
        arktest.assertEQ((c as I2).m("hello"), "I2");
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }
    class C1 implements I1, I2 {
        m(): string { return "C1" }
    }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "C1");
        arktest.assertEQ((c as I2).m(), "C1");
        arktest.assertEQ(c.m(), "C1");
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
    class C1 implements I2, I3 {
        m(): string { return "C1" }
    }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "C1");
        arktest.assertEQ((c as I2).m(), "C1");
        arktest.assertEQ((c as I3).m(), "C1");
    }
}

namespace test6 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    class A implements I2 {
        m(): string { return "A" }
    }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "A");
        arktest.assertEQ((b as I2).m(), "A");
        arktest.assertEQ(b.m(), "A");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
