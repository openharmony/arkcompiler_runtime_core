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

export namespace test1 {
    export class A1 { }

    export interface I1 {
        m(x: A1): string { throw new Error() }
    }

    export interface I2 extends I1 {
        m(x: A1): string { throw new Error() }
    }

    export interface I3 extends I1 {
        m(x: Object): string { throw new Error() }
    }

    export interface I4 extends I2, I3 { }

    export class C1 implements I4 { }
}

export namespace test2_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export interface I3 extends I1 { }
}

export namespace test2_i3 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
}

export namespace test3 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
}

export namespace test4 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I1 { }
    export interface I5 extends I2, I3, I4 { }
}

export namespace test5_c3_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { m(): string { throw new Error() } }
}

export namespace test5_c3_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { m(): string { throw new Error() } }
}

export namespace test5_c3_i3 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
    export interface I4 extends I2, I3 { m(): string { throw new Error() } }
}

export namespace test5_c3_i4 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { m(): string { throw new Error() } }
}

export namespace test5_c4_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c4_i2 {
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c4_i3 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
    export interface I4 extends I2, I3 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c4_i4 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { m(): string { throw new Error() } }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c4_i5 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c5 {
    export abstract class C2 { m(): string { throw new Error() } }
    export interface I1 { m(): string { throw new Error() } }
}

export namespace test5_c6_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c6_i4 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { m(): string { throw new Error() } }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c6_i5 {
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
    export interface I5 extends I4 { m(): string { throw new Error() } }
}

export namespace test5_c7_i1 {
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
}

export namespace test5_c8_i1 {
    export class C1 { }
    export interface I1 { m(): string { throw new Error() } }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test5_c8_i2 {
    export class C1 { }
    export interface I1 { }
    export interface I2 extends I1 { m(): string { throw new Error() } }
    export interface I3 extends I1 { }
}

export namespace test5_c8_i3 {
    export class C1 { }
    export interface I1 { }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { m(): string { throw new Error() } }
}

