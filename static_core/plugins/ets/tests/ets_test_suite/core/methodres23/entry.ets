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
        const c = new dep.test1.C();
        arktest.expectThrow(() => { (c as dep.test1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test1.I2).m(), "I2");
        arktest.assertEQ((c as dep.test1.I3).m(), "I3");
        arktest.assertEQ((c as dep.test1.I4).m(), "I4");
    }
}

export namespace test2 {
    export function run(): void {
        const c = new dep.test2.C();
        arktest.expectThrow(() => { (c as dep.test2.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test2.I4).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test2.I2).m(), "I2");
        arktest.assertEQ((c as dep.test2.I3).m(), "I3");
        arktest.assertEQ((c as dep.test2.I5).m(), "I5");
        arktest.assertEQ((c as dep.test2.I6).m(), "I6");
    }
}

export namespace test3 {
    export function run(): void {
        const c = new dep.test3.C();
        arktest.expectThrow(() => { (c as dep.test3.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test3.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c as dep.test3.I3).m(), "I3");
    }
}

export namespace test4 {
    export function run(): void {
        const d = new dep.test4.D();
        arktest.expectThrow(() => { (d as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((d as dep.test4.I2).m(), "I2");
        arktest.assertEQ((d as dep.test4.I3).m(), "I3");
        arktest.assertEQ((d as dep.test4.I4).m(), "I4");
    }
}

export namespace test5 {
    export function run(): void {
        const c = new dep.test5.C();
        arktest.expectThrow(() => { (c as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test5.I2).m(), "I2m");
        arktest.assertEQ((c as dep.test5.I3).m(), "I3m");
        arktest.expectThrow(() => { (c as dep.test5.J1).n(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c as dep.test5.J2).n(), "J2n");
        arktest.assertEQ((c as dep.test5.J3).n(), "J3n");
    }
}

export namespace test6 {
    export function run(): void {
        const c3 = new dep.test6.C3();
        arktest.expectThrow(() => { (c3 as dep.test6.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.assertEQ((c3 as dep.test6.I2).m(), "I2");
        arktest.assertEQ((c3 as dep.test6.I3).m(), "I3");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
