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

/*---
desc: Test for binary compatibility. Adding new method to a base class.
tags:
    - compile-only
    - not-a-test
---*/

export class A {}
export class B extends A {}

export class C1 {
    /*
     * Adding the following public method causes
     * an ambiguity in method resolution:
     * the both methods in C2 now override
     * the added method. This is prohibited by
     * 15.9.1 Overriding in Classes
     */
    m(p: B): string {
        return "C1.B"
    }

    m(p: A): string {
        return "C1.A"
    }
}
