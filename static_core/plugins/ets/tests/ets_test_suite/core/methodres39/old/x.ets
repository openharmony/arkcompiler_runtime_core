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

export namespace test2 {
    export class A1 { }

    export interface I {
        m(x: A1): string { throw new Error() }
    }

    class A {
        m(x: Object): string { throw new Error() }
    }

    export class B extends A implements I {
        m(x: Object): string { throw new Error() }
    }
}

export namespace test3 {
    export interface I1 {
        m(): string
    }

    export interface I2 {
        m(): string { throw new Error() }
    }

    export class C implements I1, I2 {
        m(): string { throw new Error() }
    }
}

export namespace test5 {
    export interface I0 {
        m(): string { throw new Error() }
    }

    export interface I1 extends I0 {
        m(): string { throw new Error() }
    }

    export interface I2 extends I0 {
        m(): string { throw new Error() }
    }

    export interface I3 extends I1, I2 { }

    export class C implements I3 {
        m(): string { throw new Error() }
    }
}

export namespace test6 {
    export interface I0 {
        m(): string { throw new Error() }
    }

    export interface I1 extends I0 {
        m(): string
    }

    export interface I2 extends I0 {
        m(): string { throw new Error() }
    }

    export class C implements I1, I2 {
        m(): string { throw new Error() }
    }
}
