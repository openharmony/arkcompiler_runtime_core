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

export namespace test3a {
    class C implements dep.test3a.I1, dep.test3a.I2 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test3a.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3b {
    class C implements dep.test3b.I1, dep.test3b.I2 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test3b.I2).m(), "I2");
    }
}

namespace test4a {
    interface I0 { m(): string { return "I0" } }
    interface I1 extends I0 { m(): string { return "I1" } }
    interface I3 extends I1 { m(): string { return "I3" } }
    class C implements I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I0).m(), "I3");
    }
}

namespace test4b {
    interface I0 { m(): string { return "I0" } }
    interface I1 extends I0 { m(): string { return "I1" } }
    interface I3 extends I1 { m(): string { return "I3" } }
    class C implements I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I3");
    }
}

namespace test4c {
    interface I0 { m(): string { return "I0" } }
    interface I2 extends I0 { m(): string { return "I2" } }
    interface I3 extends I2 { m(): string { return "I3" } }
    class C implements I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I2).m(), "I3");
    }
}

namespace test4d {
    interface I0 { m(): string { return "I0" } }
    interface I1 extends I0 { m(): string { return "I1" } }
    interface I3 extends I1 { m(): string { return "I3" } }
    class C implements I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

export namespace test5a {
    class C implements dep.test5a.I3 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test5a.I0).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5b {
    class C implements dep.test5b.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test5b.I1).m(), "I1");
    }
}

export namespace test5c {
    class C implements dep.test5c.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test5c.I2).m(), "I2");
    }
}

export namespace test6a {
    class C implements dep.test6a.I1, dep.test6a.I2 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test6a.I0).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test6b {
    class C implements dep.test6b.I1, dep.test6b.I2 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test6b.I2).m(), "I2");
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
test3a.run();
test3b.run();
test4a.run();
test4b.run();
test4c.run();
test4d.run();
test5a.run();
test5b.run();
test5c.run();
test6a.run();
test6b.run();
test7.run();
