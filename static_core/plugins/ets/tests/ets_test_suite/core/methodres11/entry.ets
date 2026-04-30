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

export namespace test1a {
    class A implements dep.test1a.I2 { }
    class B extends A implements dep.test1a.I3 { }

    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test1a.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test1b {
    class A implements dep.test1b.I2 { }
    class B extends A implements dep.test1b.I3 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as dep.test1b.I2).m(), "I2");
    }
}

export namespace test1c {
    class A implements dep.test1c.I2 { }
    class B extends A implements dep.test1c.I3 { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as dep.test1c.I3).m(), "I3");
    }
}

export namespace test2 {
    class A implements dep.test2.I2 { }

    export function run(): void {
        const a = new A();
        arktest.expectThrow(() => { (a as dep.test2.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (a as dep.test2.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3 {
    class A implements dep.test3.I2 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test3.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (b as dep.test3.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test4 {
    class A implements dep.test4.I2 { }
    class B extends A implements dep.test4.I3 { }

    export function run(): void {
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test4.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5 {
    class A implements dep.test5.I4 { }
    class B extends A { }

    export function run(): void {
        const a = new A();
        arktest.expectThrow(() => { (a as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
        const b = new B();
        arktest.expectThrow(() => { (b as dep.test5.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

test1a.run();
test1b.run();
test1c.run();
test2.run();
test3.run();
test4.run();
test5.run();
