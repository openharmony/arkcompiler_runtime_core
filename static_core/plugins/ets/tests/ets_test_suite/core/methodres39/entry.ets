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

    interface I {
        m(x: A1): string { return "I" }
    }

    class A {
        m(x: Object): string { return "A" }
    }

    class B extends A implements I {
        m(x: A1): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I).m(new A1()), "A");
        arktest.assertEQ(b.m(new A1()), "B");
        arktest.assertEQ(b.m(new Object()), "A");
    }
}

export namespace test2 {
    export function run(): void {
        const b = new dep.test2.B();
        arktest.expectThrow(() => { (b as dep.test2.I).m(new dep.test2.A1()); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C();
        arktest.expectThrow(() => { (c as dep.test3.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c as dep.test3.I2).m(), "I2");
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

    interface I3 extends I1, I2 {
        m(): string { return "I3" }
    }

    class C implements I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I0).m(), "I3");
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

export namespace test5 {
    export function run(): void {
        const c = new dep.test5.C();
        arktest.expectThrow(() => { (c as dep.test5.I0).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test5.I1).m(), "I1");
        arktest.assertEQ((c as dep.test5.I2).m(), "I2");
    }
}

export namespace test6 {
    export function run(): void {
        const c = new dep.test6.C();
        arktest.expectThrow(() => { (c as dep.test6.I0).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test6.I2).m(), "I2");
    }
}

namespace test7 {
    interface I0 {
        m(): string { return "I0" }
    }

    interface I1 extends I0 {
        m(): string
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I0).m(), "I2");
        arktest.assertEQ((c as I1).m(), "I2");
        arktest.assertEQ((c as I2).m(), "I2");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
