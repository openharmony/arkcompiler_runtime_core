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

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    class C implements I2 { }

    class D extends C {
        m(x: Object): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(new A1()), "D");
        arktest.assertEQ(d.m(new Object()), "D");
        arktest.assertEQ((d as I1).m(new A1()), "D");
        arktest.assertEQ((d as I2).m(new A1()), "D");
    }
}

namespace test2 {
    class A1 { }

    interface I1 {
        m(x: A1): string
    }

    interface I2 extends I1 {
        m(x: A1): string { return "I2" }
    }

    interface I3 {
        m(x: A1): string { return "I3" }
    }

    class A implements I2 {
        m(x: A1): string { return "A" }
    }

    class B extends A { }

    class C extends B implements I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(new A1()), "A");
        arktest.assertEQ((c as I1).m(new A1()), "A");
        arktest.assertEQ((c as I2).m(new A1()), "A");
        arktest.assertEQ((c as I3).m(new A1()), "A");
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

    class B extends A {
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

namespace test4 {
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

    class D extends C1 { }

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

namespace test5 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    interface I3 extends I1 {
        m(x: A1): string { return "I3" }
    }

    class C1 implements I2, I3 { }

    class D extends C1 {
        m(x: Object): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(new A1()), "D");
        arktest.assertEQ((d as I1).m(new A1()), "D");
        arktest.assertEQ((d as I2).m(new A1()), "D");
        arktest.assertEQ((d as I3).m(new A1()), "D");
    }
}

export namespace test6 {
    export function run(): void {
        const c = new dep.test6.C();
        arktest.expectThrow(() => { (c as dep.test6.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test6.I2).m(), "I2");
        arktest.assertEQ((c as dep.test6.I3).m(), "I4");
        arktest.assertEQ((c as dep.test6.I4).m(), "I4");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
