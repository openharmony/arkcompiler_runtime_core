/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: >-
    Mandatory call to a superconstructor if a class has an extension clause.
    A compile-time error occurs if: The call is not a root-level statement within a constructor.
expected_out: |-
    Semantic error ESE2077: 'super' call must be a root-level statement in constructor
tags: [compile-only, negative]
---*/

class A {
    fldA: int

    constructor(a: int) {
        this.fldA = a
    }
}

class B extends A {
    constructor(a: int) {
        try {
            if (a == 0) {
                throw new Error('Error thrown')
            }
            super(a)
        } catch (e) {
            super(-1)
        }
    }
}
