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
        arktest.expectThrow(() => { (new dep.test1.C1() as dep.test1.I1).m(new dep.test1.A1(), new dep.test1.A1()); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

namespace test2 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class A {
        m(x: Object): string { return "A" }
    }

    class B extends A implements I1 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(new A1()), "A");
        arktest.assertEQ(b.m(new A1()), "A");
    }
}

namespace test3 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class A implements I1 {
        m(x: A1): string { return "A" }
    }

    class B extends A {
        m(x: Object): string { return "B" }
    }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(new A1()), "B");
        arktest.assertEQ(b.m(new A1()), "B");
    }
}

namespace test4 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "narrow" }
        m(x: Object): string { return "broad" }
    }

    interface I2 extends I1 { }

    class C1 implements I2 { }

    export function run(): void {
        const c = new C1();
        const a1 = new A1();
        arktest.assertEQ((c as I1).m(a1), "broad");
        arktest.assertEQ((c as I1).m(new Object()), "broad");
        arktest.assertEQ((c as I2).m(a1), "broad");
    }
}

export namespace test5 {
    class C1 implements dep.test5.I1 { }
    export function run(): void {
        const c = new C1();
        const a1 = new dep.test5.A1();
        arktest.expectThrow(() => { (c as dep.test5.I1).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c as dep.test5.I1).m(new Object()); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
