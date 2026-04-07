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
        m(x: int): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I4 extends I1 {
        m(x: int): string { throw new Error() }
    }
    export interface I5 extends I1 {
        m(x: int): string { throw new Error() }
    }
    export class C implements I2, I3, I4, I5 {
        m(): string { throw new Error() }
        m(x: int): string { throw new Error() }
    }
}

export namespace test2 {
    export interface I1 {
        m(): string { throw new Error() }
        m(x: int): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export class C implements I2, I3 {
        m(): string { throw new Error() }
    }
}

export namespace test3 {
    export interface I1 {
        m(): string { throw new Error() }
        m(x: int): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I4 extends I1 {
        m(x: int): string { throw new Error() }
    }
    export interface I5 extends I1 {
        m(x: int): string { throw new Error() }
    }
    class C implements I2, I3, I4, I5 { }
    export class D extends C {
        m(): string { throw new Error() }
        m(x: int): string { throw new Error() }
    }
}

export namespace test4 {
    export interface I1 {
        m(): string { throw new Error() }
        m(x: int): string { throw new Error() }
        m(x: string): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    export interface I4 extends I1 {
        m(x: int): string { throw new Error() }
    }
    export class C implements I2, I3, I4 {
        m(): string { throw new Error() }
    }
}
