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
    export class C1 implements dep.test1.I2, dep.test1.I3 { }

    export function run(): void {
        arktest.assertEQ((new C1() as dep.test1.I1).m(new C1()), "I1");
    }
}

namespace test2 {
    class C1 { }
    class C2 extends C1 { }

    interface I1 {
        m(obj: C1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(obj: C2): string { return "I2" }
    }

    class C3 implements I2 { }

    export function run(): void {
        arktest.assertEQ((new C3() as I1).m(new C1()), "I1");
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }

    class C1 implements I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I1");
    }
}

export namespace test4a {
    export class C1 implements dep.test4a.I1, dep.test4a.I2 { }

    export function run(): void {
        const c1 = new C1();
        arktest.assertEQ((c1 as dep.test4a.I1).m(), "I1");
    }
}

export namespace test4b {
    export class C1 implements dep.test4b.I1, dep.test4b.I2 { }

    export function run(): void {
        const c1 = new C1();
        arktest.assertEQ((c1 as dep.test4b.I2).m(), "I2");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }

    class C1 {
        m(): string { return "C1" }
    }

    class C2 extends C1 { }
    class C3 extends C2 implements I1 { }

    export function run(): void {
        arktest.assertEQ((new C3() as I1).m(), "C1");
    }
}

export namespace test5b {
    export class C4 implements dep.test5b.I3 { }

    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5b.I1).m(), "I1");
    }
}

export namespace test5c {
    export class C4 implements dep.test5c.I3 { }

    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5c.I2).m(), "I2");
    }
}

test1.run();
test2.run();
test3.run();
test4a.run();
test4b.run();
test5.run();
test5b.run();
test5c.run();
