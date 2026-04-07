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

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C1 implements I3 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I3");
    }
}

namespace test2 {
    interface I1 {
        m(): string
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    class C1 implements I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I2");
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C1 implements I3 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I3");
    }
}

namespace test4 {
    interface I1 {
        m(): string
    }

    interface I2 extends I1 {
        m(): string
    }

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C1 implements I3 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I3");
    }
}

export namespace test5 {
    export function run(): void {
        const c1 = new dep.test5.C1();
        arktest.assertEQ((c1 as dep.test5.I1).m(), "I4");
        arktest.assertEQ((c1 as dep.test5.I2).m(), "I4");
        arktest.assertEQ((c1 as dep.test5.I3).m(), "I4");
        arktest.assertEQ((c1 as dep.test5.I4).m(), "I4");

        const c2 = new dep.test5.C2();
        arktest.expectThrow(() => { (c2 as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c2 as dep.test5.I2).m(), "I2");
        arktest.assertEQ((c2 as dep.test5.I3).m(), "I3");

        const c3 = new dep.test5.C3();
        arktest.assertEQ((c3 as dep.test5.I1).m(), "I2");
        arktest.assertEQ((c3 as dep.test5.I2).m(), "I2");

        arktest.assertEQ((new dep.test5.C4() as dep.test5.I1).m(), "I1");

        const c5 = new dep.test5.C5();
        arktest.expectThrow(() => { (c5 as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c5 as dep.test5.I2).m(), "I2");
        arktest.assertEQ((c5 as dep.test5.I3).m(), "I3");

        const c6 = new dep.test5.C6();
        arktest.assertEQ((c6 as dep.test5.I1).m(), "I2");
        arktest.assertEQ((c6 as dep.test5.I2).m(), "I2");

        const c7 = new dep.test5.C7();
        arktest.assertEQ((c7 as dep.test5.I1).m(), "I3");
        arktest.assertEQ((c7 as dep.test5.I3).m(), "I3");
    }
}

export namespace test6 {
    export function run(): void {
        const c1 = new dep.test6.C1();
        arktest.assertEQ((c1 as dep.test6.I1).m(), "I4");
        arktest.assertEQ((c1 as dep.test6.I2).m(), "I4");
        arktest.assertEQ((c1 as dep.test6.I4).m(), "I4");

        const c2 = new dep.test6.C2();
        arktest.expectThrow(() => { (c2 as dep.test6.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c2 as dep.test6.I4).m(), "I4");
        arktest.assertEQ((c2 as dep.test6.I5).m(), "I5");

        const c3 = new dep.test6.C3();
        arktest.expectThrow(() => { (c3 as dep.test6.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c3 as dep.test6.I4).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c3 as dep.test6.I5).m(); }, (e) => e instanceof LinkerAbstractMethodError);

        const c4 = new dep.test6.C4();
        arktest.expectThrow(() => { (c4 as dep.test6.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c4 as dep.test6.I4).m(), "I4");
        arktest.assertEQ((c4 as dep.test6.I5).m(), "I5");

        const c5 = new dep.test6.C5();
        arktest.expectThrow(() => { (c5 as dep.test6.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c5 as dep.test6.I5).m(), "I5");

        arktest.expectThrow(() => { (new dep.test6.C6() as dep.test6.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);

        const c7 = new dep.test6.C7();
        arktest.assertEQ((c7 as dep.test6.I1).m(), "I4");
        arktest.assertEQ((c7 as dep.test6.I2).m(), "I4");
        arktest.assertEQ((c7 as dep.test6.I4).m(), "I4");
    }
}

namespace test7 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string
    }

    interface I3 extends I2 {
        m(): string
    }

    interface I4 extends I3 {
        m(): string { return "I4" }
    }

    class C1 implements I4 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I4");
    }
}

namespace test8 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class C1 {
        m(x: Object): string { return "C1" }
    }

    class C2 extends C1 implements I1 {
        m(x: A1): string { return "C2" }
    }

    export function run(): void {
        arktest.assertEQ((new C2() as I1).m(new A1()), "C1");
    }
}

export namespace test9 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test9.C1() as dep.test9.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
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
