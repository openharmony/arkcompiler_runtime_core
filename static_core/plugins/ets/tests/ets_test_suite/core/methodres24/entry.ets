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

export namespace test1 {
    export function run(): void {
        const c = new dep.test1.C();
        arktest.assertEQ((c as dep.test1.I3).m(new dep.test1.A1()), "I3");
        arktest.assertEQ((c as dep.test1.I2).m(new dep.test1.A1()), "I2");
        arktest.expectThrow(() => { (c as dep.test1.I1).m(new dep.test1.A1()); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test2 {
    export function run(): void {
        const c = new dep.test2.C();
        arktest.expectThrow(() => { (c as dep.test2.I1).m(new dep.test2.A1()); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test2.I2).m(new dep.test2.A1()), "I2");
        arktest.assertEQ((c as dep.test2.I3).m(new dep.test2.A1()), "I3");
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C();
        arktest.expectThrow(() => { (c as dep.test3.I1).m(new dep.test3.A1()); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test3.I3).m(new dep.test3.A1()), "I3");
        arktest.assertEQ((c as dep.test3.I4).m(new dep.test3.A1()), "I4");
        arktest.assertEQ((c as dep.test3.I5).m(new dep.test3.A1()), "I5");
    }
}

namespace test4 {
    class A1 { }

    interface I1 {
        m(x: A1): string
    }

    interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(new A1()), "I2");
        arktest.assertEQ((c as I2).m(new A1()), "I2");
    }
}

namespace test5 {
    class A1 { }

    class A {
        m(x: Object): string { return "A" }
    }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    interface I3 extends I1 {
        m(x: A1): string { return "I3" }
    }

    class B extends A implements I2, I3 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(new A1()), "A");
        arktest.assertEQ((b as I2).m(new A1()), "A");
        arktest.assertEQ((b as I3).m(new A1()), "A");
    }
}

namespace test6 {
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

    class D extends C1 {
        m(): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(), "D");
        arktest.assertEQ((d as I1).m(), "D");
        arktest.assertEQ((d as I2).m(), "D");
        arktest.assertEQ((d as I3).m(), "D");
    }
}

namespace test7 {
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
        m(x: A1): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(new A1()), "D");
        arktest.assertEQ((d as I1).m(new A1()), "D");
        arktest.assertEQ((d as I3).m(new A1()), "D");
        arktest.assertEQ((d as I2).m(new A1()), "I2");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
