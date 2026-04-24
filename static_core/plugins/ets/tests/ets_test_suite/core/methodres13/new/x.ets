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
        m(): string
    }

    export interface I2 extends I1 {
        m(): string
    }

    export interface I3 extends I1 {
        m(): string { return "I3" }
    }

    export interface I4 extends I2, I3 { }

    export class C1 implements I4 { }
}

export namespace test3 {
    export interface I1 {
        m(): string
    }

    export interface I2 extends I1 {
        m(): string
    }

    export interface I3 extends I1 {
        m(): string
    }

    export class C1 implements I2, I3 { }
}

export namespace test4 {
    export interface I1 {
        m(): string
    }

    export interface I2 extends I1 {
        m(): string
    }

    export interface I3 extends I1 {
        m(): string
    }

    export interface I4 extends I2, I3 { }

    export class A implements I4 { }

    export class B extends A { }
}

export namespace test5 {
    export interface I1 {
        m(x: Object): string
    }

    export interface I2 extends I1 {
        m(x: string): string { return "I2" }
    }

    export class C1 implements I2 { }
}
