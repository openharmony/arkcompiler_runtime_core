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
        m(): string
    }

    class C1 implements I1 {
        m(): string { return "C1" }
    }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "C1");
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
        m(): string
    }

    class C1 implements I3 {
        m(): string { return "C1" }
    }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "C1");
    }
}

namespace test3 {
    interface A1 { }
    interface A2 { }
    interface A3 extends A1, A2 { }

    interface I1 {
        m(x: A3): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: A1): string
    }

    interface I3 extends I1 {
        m(x: A2): string { return "I3" }
    }

    interface I4 extends I2, I3 { }

    class C1 implements I4 {
        m(x: A1): string { return "C1" }
    }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m({} as A3), "C1");
        arktest.assertEQ((new C1() as I2).m({} as A1), "C1");
        arktest.assertEQ((new C1() as I3).m({} as A2), "I3");
    }
}

namespace test4 {
    interface I {
        m(): string { return "I" }
    }

    class A1 implements I {
        m(): string { return "A1" }
    }

    abstract class A2 implements I {
        abstract m(): string
    }

    interface J extends I {
        m(): string { return "J" }
    }

    interface K extends I {
        m(): string
    }

    class B1 extends A1 implements J { }
    class B2 extends A1 implements J {
        override m(): string { return "B2" }
    }
    class B3 extends A2 implements J {
        m(): string { return "B3" }
    }
    class B4 extends A2 implements K, J {
        m(): string { return "B4" }
    }
    class C implements J { }

    export function run(): void {
        const b1 = new B1();
        arktest.assertEQ((b1 as I).m(), "A1");
        arktest.assertEQ((b1 as J).m(), "A1");

        const b2 = new B2();
        arktest.assertEQ((b2 as I).m(), "B2");
        arktest.assertEQ((b2 as J).m(), "B2");

        const b3 = new B3();
        arktest.assertEQ((b3 as I).m(), "B3");
        arktest.assertEQ((b3 as J).m(), "B3");

        const b4 = new B4();
        arktest.assertEQ((b4 as I).m(), "B4");
        arktest.assertEQ((b4 as J).m(), "B4");
        arktest.assertEQ((b4 as K).m(), "B4");

        const c = new C();
        arktest.assertEQ((c as I).m(), "J");
        arktest.assertEQ((c as J).m(), "J");
    }
}

namespace test5 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class C1 {
        m(x: Object): string { return "B1" }
    }

    class C2 {
        m(x: A1): string { return "B2" }
    }

    abstract class C3 extends C1 implements I1 {
        abstract m(x: Object): string
    }

    abstract class C4 extends C2 implements I1 {
        abstract m(x: Object): string
    }

    class C5 extends C1 implements I1 {
        override m(x: Object): string { return "C5" }
    }

    class C6 extends C2 implements I1 {
        m(x: Object): string { return "C6" }
    }

    class C7 extends C3 {
        m(x: Object): string { return "C7" }
    }

    class C8 extends C4 {
        m(x: Object): string { return "C8" }
    }

    export function run(): void {
        const a1 = new A1();
        arktest.assertEQ((new C5() as I1).m(a1), "C5");
        arktest.assertEQ((new C6() as I1).m(a1), "C6");
        arktest.assertEQ((new C7() as I1).m(a1), "C7");
        arktest.assertEQ((new C8() as I1).m(a1), "C8");
    }
}

export namespace test6 {
    class C6 extends dep.test6.C3 implements dep.test6.I2 { }
    class C10 extends dep.test6.C3 implements dep.test6.I1 { }

    export function run(): void {
        const a1 = new dep.test6.A1();

        arktest.assertEQ((new dep.test6.C4() as dep.test6.I1).m(a1), "C1");
        arktest.assertEQ((new dep.test6.C5() as dep.test6.I1).m(a1), "C2");

        const c6 = new C6();
        arktest.expectThrow(() => { (c6 as dep.test6.C3).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c6 as dep.test6.I2).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);

        arktest.assertEQ((new dep.test6.C7() as dep.test6.I2).m(a1), "I2");

        arktest.assertEQ((new dep.test6.C9() as dep.test6.I1).m(a1), "C1");

        const c10 = new C10();
        arktest.expectThrow(() => { (c10 as dep.test6.C3).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c10 as dep.test6.I1).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
