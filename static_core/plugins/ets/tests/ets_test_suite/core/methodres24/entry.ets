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
    class C implements dep.test1.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test1.I3).m(new dep.test1.A1()), "I3");
        arktest.assertEQ((c as dep.test1.I2).m(new dep.test1.A1()), "I2");
        arktest.expectThrow(() => { (c as dep.test1.I1).m(new dep.test1.A1()); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test2_i1 {
    class C implements dep.test2_i1.I2, dep.test2_i1.I3 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test2_i1.I1).m(new dep.test2_i1.A1()); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test2_i2 {
    class C implements dep.test2_i2.I2, dep.test2_i2.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test2_i2.I2).m(new dep.test2_i2.A1()), "I2");
    }
}

export namespace test2_i3 {
    class C implements dep.test2_i3.I2, dep.test2_i3.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test2_i3.I3).m(new dep.test2_i3.A1()), "I3");
    }
}

export namespace test3_i1 {
    class C implements dep.test3_i1.I3, dep.test3_i1.I4, dep.test3_i1.I5 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test3_i1.I1).m(new dep.test3_i1.A1()); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test3_i3 {
    class C implements dep.test3_i3.I3, dep.test3_i3.I4, dep.test3_i3.I5 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test3_i3.I3).m(new dep.test3_i3.A1()), "I3");
    }
}

export namespace test3_i4 {
    class C implements dep.test3_i4.I3, dep.test3_i4.I4, dep.test3_i4.I5 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test3_i4.I4).m(new dep.test3_i4.A1()), "I4");
    }
}

export namespace test3_i5 {
    class C implements dep.test3_i5.I3, dep.test3_i5.I4, dep.test3_i5.I5 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test3_i5.I5).m(new dep.test3_i5.A1()), "I5");
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

namespace test5a {
    class A1 { }
    class A { m(x: Object): string { return "A" } }
    interface I1 { m(x: A1): string { return "I1" } }
    class B extends A implements I1 { }
    export function run(): void { const b = new B(); arktest.assertEQ((b as I1).m(new A1()), "A"); }
}

namespace test5b {
    class A1 { }
    class A { m(x: Object): string { return "A" } }
    interface I1 { m(x: A1): string { return "I1" } }
    interface I2 extends I1 { m(x: Object): string { return "I2" } }
    class B extends A implements I2 { }
    export function run(): void { const b = new B(); arktest.assertEQ((b as I2).m(new A1()), "A"); }
}

namespace test5c {
    class A1 { }
    class A { m(x: Object): string { return "A" } }
    interface I1 { m(x: A1): string { return "I1" } }
    interface I3 extends I1 { m(x: A1): string { return "I3" } }
    class B extends A implements I3 { }
    export function run(): void { const b = new B(); arktest.assertEQ((b as I3).m(new A1()), "A"); }
}

namespace test6a { class D { m(): string { return "D" } } export function run(): void { const d = new D(); arktest.assertEQ(d.m(), "D"); } }
namespace test6b { interface I1 { m(): string { return "I1" } } class D implements I1 { m(): string { return "D" } } export function run(): void { const d = new D(); arktest.assertEQ((d as I1).m(), "D"); } }
namespace test6c { interface I2 { m(): string { return "I2" } } class D implements I2 { m(): string { return "D" } } export function run(): void { const d = new D(); arktest.assertEQ((d as I2).m(), "D"); } }
namespace test6d { interface I3 { m(): string { return "I3" } } class D implements I3 { m(): string { return "D" } } export function run(): void { const d = new D(); arktest.assertEQ((d as I3).m(), "D"); } }

namespace test7a {
    class A1 { }
    interface I1 { m(x: A1): string { return "I1" } }
    interface I3 extends I1 { m(x: A1): string { return "I3" } }
    class D implements I3 { m(x: A1): string { return "D" } }
    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(new A1()), "D");
        arktest.assertEQ((d as I1).m(new A1()), "D");
        arktest.assertEQ((d as I3).m(new A1()), "D");
    }
}

namespace test7b {
    class A1 { }
    interface I1 { m(x: A1): string { return "I1" } }
    interface I2 extends I1 { m(x: Object): string { return "I2" } }
    class D implements I2 { }
    export function run(): void {
        const d = new D();
        arktest.assertEQ((d as I2).m(new A1()), "I2");
    }
}

test1.run();
test2_i1.run();
test2_i2.run();
test2_i3.run();
test3_i1.run();
test3_i3.run();
test3_i4.run();
test3_i5.run();
test4.run();
test5a.run();
test5b.run();
test5c.run();
test6a.run();
test6b.run();
test6c.run();
test6d.run();
test7a.run();
test7b.run();
