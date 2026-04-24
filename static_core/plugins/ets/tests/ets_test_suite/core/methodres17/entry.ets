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
        const b = new dep.test1.B();
        arktest.expectThrow(() => { b.m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

namespace test2 {
    interface I1 {
        m(): string { return "I1" }
    }
    class A implements I1 {
        m(): string { return "A" }
    }
    class B extends A { }
    export function run(): void {
        arktest.assertEQ((new B() as I1).m(), "A");
    }
}

namespace test3 {
    interface I1 {
        m(): string
    }
    interface I2 {
        m(): string { return "I2" }
    }
    class A implements I1 {
        m(): string { return "A" }
    }
    class B extends A implements I2 { }
    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "A");
        arktest.assertEQ((b as I2).m(), "A");
    }
}

export namespace test4 {
    export function run(): void {
        const b = new dep.test4.B();
        arktest.expectThrow(() => { (b as dep.test4.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (b as dep.test4.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

namespace test5 {
    interface I1 {
        m(): string
    }
    interface I2 extends I1 {
        m(): string { return "I2" }
    }
    interface I3 extends I1 {
        m(): string { return "I3" }
    }
    class A implements I2 {
        m(): string { return "A" }
    }
    class B extends A implements I3 { }
    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(), "A");
        arktest.assertEQ((b as I2).m(), "A");
        arktest.assertEQ((b as I3).m(), "A");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
