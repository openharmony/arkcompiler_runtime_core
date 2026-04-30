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
    class A implements dep.test1.I2 { }
    export function run(): void {
        const a = new A();
        arktest.expectThrow(() => { (a as dep.test1.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test2 {
    class A implements dep.test2.I2 { }
    class B extends A { }
    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test2.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3a {
    class A implements dep.test3a.I2 { }
    class B extends A implements dep.test3a.I3 { }
    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test3a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test4a {
    class A implements dep.test4a.I2 { }
    class B extends A implements dep.test4a.I3 { }
    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test4a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5a {
    class A implements dep.test5a.I4 { }
    class B extends A { }
    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test5a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

test1.run();
test2.run();
test3a.run();
test4a.run();
test5a.run();
