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

export namespace test1_i1_conflict {
    class C implements dep.test1_i1_conflict.I2, dep.test1_i1_conflict.I3 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test1_i1_conflict.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test1_i2 {
    class C implements dep.test1_i2.I2 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test1_i2.I2).m(), "I2");
    }
}

export namespace test1_i3 {
    class C implements dep.test1_i3.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test1_i3.I3).m(), "I3");
    }
}

export namespace test1_i4 {
    class C implements dep.test1_i4.I4 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test1_i4.I4).m(), "I4");
    }
}

export namespace test2_i1 {
    class C implements dep.test2_i1.I2, dep.test2_i1.I3 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test2_i1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test2_i4 {
    class C implements dep.test2_i4.I5, dep.test2_i4.I6 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test2_i4.I4).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test2_i2 {
    class C implements dep.test2_i2.I2 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test2_i2.I2).m(), "I2");
    }
}

export namespace test2_i3 {
    class C implements dep.test2_i3.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test2_i3.I3).m(), "I3");
    }
}

export namespace test2_i5 {
    class C implements dep.test2_i5.I5 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test2_i5.I5).m(), "I5");
    }
}

export namespace test2_i6 {
    class C implements dep.test2_i6.I6 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test2_i6.I6).m(), "I6");
    }
}

export namespace test3_i1 {
    class C implements dep.test3_i1.I2, dep.test3_i1.I3 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test3_i1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test3_i2 {
    class C extends dep.test3_i2.C { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test3_i2.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3_i3 {
    export function run(): void {
        const c = new dep.test3_i3.C();
        arktest.assertEQ((c as dep.test3_i3.I3).m(), "I3");
    }
}

export namespace test4_i1 {
    class D implements dep.test4_i1.I2, dep.test4_i1.I3, dep.test4_i1.I4 { }
    export function run(): void {
        const d = new D();
        arktest.expectThrow(() => { (d as dep.test4_i1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test4_i2 {
    export function run(): void {
        const d = new dep.test4_i2.D();
        arktest.assertEQ((d as dep.test4_i2.I2).m(), "I2");
    }
}

export namespace test4_i3 {
    export function run(): void {
        const d = new dep.test4_i3.D();
        arktest.assertEQ((d as dep.test4_i3.I3).m(), "I3");
    }
}

export namespace test4_i4 {
    export function run(): void {
        const d = new dep.test4_i4.D();
        arktest.assertEQ((d as dep.test4_i4.I4).m(), "I4");
    }
}

export namespace test5_i1 {
    class C implements dep.test5_i1.I3, dep.test5_i1.I5 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test5_i1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5_i2 {
    class C implements dep.test5_i2.I2 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test5_i2.I2).m(), "I2m");
    }
}

export namespace test5_i3 {
    class C implements dep.test5_i3.I3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test5_i3.I3).m(), "I3m");
    }
}

export namespace test5_j1 {
    class C implements dep.test5_j1.J3, dep.test5_j1.J5 { }
    export function run(): void {
        const c = new C();
        arktest.expectThrow(() => { (c as dep.test5_j1.J1).n(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test5_j2 {
    class C implements dep.test5_j2.J2 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test5_j2.J2).n(), "J2n");
    }
}

export namespace test5_j3 {
    class C implements dep.test5_j3.J3 { }
    export function run(): void {
        const c = new C();
        arktest.assertEQ((c as dep.test5_j3.J3).n(), "J3n");
    }
}

export namespace test6_i1 {
    export function run(): void {
        const c3 = new dep.test6_i1.C3();
        arktest.expectThrow(() => { (c3 as dep.test6_i1.I1).m(); }, (e) => e instanceof LinkerMethodConflictError);
    }
}

export namespace test6_i2 {
    export function run(): void {
        const c3 = new dep.test6_i2.C3();
        arktest.assertEQ((c3 as dep.test6_i2.I2).m(), "I2");
    }
}

export namespace test6_i3 {
    export function run(): void {
        const c3 = new dep.test6_i3.C3();
        arktest.assertEQ((c3 as dep.test6_i3.I3).m(), "I3");
    }
}

test1_i1_conflict.run();
test1_i2.run();
test1_i3.run();
test1_i4.run();
test2_i1.run();
test2_i4.run();
test2_i2.run();
test2_i3.run();
test2_i5.run();
test2_i6.run();
test3_i1.run();
test3_i2.run();
test3_i3.run();
test4_i1.run();
test4_i2.run();
test4_i3.run();
test4_i4.run();
test5_i1.run();
test5_i2.run();
test5_i3.run();
test5_j1.run();
test5_j2.run();
test5_j3.run();
test6_i1.run();
test6_i2.run();
test6_i3.run();
