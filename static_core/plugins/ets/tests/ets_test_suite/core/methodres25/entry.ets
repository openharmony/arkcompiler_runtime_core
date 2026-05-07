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
    interface I {
        m(x: int, y: int): int {
            return (x * y) / 5252;
        }

        n(x: int): int {
            return this.m(5252, x);
        }

        m(x: int): int {
            return x;
        }

        n(x: int, y: int): int {
            return this.m(x + y);
        }
    }

    class C implements I { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ(c.n(1), 1);
        arktest.assertEQ(c.n(1, 1), 2);
    }
}

namespace test2 {
    interface I {
        m(x: int): int { return x * 10 }
        m(x: int, y: int): int { return x + y }
        g(): int { return this.m(3, 4) }
    }

    class C implements I { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(5), 50);
        arktest.assertEQ((c as I).m(3, 4), 7);
        arktest.assertEQ((c as I).g(), 7);
    }
}

namespace test3 {
    interface I {
        m(): int { return 0 }
        m(x: int): int { return x }
        m(x: int, y: int): int { return x + y }
    }

    class C implements I { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m(), 0);
        arktest.assertEQ((c as I).m(1), 1);
        arktest.assertEQ((c as I).m(1, 2), 3);
    }
}

namespace test4 {
    interface I1 {
        m(x: int): int { return x * 2 }
    }

    interface I2 extends I1 {
        m(x: int, y: int): int { return x + y }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(3), 6);
        arktest.assertEQ((c as I2).m(3, 4), 7);
        arktest.assertEQ((c as I2).m(3), 6);
    }
}

namespace test5 {
    interface I1 {
        m(x: int): int { return x + 1 }
    }

    interface I2 {
        m(x: int, y: int): int { return x + y + 1 }
    }

    class C implements I1, I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I1).m(5), 6);
        arktest.assertEQ((c as I2).m(5, 3), 9);
    }
}

namespace test6a {
    interface I1 {
        m(x: int): int { return 10 }
    }

    interface I2 extends I1 {
        m(x: int): int { return 20 }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I2).m(0), 20);
        arktest.assertEQ((c as I1).m(0), 20);
    }
}

namespace test6b {
    interface I1 { }

    interface I3 extends I1 {
        m(x: int, y: int): int { return 30 }
    }

    class C implements I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I3).m(0, 0), 30);
    }
}

namespace test7 {
    interface I {
        m(x: string): string { return "str:" + x }
        m(x: int): int { return x + 1 }
    }

    class C implements I { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I).m("hello"), "str:hello");
        arktest.assertEQ((c as I).m(5), 6);
    }
}

namespace test8a {
    interface I1 {
        m(): string { return "I1m" }
        m(x: int): string { return "I1m1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2m" }
        m(x: int): string { return "I2m1" }
    }

    class C implements I2 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I2).m(), "I2m");
        arktest.assertEQ((c as I2).m(0), "I2m1");
    }
}

namespace test8b {
    interface I1 {
        m(): string { return "I1m" }
        m(x: int): string { return "I1m1" }
    }

    interface I3 extends I1 {
        m(): string { return "I3m" }
        m(x: int): string { return "I3m1" }
    }

    class C implements I3 { }

    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as I3).m(), "I3m");
        arktest.assertEQ((c as I3).m(0), "I3m1");
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6a.run();
test6b.run();
test7.run();
test8a.run();
test8b.run();
