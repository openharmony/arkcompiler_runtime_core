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
    Mandatory call to another same-class constructor that uses the keyword this.
    A compile-time error occurs if: The call is not a root-level statement within a constructor.
expected_out: |-
    Semantic error ESE2077: 'this' call must be a root-level statement in constructor
tags: [compile-only, negative]
---*/

abstract class A {
    fldA: string

    constructor(a: string) {
        this.fldA = a
    }
}

final class B extends A {
    constructor(a: string) {  // primary
        super(a)
    }

    constructor(a: int) {  // secondary
        if (a == 0) {
            throw new Error('zero')
        } else {
            if (a == 1) {
                this('1')
                return
            } else {
                if (a == 2) {
                    this('2')
                    return
                }
                this('3')
            }
        }
    }
}
