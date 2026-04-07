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
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }
    class C1 implements I1, I2 { }
    class C2 implements I2, I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I1");
        arktest.assertEQ((new C2() as I1).m(), "I1");
        arktest.assertEQ((new C1() as I2).m(), "I2");
        arktest.assertEQ((new C2() as I2).m(), "I2");
    }
}

namespace test2 {
    interface I1 {
        m(x: Object): string { return "I1" }
    }
    interface I2 {
        m(x: string): string { return "I2" }
    }
    class C1 implements I1, I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new Object()), "I1");
        arktest.assertEQ((new C1() as I2).m("hello"), "I2");
    }
}

export namespace test3 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test3.C1() as dep.test3.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (new dep.test3.C2() as dep.test3.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

namespace test4 {
    interface I1 {
        m(): string
    }
    abstract class A {
        abstract m(): string
    }
    class B extends A implements I1 {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I1).m(), "B");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }
    interface I3 extends I1 {
        m(): string { return "I3" }
    }
    class C1 implements I2, I3 { }
    class C2 implements I3, I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I3");
        arktest.assertEQ((new C1() as I2).m(), "I2");
        arktest.assertEQ((new C1() as I3).m(), "I3");
        arktest.assertEQ((new C2() as I1).m(), "I3");
        arktest.assertEQ((new C2() as I2).m(), "I2");
        arktest.assertEQ((new C2() as I3).m(), "I3");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
