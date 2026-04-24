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
        m(): string { return "I1.m" }
    }

    interface I2 {
        n(): string { return "I2.n" }
    }

    class A implements I1 { }

    class B extends A implements I2 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "I1.m");
        arktest.assertEQ((b as I2).n(), "I2.n");
    }
}

namespace test2 {
    interface I {
        m(): string { return "I" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A { }

    class C extends B implements I { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(), "A");
        arktest.assertEQ(c.m(), "A");
    }
}

export namespace test3 {
    export function run(): void {
        const b = new dep.test3.B();
        arktest.expectThrow(() => { (b as dep.test3.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((b as dep.test3.I2).m(), "I2");
        arktest.assertEQ((b as dep.test3.I3).m(), "I3");
    }
}

namespace test4 {
    interface I {
        m(): string { return "I.m0" }
        m(x: int): string { return "I.m1" }
    }

    class C implements I {
        m(): string { return "C.m0" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(), "C.m0");
        arktest.assertEQ((c as I).m(42), "I.m1");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    interface I3 {
        m(): string { return "I3" }
    }

    interface I4 {
        m(): string { return "I4" }
    }

    class A implements I1 { }

    class B extends A implements I2 { }

    class C extends B implements I3 { }

    class D extends C implements I4 { }

    export function run(): void {
        const d = new D();
        arktest.assertEQ((d as I1).m(), "I1");
        arktest.assertEQ((d as I2).m(), "I2");
        arktest.assertEQ((d as I3).m(), "I3");
        arktest.assertEQ((d as I4).m(), "I4");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
