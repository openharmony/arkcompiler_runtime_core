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
        m(x: I1): string { throw new Error() }
    }

    export interface I2 extends I1 { }

    export interface I3 extends I1 { }

    export interface I4 extends I2, I3 { }
}

export namespace test2 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test3 {
    export class C1 {
        m(): string { throw new Error() }
    }

    export interface I1 {
        m(): number { throw new Error() }
    }
}

export namespace test4 {
    export class A1 { }

    export class C1 {
        m(x: A1): string { throw new Error() }
    }

    export interface I1 {
        m(x: A1): string { throw new Error() }
    }
}

export namespace test5 {
    export interface I1 {
        m(): string
    }

    export abstract class C1 implements I1 {
        m(): string { throw new Error() }
    }
}

export namespace test6 {
    export class C1 {
        m(): string { throw new Error() }
    }

    export interface I1 {
        m(): string { throw new Error() }
    }

    export abstract class C2 extends C1 implements I1 {
        m(): string { throw new Error() }
    }

    export class C3 extends C2 { }
}

export namespace test7 {
    export interface I1 {
        m(): string { throw new Error() }
    }

    export abstract class C1 implements I1 {
        m(): string { throw new Error() }
    }

    export class C2 extends C1 { }
}
