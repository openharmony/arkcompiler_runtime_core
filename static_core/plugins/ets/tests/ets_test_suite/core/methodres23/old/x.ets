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
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I4 extends I1 {
        m(): string { throw new Error() }
    }
    export class C implements I2, I3, I4 {
        m(): string { throw new Error() }
    }
}

export namespace test2 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I4 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I5 extends I4 {
        m(): string { throw new Error() }
    }
    export interface I6 extends I4 {
        m(): string { throw new Error() }
    }
    export class C implements I2, I3, I5, I6 {
        m(): string { throw new Error() }
    }
}

export namespace test3 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export class C implements I2, I3 {
        m(): string { throw new Error() }
    }
}

export namespace test4 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I4 extends I1 {
        m(): string { throw new Error() }
    }
    class C1 implements I2, I3 {
        m(): string { throw new Error() }
    }
    export class D extends C1 implements I4 {
        m(): string { throw new Error() }
    }
}

export namespace test5 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface J1 {
        n(): string { throw new Error() }
    }
    export interface J2 extends J1 {
        n(): string { throw new Error() }
    }
    export interface J3 extends J1 {
        n(): string { throw new Error() }
    }
    export class C implements I2, I3, J2, J3 {
        m(): string { throw new Error() }
        n(): string { throw new Error() }
    }
}

export namespace test6 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    class C1 implements I2, I3 {
        m(): string { throw new Error() }
    }
    class C2 extends C1 {
        m(): string { throw new Error() }
    }
    export class C3 extends C2 {
        m(): string { throw new Error() }
    }
}
