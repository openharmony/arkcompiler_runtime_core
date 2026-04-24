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
    interface I1 { m(): string { return "I1" } }
    interface I2 { m(): string { return "I2" } }
    interface I3 { m(): string { return "I3" } }
    interface I4 { m(): string { return "I4" } }
    interface I5 { m(): string { return "I5" } }
    interface I6 { m(): string { return "I6" } }

    class C implements I1, I2, I3, I4, I5, I6 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I1");
        arktest.assertEQ((c as I2).m(), "I2");
        arktest.assertEQ((c as I3).m(), "I3");
        arktest.assertEQ((c as I4).m(), "I4");
        arktest.assertEQ((c as I5).m(), "I5");
        arktest.assertEQ((c as I6).m(), "I6");
    }
}

namespace test2 {
    interface I1 { m(): string { return "I1" } }
    interface I2 { m(): string { return "I2" } }
    interface I3 { m(): string { return "I3" } }
    interface I4 { m(): string { return "I4" } }
    interface I5 { m(): string { return "I5" } }
    interface I6 { m(): string { return "I6" } }

    class C implements I1, I2, I3, I4, I5, I6 {
        m(): string { return "C" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "C");
        arktest.assertEQ((c as I1).m(), "C");
        arktest.assertEQ((c as I6).m(), "C");
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C();
        arktest.expectThrow(() => { (c as dep.test3.I0).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test3.I1).m(), "I1");
        arktest.assertEQ((c as dep.test3.I2).m(), "I2");
        arktest.assertEQ((c as dep.test3.I3).m(), "I3");
    }
}

namespace test4 {
    interface I0 { m(): string { return "I0" } }
    interface I1 extends I0 { m(): string { return "I1" } }
    interface I2 extends I0 { m(): string { return "I2" } }
    interface I3 extends I0 { m(): string { return "I3" } }

    class C implements I1, I2, I3 {
        m(): string { return "C" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "C");
        arktest.assertEQ((c as I0).m(), "C");
        arktest.assertEQ((c as I1).m(), "C");
        arktest.assertEQ((c as I3).m(), "C");
    }
}

namespace test5 {
    interface I {
        m(): string { return "I.m" }
        n(): string { return "I.n" }
    }

    interface I3 extends I {
        m(): string { return "I3.m" }
    }

    interface I4 extends I {
        n(): string { return "I4.n" }
    }

    class C implements I3, I4 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(), "I3.m");
        arktest.assertEQ((c as I).n(), "I4.n");
        arktest.assertEQ((c as I3).m(), "I3.m");
        arktest.assertEQ((c as I4).n(), "I4.n");
    }
}

export namespace test6 {
    export function run(): void {
        const c = new dep.test6.C();
        arktest.expectThrow(() => { (c as dep.test6.I).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test6.I).n(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test6.I2).m(), "I2.m");
        arktest.assertEQ((c as dep.test6.I2).n(), "I2.n");
        arktest.assertEQ((c as dep.test6.I3).m(), "I3.m");
        arktest.assertEQ((c as dep.test6.I3).n(), "I3.n");
    }
}

namespace test7 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A implements I1 {
        m(): string { return "B" }
    }

    class C extends B implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "B");
        arktest.assertEQ((c as I1).m(), "B");
        arktest.assertEQ((c as I2).m(), "B");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
