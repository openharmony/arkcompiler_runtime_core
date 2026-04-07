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

    export interface I4 extends I2, I3 {
        m(): string { throw new Error() }
    }

    export interface I5 extends I2, I3 { }

    export class C1 implements I4 { }
    export class C2 implements I5 {
        m(): string { throw new Error() }
    }
    export class C3 implements I2 { }
    export class C4 implements I1 { }
    export class C5 implements I2, I3 {
        m(): string { throw new Error() }
    }
    export class C6 implements I1, I2 { }

    class A1 { }
    export class C7 extends A1 implements I3 { }
}

export namespace test6 {
    export interface I1 {
        m(): string { throw new Error() }
    }

    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }

    export interface I3 extends I2 {
        m(): string
    }

    export interface I4 extends I3 {
        m(): string { throw new Error() }
    }

    export interface I5 extends I1 {
        m(): string { throw new Error() }
    }

    export interface I6 extends I4, I5 { }

    export interface I7 extends I4, I5 {
        m(): string
    }

    export class C1 implements I4 { }
    export class C2 implements I6 {
        m(): string { throw new Error() }
    }
    export class C3 implements I7 {
        m(): string { throw new Error() }
    }
    export class C4 implements I4, I5 {
        m(): string { throw new Error() }
    }
    export class C5 implements I3, I5 {
        m(): string { throw new Error() }
    }
    export class C6 implements I3 {
        m(): string { throw new Error() }
    }
    export class C7 implements I2, I4 { }
}

export namespace test9 {
    export interface I1 {
        m(): string
    }

    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }

    export interface I3 extends I1 {
        m(): string
    }

    export interface I4 extends I1 {
        m(): string { throw new Error() }
    }

    export interface I5 extends I2, I3, I4 { }

    export class C1 implements I5 {
        m(): string { throw new Error() }
    }
}
