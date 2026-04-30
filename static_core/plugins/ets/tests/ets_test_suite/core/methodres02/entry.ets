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
    class A1 { }
    class A2 extends A1 { }

    interface I1 {
        m(x: A2): A1 { return new A1() }
    }

    interface I2 extends I1 {
        m(x: A1): A2 { return new A2() }
    }

    class C1 implements I2 { }

    export function run(): void {
        const c1 = new C1();
        arktest.assertEQ(c1.m(new A2()) instanceof A2, true);
    }
}

namespace test2 {
    class A1 { }

    interface I1 {
        m(x: A1, y: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object, y: Object): string { return "I2" }
    }

    class C1 implements I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new A1(), new A1()), "I2");
    }
}

export namespace test3a {
    export class C1 implements dep.test3a.I4 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test3a.I1).m(new dep.test3a.A1()), "I2");
    }
}

export namespace test3b {
    export class C1 implements dep.test3b.I4 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test3b.I2).m(new dep.test3b.A1()), "I2");
    }
}

export namespace test3c {
    export class C1 implements dep.test3c.I4 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test3c.I3).m(new dep.test3c.A2()), "I3");
    }
}

export namespace test3d {
    export class C1 implements dep.test3d.I4 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test3d.I4).m(new dep.test3d.A1()), "I2");
    }
}

namespace test4 {
    interface A1 { }
    interface A2 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: A2): string { return "I2" }
    }

    class C1 implements I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m({} as A1), "I1");
    }
}

namespace test5 {
    class A1 { }
    class A2 extends A1 { }

    interface I1 {
        m(x: A1, y: A1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(x: Object, y: A2): string { return "I2" }
    }

    class C1 implements I2 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new A1(), new A1()), "I1");
    }
}

export namespace test6 {
    export class C1 implements dep.test6.I2 { }

    export function run(): void {
        const c = new C1();
        arktest.expectThrow(() => { (c as dep.test6.I1).m(new Object()); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

test1.run();
test2.run();
test3a.run();
test3b.run();
test3c.run();
test3d.run();
test4.run();
test5.run();
test6.run();
