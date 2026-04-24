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
    class A2 extends A1 { }
    class B1 { }
    class B2 extends B1 { }

    interface I1 {
        m(x: A1, y: B1): string { return "I1" }
    }

    interface I2 {
        m(x: A2, y: B2): string { return "I2" }
    }

    class C implements I1, I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(new A1(), new B1()), "I1");
        arktest.assertEQ((c as I2).m(new A2(), new B2()), "I2");
    }
}

namespace test2 {
    class A1 { }
    class B1 { }

    interface I1 {
        m(x: A1, y: B1): string { return "I1" }
    }

    interface I2 {
        m(x: Object, y: Object): string { return "I2" }
    }

    class C implements I1, I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(new A1(), new B1()), "I1");
        arktest.assertEQ((c as I2).m(new Object(), new Object()), "I2");
    }
}

namespace test3 {
    class A1 { }
    class B1 { }

    interface I1 {
        m(x: A1, y: B1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object, y: B1): string { return "I2" }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(new A1(), new B1()), "I2");
        arktest.assertEQ((c as I2).m(new A1(), new B1()), "I2");
        arktest.assertEQ((c as I2).m(new Object(), new B1()), "I2");
    }
}

export namespace test4 {
    export function run(): void {
        const c = new dep.test4.C();
        arktest.expectThrow(() => {
            (c as dep.test4.I1).m(new dep.test4.A1(), new dep.test4.B1());
        }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test4.I2).m(new Object(), new dep.test4.B1()), "I2");
        arktest.assertEQ((c as dep.test4.I3).m(new dep.test4.A1(), new Object()), "I3");
    }
}

namespace test5 {
    class A1 { }
    class B1 { }

    interface I {
        m(x: A1, y: B1): string { return "I" }
    }

    class C implements I {
        m(x: Object, y: Object): string { return "C" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(new A1(), new B1()), "C");
        arktest.assertEQ(c.m(new A1(), new B1()), "C");
        arktest.assertEQ(c.m(new Object(), new Object()), "C");
    }
}

namespace test6 {
    class A1 { }
    class B1 { }
    class B2 extends B1 { }

    interface I {
        m(x: A1, y: B1): string { return "I" }
    }

    class C implements I {
        m(x: Object, y: B2): string { return "C" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(new A1(), new B1()), "I");
    }
}

namespace test7 {
    class A1 { }
    class B1 { }

    interface I1 {
        m(x: A1, y: B1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object, y: B1): string { return "I2" }
    }

    interface I3 extends I2 {
        m(x: Object, y: Object): string { return "I3" }
    }

    class C implements I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(new A1(), new B1()), "I3");
        arktest.assertEQ((c as I2).m(new Object(), new B1()), "I3");
        arktest.assertEQ((c as I3).m(new Object(), new Object()), "I3");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
