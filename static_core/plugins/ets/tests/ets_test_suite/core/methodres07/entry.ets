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
        const c = new dep.test1.C1();
        const a1 = new dep.test1.A1();
        arktest.expectThrow(() => { (c as dep.test1.I1).m(a1); }, (e) => e instanceof LinkerMethodConflictError);
        arktest.expectThrow(() => { (c as dep.test1.I2).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.assertEQ((c as dep.test1.I3).m(a1), "I3");
        arktest.expectThrow(() => { (c as dep.test1.I4).m(a1); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test2_i2 {
    class C1 implements dep.test2_i2.I2, dep.test2_i2.I3 { }

    export function run(): void {
        const cb = new C1();
        arktest.assertEQ((cb as dep.test2_i2.I2).m(), "I2");
    }
}

export namespace test2_i3 {
    class C1 implements dep.test2_i3.I2, dep.test2_i3.I3 { }

    export function run(): void {
        const cb = new C1();
        arktest.expectThrow(() => { (cb as dep.test2_i3.I3).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3 {
    class C1 implements dep.test3.I4 { }

    export function run(): void {
        arktest.expectThrow(() => { (new C1() as dep.test3.I1).m(); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test4 {
    class C1 implements dep.test4.I5 { }

    export function run(): void {
        arktest.expectThrow(() => { (new C1() as dep.test4.I1).m(); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c3_i1 {
    class C3 implements dep.test5_c3_i1.I4 { }
    export function run(): void {
        const c3 = new C3();
        arktest.expectThrow(() => { (c3 as dep.test5_c3_i1.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c3_i2 {
    class C3 implements dep.test5_c3_i2.I4 { }
    export function run(): void {
        const c3 = new C3();
        arktest.expectThrow(() => { (c3 as dep.test5_c3_i2.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c3_i3 {
    class C3 implements dep.test5_c3_i3.I4 { }
    export function run(): void {
        const c3 = new C3();
        arktest.expectThrow(() => { (c3 as dep.test5_c3_i3.I3).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c3_i4 {
    class C3 implements dep.test5_c3_i4.I4 { }
    export function run(): void {
        const c3 = new C3();
        arktest.expectThrow(() => { (c3 as dep.test5_c3_i4.I4).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c4_i1 {
    class C4 implements dep.test5_c4_i1.I5 { }
    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5_c4_i1.I1).m(), "I5");
    }
}

export namespace test5_c4_i2 {
    class C4 implements dep.test5_c4_i2.I5 { }
    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5_c4_i2.I2).m(), "I5");
    }
}

export namespace test5_c4_i3 {
    class C4 implements dep.test5_c4_i3.I5 { }
    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5_c4_i3.I3).m(), "I5");
    }
}

export namespace test5_c4_i4 {
    class C4 implements dep.test5_c4_i4.I5 { }
    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5_c4_i4.I4).m(), "I5");
    }
}

export namespace test5_c4_i5 {
    class C4 implements dep.test5_c4_i5.I5 { }
    export function run(): void {
        const c4 = new C4();
        arktest.assertEQ((c4 as dep.test5_c4_i5.I5).m(), "I5");
    }
}

export namespace test5_c5 {
    class C5 extends dep.test5_c5.C2 implements dep.test5_c5.I1 { }
    export function run(): void {
        const c5 = new C5();
        arktest.expectThrow(() => { (c5 as dep.test5_c5.C2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (c5 as dep.test5_c5.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c6_i1 {
    class C6 implements dep.test5_c6_i1.I5, dep.test5_c6_i1.I1 { }
    export function run(): void {
        const c6 = new C6();
        arktest.assertEQ((c6 as dep.test5_c6_i1.I1).m(), "I5");
    }
}

export namespace test5_c6_i4 {
    class C6 implements dep.test5_c6_i4.I5, dep.test5_c6_i4.I1 { }
    export function run(): void {
        const c6 = new C6();
        arktest.assertEQ((c6 as dep.test5_c6_i4.I4).m(), "I5");
    }
}

export namespace test5_c6_i5 {
    class C6 implements dep.test5_c6_i5.I5, dep.test5_c6_i5.I1 { }
    export function run(): void {
        const c6 = new C6();
        arktest.assertEQ((c6 as dep.test5_c6_i5.I5).m(), "I5");
    }
}

export namespace test5_c7_i1 {
    class C7 implements dep.test5_c7_i1.I4, dep.test5_c7_i1.I1 { }
    export function run(): void {
        const c7 = new C7();
        arktest.expectThrow(() => { (c7 as dep.test5_c7_i1.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5_c8_i1 {
    class C8 extends dep.test5_c8_i1.C1 implements dep.test5_c8_i1.I2, dep.test5_c8_i1.I3 { }
    export function run(): void {
        const c8 = new C8();
        arktest.expectThrow(() => { (c8 as dep.test5_c8_i1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5_c8_i2 {
    class C8 extends dep.test5_c8_i2.C1 implements dep.test5_c8_i2.I2, dep.test5_c8_i2.I3 { }
    export function run(): void {
        const c8 = new C8();
        arktest.assertEQ((c8 as dep.test5_c8_i2.I2).m(), "I2");
    }
}

export namespace test5_c8_i3 {
    class C8 extends dep.test5_c8_i3.C1 implements dep.test5_c8_i3.I2, dep.test5_c8_i3.I3 { }
    export function run(): void {
        const c8 = new C8();
        arktest.assertEQ((c8 as dep.test5_c8_i3.I3).m(), "I3");
    }
}

test1.run();
test2_i2.run();
test2_i3.run();
test3.run();
test4.run();
test5_c3_i1.run();
test5_c3_i2.run();
test5_c3_i3.run();
test5_c3_i4.run();
test5_c4_i1.run();
test5_c4_i2.run();
test5_c4_i3.run();
test5_c4_i4.run();
test5_c4_i5.run();
test5_c5.run();
test5_c6_i1.run();
test5_c6_i4.run();
test5_c6_i5.run();
test5_c7_i1.run();
test5_c8_i1.run();
test5_c8_i2.run();
test5_c8_i3.run();
