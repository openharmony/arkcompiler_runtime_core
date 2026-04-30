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
        m(): string
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    class A implements I2 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "I2");
    }
}

namespace test2 {
    interface I1 {
        m(): string
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    interface I3 extends I2 {
        m(): string { return "I3" }
    }
    class A implements I3 { }
    class B extends A { }
    class C extends B { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
    }
}

export namespace test3 {
    class A implements dep.test3.I2 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test3.I1).m(new Object()); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    class A implements I2 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "I2");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 extends I1 {
        m(): string
    }
    interface I3 extends I2 {
        m(): string { return "I3" }
    }
    class A implements I3 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "I3");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
