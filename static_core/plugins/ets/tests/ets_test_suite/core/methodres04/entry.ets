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
        arktest.expectThrow(() => { (new dep.test1.C1() as dep.test1.I1).m(new dep.test1.A1(), new dep.test1.A1()); },
            (e) => e instanceof LinkerMethodConflictError);
    }
}

namespace test2 {
    class A1 { }
    class A2 extends A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    class C1 implements I1 {
        m(x: Object): string { return "C1a" }
    }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new A1()), "C1a");
    }
}

export namespace test3 {
    export function run(): void {
        const c2 = new dep.test3.C2();
        arktest.expectThrow(() => { (c2 as dep.test3.I1).m(new dep.test3.A1(), new dep.test3.A1()); },
            (e) => e instanceof LinkerMethodConflictError);
    }
}

namespace test4 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    abstract class C1 implements I1 {
        abstract m(x: A1): string
    }

    class C2 extends C1 {
        m(x: A1): string { return "C2" }
    }

    export function run(): void {
        const d1 = new C2();
        arktest.assertEQ((d1 as I1).m(new A1()), "C2");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
