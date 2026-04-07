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


namespace test1 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "narrow" }
        m(x: Object): string { return "broad" }
    }

    class C1 implements I1 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(new A1()), "broad");
        arktest.assertEQ((c as I1).m(new Object()), "broad");
    }
}

namespace test2 {
    class A1 { }

    interface I1 {
        m(x: A1): string
        m(x: Object): string { return "broad" }
    }

    class C1 implements I1 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(new A1()), "broad");
        arktest.assertEQ((c as I1).m(new Object()), "broad");
    }
}

namespace test3 {
    class A1 { }
    class A2 extends A1 { }

    interface I1 {
        m(x: A2): string { return "narrow2" }
        m(x: A1): string { return "narrow1" }
        m(x: Object): string { return "broad" }
    }

    class C1 implements I1 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(new A2()), "broad");
        arktest.assertEQ((c as I1).m(new A1()), "broad");
        arktest.assertEQ((c as I1).m(new Object()), "broad");
    }
}

namespace test4 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "I1" }
    }

    interface I2 {
        m(x: Object): string { return "I2" }
    }

    class C1 implements I1, I2 { }

    export function run(): void {
        const c = new C1();
        arktest.assertEQ((c as I1).m(new A1()), "I1");
        arktest.assertEQ((c as I2).m(new A1()), "I2");
    }
}

namespace test5 {
    class A1 { }

    interface I1 {
        m(x: A1): string { return "narrow" }
        m(x: Object): string { return "broad" }
    }

    class A implements I1 { }
    class B extends A { }

    export function run(): void {
        const b = new B();
        arktest.assertEQ((b as I1).m(new A1()), "broad");
        arktest.assertEQ((b as I1).m(new Object()), "broad");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
