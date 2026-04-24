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

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I1");
        arktest.assertEQ((c as I2).m(), "I2");
    }
}

export namespace test2 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test2.C1() as dep.test2.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test3 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test3.C1() as dep.test3.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }
    interface I3 {
        m(): string { return "I3" }
    }
    class C1 implements I1, I2, I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(), "I1");
        arktest.assertEQ((c as I2).m(), "I2");
        arktest.assertEQ((c as I3).m(), "I3");
    }
}

export namespace test5 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test5.C1() as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
