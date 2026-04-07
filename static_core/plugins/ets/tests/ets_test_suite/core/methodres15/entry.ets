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
        const a = new dep.test1.A();
        arktest.expectThrow(() => { (a as dep.test1.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test2 {
    export function run(): void {
        const b = new dep.test2.B();
        arktest.expectThrow(() => { (b as dep.test2.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3 {
    export function run(): void {
        const b = new dep.test3.B();
        arktest.expectThrow(() => { (b as dep.test3.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test4 {
    export function run(): void {
        const b = new dep.test4.B();
        arktest.expectThrow(() => { (b as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5 {
    export function run(): void {
        const b = new dep.test5.B();
        arktest.expectThrow(() => { (b as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
