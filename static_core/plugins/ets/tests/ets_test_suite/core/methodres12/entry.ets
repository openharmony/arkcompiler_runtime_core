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

    abstract class A implements I1 {
        abstract m(): string
    }

    class B extends A {
        m(): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ(b.m(), "B");
        arktest.assertEQ((b as I1).m(), "B");
    }
}

export namespace test2 {
    class B extends dep.test2.A { }

    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { b.m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (b as dep.test2.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }

    class A {
        m(): string { return "A" }
    }

    class B extends A implements I1 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "A");
        arktest.assertEQ(b.m(), "A");
    }
}

export namespace test4 {
    class C extends dep.test4.B { }

    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { c.m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c as dep.test4.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

namespace test5 {
    interface I1 {
        m(x: Object): string { return "I1" }
    }

    interface I2 {
        m(x: string): string { return "I2" }
    }

    class A implements I2 {
        m(x: string): string { return "A" }
    }

    class B extends A implements I1 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(new Object()), "I1");
        arktest.assertEQ((b as I2).m("x"), "A");
        arktest.assertEQ(b.m("x"), "A");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
