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
    export interface I1 {
        m(): string
    }
    interface I2 extends I1 {
        m(): string { throw new Error() }
    }
    interface I3 extends I1 {
        m(): string { throw new Error() }
    }
    interface I4a extends I2, I3 { }
    interface I4b extends I3, I2 { }
    export class C1 implements I4a {
        m(): string { throw new Error() }
    }
    export class C2 implements I4b {
        m(): string { throw new Error() }
    }
}
