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
        const c = new dep.test1.C1();
        arktest.expectThrow(() => { (c as dep.test1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test1.I1).n(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test1.I2).m(), "I2m");
        arktest.assertEQ((c as dep.test1.I2).n(), "I2n");
        arktest.assertEQ((c as dep.test1.I3).m(), "I3m");
        arktest.assertEQ((c as dep.test1.I3).n(), "I3n");
    }
}

namespace test2 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        n(): string { return "I2" }
    }

    class A implements I1 { }
    class B extends A implements I2 { }
    class C extends B { }
    class D extends C { }

    export function run(): void {
        const d = new D();
        arktest.assertEQ((d as I1).m(), "I1");
        arktest.assertEQ((d as I2).n(), "I2");
    }
}

namespace test3 {
    interface I {
        m(): string { return "I" }
    }

    class A implements I {
        m(): string { return "A" }
    }

    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I).m(), "A");
        arktest.assertEQ(b.m(), "A");
    }
}

export namespace test4 {
    export function run(): void {
        const c = new dep.test4.C1();
        arktest.expectThrow(() => { (c as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test4.I1).n(), "I2n");
        arktest.assertEQ((c as dep.test4.I2).m(), "I2m");
        arktest.assertEQ((c as dep.test4.I2).n(), "I2n");
        arktest.assertEQ((c as dep.test4.I3).m(), "I3m");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
