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
        const a1 = new dep.test1.A1();
        arktest.expectThrow(() => { (c as dep.test1.I1).m(a1); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test1.I2).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c as dep.test1.I3).m(a1), "I3");
        arktest.expectThrow(() => { (c as dep.test1.I4).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test2 {
    export function run(): void {
        const cb = new dep.test2.C1();
        arktest.assertEQ((cb as dep.test2.I2).m(), "I2");
        arktest.expectThrow(() => { (cb as dep.test2.I3).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test3.C1() as dep.test3.I1).m(); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test4 {
    export function run(): void {
        arktest.expectThrow(() => { (new dep.test4.C1() as dep.test4.I1).m(); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5 {
    export function run(): void {
        const c3 = new dep.test5.C3();
        arktest.expectThrow(() => { (c3 as dep.test5.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c3 as dep.test5.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c3 as dep.test5.I3).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c3 as dep.test5.I4).m(); }, (e) => e instanceof LinkerAbstractMethodError);

        const c4 = new dep.test5.C4();
        arktest.assertEQ((c4 as dep.test5.I1).m(), "I5");
        arktest.assertEQ((c4 as dep.test5.I2).m(), "I5");
        arktest.assertEQ((c4 as dep.test5.I3).m(), "I5");
        arktest.assertEQ((c4 as dep.test5.I4).m(), "I5");
        arktest.assertEQ((c4 as dep.test5.I5).m(), "I5");

        const c5 = new dep.test5.C5();
        arktest.expectThrow(() => { (c5 as dep.test5.C2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c5 as dep.test5.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);

        const c6 = new dep.test5.C6();
        arktest.assertEQ((c6 as dep.test5.I1).m(), "I5");
        arktest.assertEQ((c6 as dep.test5.I4).m(), "I5");
        arktest.assertEQ((c6 as dep.test5.I5).m(), "I5");

        const c7 = new dep.test5.C7();
        arktest.expectThrow(() => { (c7 as dep.test5.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);

        const c8 = new dep.test5.C8();
        arktest.expectThrow(() => { (c8 as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c8 as dep.test5.I2).m(), "I2");
        arktest.assertEQ((c8 as dep.test5.I3).m(), "I3");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
