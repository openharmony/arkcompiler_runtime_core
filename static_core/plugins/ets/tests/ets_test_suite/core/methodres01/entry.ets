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
    interface I1 {
        m(p: I1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(p: I2): string { return "I2" }
    }

    interface I3 extends I1 {
        m(p: I3): string { return "I3" }
    }

    class C1 implements I2, I3 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(new C1()), "I1");
    }
}

namespace test2 {
    class C1 { }
    class C2 extends C1 { }

    interface I1 {
        m(obj: C1): string { return "I1" }
    }

    interface I2 extends I1 {
        m(obj: C2): string { return "I2" }
    }

    class C3 implements I2 { }

    export function run(): void {
        arktest.assertEQ((new C3() as I1).m(new C1()), "I1");
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }
    interface I2 {
        m(): string { return "I2" }
    }

    class C1 implements I1 { }

    export function run(): void {
        arktest.assertEQ((new C1() as I1).m(), "I1");
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    class C1 implements I1, I2 { }

    export function run(): void {
        const c1 = new C1();
        arktest.assertEQ((c1 as I1).m(), "I1");
        arktest.assertEQ((c1 as I2).m(), "I2");
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 {
        m(): string { return "I2" }
    }

    interface I3 extends I1, I2 { }

    class C1 {
        m(): string { return "C1" }
    }

    class C2 extends C1 { }
    class C3 extends C2 implements I1 { }

    class C4 implements I3 { }

    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as I1).m(), "I1");
        arktest.assertEQ((c4 as I2).m(), "I2");

        arktest.assertEQ((new C3() as I1).m(), "C1");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
