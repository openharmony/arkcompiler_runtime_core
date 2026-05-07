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
        m(): string
    }
}

export namespace test2 {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 {
        m(): string
    }
}

export namespace test3a {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test4a {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
}

export namespace test5a {
    export interface I1 {
        m(): string { throw new Error() }
    }
    export interface I2 extends I1 { }
    export interface I3 extends I1 { }
    export interface I4 extends I2, I3 { }
}
