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

export namespace test3 {
    export interface I0 { m(): string { return "I0" } }
    export interface I1 extends I0 { m(): string { return "I1" } }
    export interface I2 extends I0 { m(): string { return "I2" } }
    export interface I3 extends I0 { m(): string { return "I3" } }
    export class C implements I1, I2, I3 { }
}

export namespace test6 {
    export interface I {
        m(): string { return "I.m" }
        n(): string { return "I.n" }
    }
    export interface I2 extends I {
        m(): string { return "I2.m" }
        n(): string { return "I2.n" }
    }
    export interface I3 extends I {
        m(): string { return "I3.m" }
        n(): string { return "I3.n" }
    }
    export class C implements I2, I3 { }
}
