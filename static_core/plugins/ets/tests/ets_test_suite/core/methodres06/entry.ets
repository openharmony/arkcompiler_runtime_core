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
        m(x: I1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: I2): string { return "I2" }
    }

    interface I3 extends I1 {
        m(x: I3): string { return "I3" }
    }

    class C1 implements I2, I3 {
        m(x: C1): string { return "C1" }
    }

    export function run(): void {
        let obj: C1 = new C1();
        arktest.assertEQ((obj as I1).m(obj), "I1");
        arktest.assertEQ((obj as I2).m(obj), "I2");
        arktest.assertEQ((obj as I3).m(obj), "I3");
        arktest.assertEQ((obj as C1).m(obj), "C1");
    }
}

namespace test2 {
    class A {
        m(): string { return "A" }
    }
    class B extends A { }
    interface I1 {
        m(): string { return "I1" }
    }
    class C1 extends B implements I1 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ(c.m(), "A");
        arktest.assertEQ((c as I1).m(), "A");
    }
}

export namespace test3a {
    export class C1 implements dep.test3a.I2, dep.test3a.I3 { }

    export function run(): void {
        const c = new C1();
        arktest.expectThrow(() => { (c as dep.test3a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test3b {
    export class C1 implements dep.test3b.I2, dep.test3b.I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test3b.I2).m(), "I2");
    }
}

export namespace test3c {
    export class C1 implements dep.test3c.I2, dep.test3c.I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test3c.I3).m(), "I3");
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1m" }
        n(): string { return "I1n" }
    }
    interface I2 extends I1 {
        m(): string { return "I2m" }
    }
    class C1 implements I2 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I2m");
        arktest.assertEQ((c as I1).n(), "I1n");
        arktest.assertEQ((c as I2).m(), "I2m");
    }
}

test1.run();
test2.run();
test3a.run();
test3b.run();
test3c.run();
test4.run();
