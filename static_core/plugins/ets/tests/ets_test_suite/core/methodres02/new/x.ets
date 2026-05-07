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

export namespace test3a {
    export class A1 { }
    export class A2 extends A1 { }

    export interface I1 {
        m(x: A1): string { return "I1" }
    }

    export interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    export interface I3 extends I1 {
        m(x: A2): string { return "I3" }
    }

    export interface I4 extends I2, I3 { }
}

export namespace test3b {
    export class A1 { }
    export class A2 extends A1 { }

    export interface I1 {
        m(x: A1): string { return "I1" }
    }

    export interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    export interface I3 extends I1 {
        m(x: A2): string { return "I3" }
    }

    export interface I4 extends I2, I3 { }
}

export namespace test3c {
    export class A1 { }
    export class A2 extends A1 { }

    export interface I1 {
        m(x: A1): string { return "I1" }
    }

    export interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    export interface I3 extends I1 {
        m(x: A2): string { return "I3" }
    }

    export interface I4 extends I2, I3 { }
}

export namespace test3d {
    export class A1 { }
    export class A2 extends A1 { }

    export interface I1 {
        m(x: A1): string { return "I1" }
    }

    export interface I2 extends I1 {
        m(x: Object): string { return "I2" }
    }

    export interface I3 extends I1 {
        m(x: A2): string { return "I3" }
    }

    export interface I4 extends I2, I3 { }
}

export namespace test6 {
    export class R1 { }
    export class R2 extends R1 { }
    export interface I1 {
        m(x: Object): R1
    }
    export interface I2 extends I1 {
        m(x: string): R2 { return new R2() }
    }
}
