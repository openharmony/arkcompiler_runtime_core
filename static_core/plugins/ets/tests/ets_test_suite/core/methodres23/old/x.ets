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

export namespace test1_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export interface I3 extends I1 { m(): string { throw new Error() } }
}

export namespace test1_i1_conflict {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test1_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
}

export namespace test1_i3 {
    export interface I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
}

export namespace test1_i4 {
    export interface I1 { }
    export interface I4 extends I1 { m(): string { throw new Error() } }
}

export namespace test2_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test2_i4 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I4 extends I1 { m(): string { throw new Error() } }
    export interface I5 extends I4 { }
    export interface I6 extends I4 { }
}

export namespace test2_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
}

export namespace test2_i3 {
    export interface I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
}

export namespace test2_i5 {
    export interface I1 { }
    export interface I4 extends I1 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test2_i6 {
    export interface I1 { }
    export interface I4 extends I1 { }
    export interface I6 extends I4 { m(): string { throw new Error() } }
}

export namespace test3_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test3_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string }
    export class C implements I2 { m(): string { throw new Error() } }
}

export namespace test3_i3 {
    export interface I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
    export class C implements I3 { m(): string { throw new Error() } }
}

export namespace test4_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I1 { }
}

export namespace test4_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export class D implements I2 { m(): string { throw new Error() } }
}

export namespace test4_i3 {
    export interface I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
    export class D implements I3 { m(): string { throw new Error() } }
}

export namespace test4_i4 {
    export interface I1 { }
    export interface I4 extends I1 { m(): string { throw new Error() } }
    export class D implements I4 { m(): string { throw new Error() } }
}

export namespace test5_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I2 { }
    export interface I4 extends I1 { }
    export interface I5 extends I4 { }
}

export namespace test5_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
}

export namespace test5_i3 {
    export interface I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
}

export namespace test5_j1 {
    export interface J1 { n(): string { throw new Error() } }
    export interface J2 extends J1 { }
    export interface J3 extends J2 { }
    export interface J4 extends J1 { }
    export interface J5 extends J4 { }
}

export namespace test5_j2 {
    export interface J1 { }
    export interface J2 extends J1 { n(): string { throw new Error() } }
}

export namespace test5_j3 {
    export interface J1 { }
    export interface J3 extends J1 { n(): string { throw new Error() } }
}

export namespace test6_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I3 { }
    export interface I5 extends I2, I4 { }
    export class C3 implements I5 { }
}

export namespace test6_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export class C3 implements I2 { m(): string { throw new Error() } }
}

export namespace test6_i3 {
    export interface I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
    export class C3 implements I3 { m(): string { throw new Error() } }
}
