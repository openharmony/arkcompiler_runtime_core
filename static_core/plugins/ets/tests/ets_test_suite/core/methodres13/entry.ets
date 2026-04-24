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
        const c = new dep.test1.C1();
        arktest.expectThrow(() => { (c as dep.test1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test1.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c as dep.test1.I3).m(), "I3");
    }
}

namespace test2 {
    interface I1 {
        m(x: Object): string
    }

    interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    interface I3 extends I1 {
        m(x: string): string { return "I3" }
    }

    class C1 implements I2, I3 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(new Object()), "I2");
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C1();
        arktest.expectThrow(() => { (c as dep.test3.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test4 {
    export function run(): void {
        const a = new dep.test4.A();
        arktest.expectThrow(() => { (a as dep.test4.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        const b = new dep.test4.B();
        arktest.expectThrow(() => { (b as dep.test4.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5 {
    export function run(): void {
        const c = new dep.test5.C1();
        arktest.expectThrow(() => { (c as dep.test5.I1).m(new Object()); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c as dep.test5.I2).m("hello"), "I2");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
