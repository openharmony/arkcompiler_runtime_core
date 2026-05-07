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

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C1 implements I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

namespace test2 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C1 implements I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

namespace test3 {
    interface I1 {
        m(): string
    }

    interface I2 extends I1 { }

    interface I3 extends I2 {
        m(): string { return "I3" }
    }

    class C1 implements I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I3");
        arktest.assertEQ((c as I2).m(), "I3");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

export namespace test4a {
    class C1 implements dep.test4a.I2, dep.test4a.I3 { }

    export function run(): void {
        const c = new C1();
        arktest.expectThrow(() => { (c as dep.test4a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test4b {
    class C1 implements dep.test4b.I2, dep.test4b.I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test4b.I2).m(), "I2");
    }
}

export namespace test4c {
    class C1 implements dep.test4c.I2, dep.test4c.I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as dep.test4c.I3).m(), "I3");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I1 {
        m(): string { return "I3" }
    }

    class C1 implements I2, I3 {
        m(): string { return "C1" }
    }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ(c.m(), "C1");
        arktest.assertEQ((c as I1).m(), "C1");
        arktest.assertEQ((c as I2).m(), "C1");
        arktest.assertEQ((c as I3).m(), "C1");
    }
}

test1.run();
test2.run();
test3.run();
test4a.run();
test4b.run();
test4c.run();
test5.run();
