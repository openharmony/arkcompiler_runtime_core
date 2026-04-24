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
    interface I2 extends I1 { m(): string { return "I2" } }
    interface I3 extends I2 { m(): string { return "I3" } }
    interface I4 extends I3 { m(): string { return "I4" } }
    interface I5 extends I4 { m(): string { return "I5" } }
    interface I6 extends I5 { m(): string { return "I6" } }
    interface I7 extends I6 { m(): string { return "I7" } }
    class C implements I7 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I7).m(), "I7");
        arktest.assertEQ((c as I6).m(), "I7");
        arktest.assertEQ((c as I5).m(), "I7");
        arktest.assertEQ((c as I4).m(), "I7");
        arktest.assertEQ((c as I3).m(), "I7");
        arktest.assertEQ((c as I2).m(), "I7");
        arktest.assertEQ((c as I1).m(), "I7");
    }
}

namespace test2 {
    interface I1 { m(): string { return "I1" } }
    interface I2 extends I1 { m(): string { return "I2" } }
    interface I3 extends I2 { m(): string { return "I3" } }
    interface I4 extends I3 { m(): string { return "I4" } }
    interface I5 extends I4 { m(): string { return "I5" } }
    interface I6 extends I5 { m(): string { return "I6" } }
    interface I7 extends I6 { m(): string { return "I7" } }
    class A implements I7 { }
    class B extends A { m(): string { return "B" } }
    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I7).m(), "B");
        arktest.assertEQ((b as I4).m(), "B");
        arktest.assertEQ((b as I1).m(), "B");
    }
}

namespace test3 {
    interface I1 { m(): string { return "I1" } }
    interface I2 extends I1 { m(): string { return "I2" } }
    interface I3 extends I2 { m(): string { return "I3" } }
    interface I4 extends I3 { m(): string }
    interface I5 extends I4 { m(): string }
    interface I6 extends I5 { m(): string }
    interface I7 extends I6 { m(): string { return "I7" } }
    interface I8 extends I7 { m(): string { return "I8" } }
    interface I9 extends I8 { m(): string { return "I9" } }
    interface I10 extends I9 { m(): string { return "I10" } }
    class C implements I10 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I10).m(), "I10");
        arktest.assertEQ((c as I1).m(), "I10");
        arktest.assertEQ((c as I4).m(), "I10");
    }
}

export namespace test4 {
    export function run(): void {
        const c = new dep.test4.C();
        arktest.expectThrow(() => { (c as dep.test4.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

namespace test5 {
    interface I1 { m(): string { return "I1" } }
    interface I2 extends I1 { m(): string }
    interface I3 extends I2 { m(): string { return "I3" } }
    class C implements I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

namespace test6 {
    interface I1 { m(): string { return "I1" } }
    interface I2 extends I1 { m(): string }
    interface I3 extends I2 { m(): string { return "I3" } }
    interface I4 extends I3 { m(): string }
    interface I5 extends I4 { m(): string { return "I5" } }
    interface I6 extends I5 { m(): string }
    interface I7 extends I6 { m(): string { return "I7" } }
    interface I8 extends I7 { m(): string { return "I8" } }
    class C implements I8 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I8).m(), "I8");
        arktest.assertEQ((c as I2).m(), "I8");
        arktest.assertEQ((c as I1).m(), "I8");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
