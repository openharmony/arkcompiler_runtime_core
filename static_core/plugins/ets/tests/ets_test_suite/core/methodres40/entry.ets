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
        m(): string { return "I.m" }
        n(): string { return this.m() }
    }

    class C implements I {
        m(): string { return "C.m" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).n(), "C.m")
        arktest.assertEQ((c as I).m(), "C.m")
        arktest.assertEQ(c.m(), "C.m")
        arktest.assertEQ(c.n(), "C.m")
    }
}

namespace test2 {
    interface I {
        g(): string { return "I.g" }
        m(): string { return "X" + this.g() }
        n(): string { return "Y" + this.m() }
    }

    class C implements I {
        g(): string { return "C.g" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).n(), "YXC.g")
        arktest.assertEQ((c as I).m(), "XC.g")
        arktest.assertEQ((c as I).g(), "C.g")
    }
}

namespace test3 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I1 {
        m(): string { return "I3" }
    }

    class C implements I2, I3 { }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I2).m(), "I2")
        arktest.assertEQ((c as I3).m(), "I3")
    }
}

namespace test4 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I1 {
        m(): string { return "I3" }
    }

    class C implements I2, I3 {
        m(): string { return "C" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ(c.m(), "C")
        arktest.assertEQ((c as I1).m(), "C")
        arktest.assertEQ((c as I2).m(), "C")
        arktest.assertEQ((c as I3).m(), "C")
    }
}

namespace test5 {
    interface I1 {
        m(): string { return "I1" }
    }

    interface I2 extends I1 {
        m(): string { return "I2" }
    }

    interface I3 extends I1 {
        m(): string { return "I3" }
    }

    class C implements I2, I3 {
        m(): string { return "C" }
    }

    class D extends C { }

    export function run(): void {
        const d = new D()
        arktest.assertEQ(d.m(), "C")
        arktest.assertEQ((d as I1).m(), "C")
        arktest.assertEQ((d as I2).m(), "C")
        arktest.assertEQ((d as I3).m(), "C")
    }
}

namespace test6 {
    interface I {
        m(): string { return "I0" }
        m(x: int): string { return "I1:" + x.toString() }
        n(): string { return this.m(42) }
    }

    class C implements I {
        m(): string { return "C0" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).n(), "I1:42")
        arktest.assertEQ((c as I).m(), "C0")
        arktest.assertEQ((c as I).m(5), "I1:5")
    }
}

namespace test7 {
    class A1 { }

    interface I {
        m(x: A1): string { return "I.m" }
        n(): string { return this.m(new A1()) }
    }

    class C implements I {
        m(x: Object): string { return "C" }
    }

    export function run(): void {
        const c = new C()
        arktest.assertEQ((c as I).n(), "C")
        arktest.assertEQ((c as I).m(new A1()), "C")
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
