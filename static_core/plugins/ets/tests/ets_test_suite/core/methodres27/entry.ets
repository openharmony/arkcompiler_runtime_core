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
        arktest.expectThrow(() => { (c as dep.test1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test1.I1).m(42); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test1.I2).m(), "I2");
        arktest.assertEQ((c as dep.test1.I4).m(42), "I4");
    }
}

export namespace test2 {
    export function run(): void {
        const c = new dep.test2.C();
        arktest.expectThrow(() => { (c as dep.test2.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test2.I1).m(42), "I1.int");
        arktest.assertEQ((c as dep.test2.I2).m(), "I2");
        arktest.assertEQ((c as dep.test2.I2).m(42), "I1.int");
        arktest.assertEQ((c as dep.test2.I3).m(), "I3");
    }
}

export namespace test3 {
    export function run(): void {
        const d = new dep.test3.D();
        arktest.assertEQ(d.m(), "D");
        arktest.assertEQ((d as dep.test3.I1).m(), "D");
        arktest.expectThrow(() => { (d as dep.test3.I1).m(42); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test4 {
    export function run(): void {
        const c = new dep.test4.C();
        arktest.expectThrow(() => { (c as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test4.I1).m(42), "I4");
        arktest.assertEQ((c as dep.test4.I1).m("hello"), "I1.str");
        arktest.assertEQ((c as dep.test4.I4).m(42), "I4");
    }
}

namespace test5 {
    class A1 { }

    interface I1 {
        m(): string { return "I1.void" }
        m(x: Object): string { return "I1.obj" }
    }

    interface I2 extends I1 {
        m(x: A1): string { return "I2" }
    }

    interface I3 extends I1 {
        m(x: A1): string { return "I3" }
    }

    class C implements I2, I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I1.void");
        arktest.assertEQ((c as I1).m(new A1()), "I1.obj");
    }
}

namespace test6 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
        m(x: Object): string { return "I1.obj" }
    }

    interface I2 extends I1 {
        m(x: A1): string { return "I2" }
    }

    interface I3 extends I1 {
        m(x: A1): string { return "I3" }
    }

    class C1 implements I2, I3 { }

    class D extends C1 {
        m(x: Object): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(new A1()), "D");
        arktest.assertEQ(d.m(new Object()), "D");
        arktest.assertEQ((d as I1).m(new Object()), "D");
        arktest.assertEQ((d as I1).m(new A1()), "D");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
