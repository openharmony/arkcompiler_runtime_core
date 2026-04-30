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

export namespace test1b_i1 {
    class C1 implements dep.test1b_i1.I1, dep.test1b_i1.I2 { }
    class C2 implements dep.test1b_i1.I2, dep.test1b_i1.I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as dep.test1b_i1.I1).m(), "I1");
        arktest.assertEQ((new C2() as dep.test1b_i1.I1).m(), "I1");
    }
}

export namespace test1b_i2 {
    class C1 implements dep.test1b_i2.I1, dep.test1b_i2.I2 { }
    class C2 implements dep.test1b_i2.I2, dep.test1b_i2.I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as dep.test1b_i2.I2).m(), "I2");
        arktest.assertEQ((new C2() as dep.test1b_i2.I2).m(), "I2");
    }
}

namespace test2a {
    interface I1 {
        m(x: Object): string { return "I1" }
    }
    class C1 implements I1 {
        m(x: Object): string { return "I1" }
    }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new Object()), "I1");
    }
}

namespace test2b {
    interface I2 {
        m(x: string): string { return "I2" }
    }
    class C1 implements I2 {
        m(x: string): string { return "I2" }
    }

    export function run(): void {
        arktest.assertEQ((new C1() as I2).m("hello"), "I2");
    }
}

export namespace test3a {
    class C1 implements dep.test3a.I4a { }

    export function run(): void {
        arktest.expectThrow(() => { (new C1() as dep.test3a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test3b {
    class C2 implements dep.test3b.I4b { }

    export function run(): void {
        arktest.expectThrow(() => { (new C2() as dep.test3b.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
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

export namespace test5a {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }
    interface I3 extends I1 {
        m(): string { return "I3" }
    }
    class C1 implements I3 { }
    class C2 implements I3 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I3");
        arktest.assertEQ((new C1() as I3).m(), "I3");
        arktest.assertEQ((new C2() as I1).m(), "I3");
        arktest.assertEQ((new C2() as I3).m(), "I3");
    }
}

export namespace test5b {
    class C1 implements dep.test5b.I2, dep.test5b.I3 { }
    class C2 implements dep.test5b.I3, dep.test5b.I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as dep.test5b.I2).m(), "I2");
        arktest.assertEQ((new C2() as dep.test5b.I2).m(), "I2");
    }
}

test1b_i1.run();
test1b_i2.run();
test2a.run();
test2b.run();
test3a.run();
test3b.run();
test4.run();
test5a.run();
test5b.run();
