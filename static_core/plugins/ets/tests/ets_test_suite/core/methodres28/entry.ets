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
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    interface I3 extends I2 {
        m(): string { return "I3" }
    }
    interface I4 extends I3 {
        m(): string { return "I4" }
    }
    interface I5 extends I4 {
        m(): string { return "I5" }
    }
    class C implements I5 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I5).m(), "I5");
        arktest.assertEQ((c as I4).m(), "I5");
        arktest.assertEQ((c as I3).m(), "I5");
        arktest.assertEQ((c as I2).m(), "I5");
        arktest.assertEQ((c as I1).m(), "I5");
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
    interface I4 extends I3 {
        m(): string { return "I4" }
    }
    interface I5 extends I4 {
        m(): string { return "I5" }
    }
    class A implements I5 { }
    class B extends A {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I5).m(), "B");
        arktest.assertEQ((b as I3).m(), "B");
        arktest.assertEQ((b as I1).m(), "B");
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C();
        arktest.expectThrow(() => { (c as dep.test3.I0).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test3.I1).m(), "I1");
        arktest.assertEQ((c as dep.test3.I2).m(), "I2");
        arktest.assertEQ((c as dep.test3.I3).m(), "I3");
        arktest.assertEQ((c as dep.test3.I4).m(), "I4");
    }
}

namespace test4 {
    interface I0 {
        m(): string { return "I0" }
    }
    interface I1 extends I0 {
        m(): string { return "I1" }
    }
    interface I2 extends I0 {
        m(): string { return "I2" }
    }
    interface I3 extends I0 {
        m(): string { return "I3" }
    }
    interface I4 extends I0 {
        m(): string { return "I4" }
    }
    class C implements I1, I2, I3, I4 { }
    class D extends C {
        m(): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(), "D");
        arktest.assertEQ((d as I0).m(), "D");
        arktest.assertEQ((d as I1).m(), "D");
        arktest.assertEQ((d as I4).m(), "D");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }
    class C implements I1, I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I1");
        arktest.assertEQ((c as I2).m(), "I2");
    }
}

namespace test6 {
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

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I1).m(), "B");
        arktest.assertEQ((b as I2).m(), "B");
    }
}

namespace test7 {
    interface I {
        m(): string { return "I" }
    }
    class A implements I { }
    class B extends A {
        m(): string { return "B" }
    }
    class C extends B { }
    class D extends C {
        m(): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(), "D");
        arktest.assertEQ((d as I).m(), "D");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
