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
    }

    abstract class C implements I { }

    export function run(): void {
    }
}

namespace test2 {
    interface I1 {
        m(): string
    }

    interface I2 extends I1 {
        m(): string
    }

    interface I3 extends I1 {
        m(): string
    }

    abstract class C implements I2, I3 { }

    export function run(): void {
    }
}

namespace test3 {
    interface I1 {
        m(): string
    }

    interface I2 extends I1 {
        m(): string
    }

    interface I3 extends I1 {
        m(): string
    }

    abstract class C implements I2, I3 { }

    class D extends C {
        m(): string { return "D" }
    }

    export function run(): void {
        const d = new D();
        arktest.assertEQ(d.m(), "D");
        arktest.assertEQ((d as I1).m(), "D");
        arktest.assertEQ((d as I2).m(), "D");
        arktest.assertEQ((d as I3).m(), "D");
    }
}

export namespace test4 {
    export function run(): void {
        const c = new dep.test4.C();
        arktest.expectThrow(() => { (c as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test4.I2).m(), "I2");
    }
}

namespace test5 {
    interface I {
        m(): string
    }

    class A implements I {
        m(): string { return "A" }
    }

    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "A");
        arktest.assertEQ((b as I).m(), "A");
    }
}

namespace test6 {
    class A1 { }

    interface I {
        m(x: A1): string
    }

    class A implements I {
        m(x: Object): string { return "A" }
    }

    export function run(): void {
        const a = new A();
        arktest.assertEQ((a as I).m(new A1()), "A");
    }
}

namespace test7 {
    class A1 { }

    interface I {
        m(x: A1): string
    }

    class A implements I {
        m(x: A1): string { return "A" }
    }

    export function run(): void {
        const a = new A();
        arktest.assertEQ((a as I).m(new A1()), "A");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
