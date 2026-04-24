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
    export class A2 extends A1 { }

    export interface I1 {
        m(x: A1, y: A1): string { return "I1" }
    }

    export class C1 implements I1 {
        m(x: Object, y: A1): string { return "C1a" }
        m(x: A1, y: Object): string { return "C1b" }
    }
}

export namespace test3 {
    export class A1 { }

    export interface I1 {
        m(x: A1, y: A1): string { return "I1" }
    }

    class C1 {
        m(x: A1, y: Object): string { return "C1" }
    }

    export class C2 extends C1 implements I1 {
        m(x: A1, y: Object): string { return "C2a" }
        m(x: Object, y: Object): string { return "C2b" }
    }
}
