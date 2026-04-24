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
    interface I1 { m(): string { return "I1" } }
    interface I2 { m(): string { return "I2" } }
    interface I3 { m(): string { return "I3" } }
    interface I4 { m(): string { return "I4" } }
    interface I5 { m(): string { return "I5" } }
    class C implements I1, I2, I3, I4, I5 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I1");
        arktest.assertEQ((c as I2).m(), "I2");
        arktest.assertEQ((c as I3).m(), "I3");
        arktest.assertEQ((c as I4).m(), "I4");
        arktest.assertEQ((c as I5).m(), "I5");
    }
}

namespace test2 {
    interface I1 { m(): string { return "I1" } }
    interface I2 { m(): string { return "I2" } }
    interface I3 { m(): string { return "I3" } }
    interface I4 { m(): string { return "I4" } }
    interface I5 { m(): string { return "I5" } }
    class C implements I1, I2, I3, I4, I5 {
        m(): string { return "C" }
    }
    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.m(), "C");
        arktest.assertEQ((c as I1).m(), "C");
        arktest.assertEQ((c as I5).m(), "C");
    }
}

namespace test3 {
    interface I1 { m(): string { return "I1" } }
    interface I2 { m(): string { return "I2" } }
    interface I3 { m(): string { return "I3" } }
    interface I4 { m(): string { return "I4" } }
    interface I5 { m(): string { return "I5" } }
    class A implements I1 { }
    class B extends A implements I2 { }
    class C extends B implements I3 { }
    class D extends C implements I4 { }
    class E extends D implements I5 { }
    export function run(): void {
        const e = new E();
        arktest.assertEQ((e as I1).m(), "I1");
        arktest.assertEQ((e as I2).m(), "I2");
        arktest.assertEQ((e as I3).m(), "I3");
        arktest.assertEQ((e as I4).m(), "I4");
        arktest.assertEQ((e as I5).m(), "I5");
    }
}

namespace test4 {
    interface I1 { m(): string { return "I1" } }
    interface I2 { m(): string { return "I2" } }
    interface I3 { m(): string { return "I3" } }
    interface I4 { m(): string { return "I4" } }
    interface I5 { m(): string { return "I5" } }
    class A implements I1 { }
    class B extends A implements I2 { }
    class C extends B implements I3 { }
    class D extends C implements I4, I5 { }
    class E extends D { m(): string { return "E" } }
    export function run(): void {
        const e = new E();
        arktest.assertEQ(e.m(), "E");
        arktest.assertEQ((e as I1).m(), "E");
        arktest.assertEQ((e as I5).m(), "E");
    }
}

namespace test5 {
    interface I1 { m(): string { return "I1" } }
    interface I2 extends I1 { m(): string { return "I2" } }
    interface I3 extends I1 { m(): string { return "I3" } }
    interface I4 extends I3 { m(): string { return "I4" } }
    interface I5 extends I2, I4 { m(): string { return "I5" } }
    class C implements I5 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I5).m(), "I5");
        arktest.assertEQ((c as I4).m(), "I5");
        arktest.assertEQ((c as I3).m(), "I5");
        arktest.assertEQ((c as I2).m(), "I5");
        arktest.assertEQ((c as I1).m(), "I5");
    }
}

export namespace test6 {
    export function run(): void {
        const c = new dep.test6.C();
        arktest.assertEQ((c as dep.test6.I5).m(), "I2");
        arktest.assertEQ((c as dep.test6.I2).m(), "I2");
        arktest.assertEQ((c as dep.test6.I4).m(), "I4");
        arktest.assertEQ((c as dep.test6.I3).m(), "I4");
        arktest.expectThrow(() => { (c as dep.test6.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
