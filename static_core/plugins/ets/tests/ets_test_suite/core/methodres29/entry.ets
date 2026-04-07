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
    interface I {
        m(): string
        m(x: int): string { return "I" }
    }

    class C implements I {
        m(): string { return "C" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "C");
        arktest.assertEQ(c.m(42), "I");
        arktest.assertEQ((c as I).m(), "C");
        arktest.assertEQ((c as I).m(42), "I");
    }
}

namespace test2 {
    class A1 { }

    interface I {
        m(x: A1): string
        m(x: Object): string { return "I" }
    }

    class C implements I {
        m(x: A1): string { return "C" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(new A1()), "C");
        arktest.assertEQ(c.m(new Object()), "I");
        arktest.assertEQ((c as I).m(new A1()), "C");
        arktest.assertEQ((c as I).m(new Object()), "I");
    }
}

namespace test3 {
    interface I1 {
        m(): string
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I2");
        arktest.assertEQ((c as I2).m(), "I2");
    }
}

export namespace test4 {
    export function run(): void {
        const c = new dep.test4.C();
        arktest.expectThrow(() => { (c as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test4.I2).m(), "I2");
        arktest.assertEQ((c as dep.test4.I3).m(), "I3");
    }
}

export namespace test5 {
    export function run(): void {
        const c = new dep.test5.C();
        arktest.expectThrow(() => { (c as dep.test5.I1).m(new dep.test5.A1()); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test5.I1).m(new Object()), "I1");
        arktest.assertEQ((c as dep.test5.I2).m(new dep.test5.A1()), "I2");
        arktest.assertEQ((c as dep.test5.I2).m(new Object()), "I1");
    }
}

namespace test6 {
    interface I {
        m(): string
        m(x: int): string
    }

    class C implements I {
        m(): string { return "C.void" }
        m(x: int): string { return "C.int" }
    }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(), "C.void");
        arktest.assertEQ((c as I).m(42), "C.int");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
