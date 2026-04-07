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
        arktest.expectThrow(() => { (c as dep.test1.I).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test1.I).m(42); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test1.I2).m(), "I2");
        arktest.assertEQ((c as dep.test1.I4).m(42), "I4");
    }
}

export namespace test2 {
    export function run(): void {
        const c = new dep.test2.C();
        arktest.expectThrow(() => { (c as dep.test2.I).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test2.I).m(42), "I.int");
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C();
        arktest.assertEQ((c as dep.test3.I).m(), "C");
        arktest.expectThrow(() => { (c as dep.test3.I).m(42); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test3.I).m("hello"), "I.str");
    }
}

namespace test4 {
    class A1 { }

    interface I {
        m(x: Object): string { return "I.obj" }
        m(x: A1): string { return "I.a1" }
    }

    interface I2 extends I {
        m(x: A1): string { return "I2" }
    }

    interface I3 extends I {
        m(x: A1): string { return "I3" }
    }

    class C implements I2, I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(new Object()), "I.obj");
        arktest.assertEQ((c as I).m(new A1()), "I.obj");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
