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
desc: Checking on field has an initializer and is assigned in the constructor with optional parameter.
expected_out: >- 
    Semantic error ESE0297: Cannot reassign readonly field
tags: [compile-only, negative]
---*/

class A {
    readonly field: int = 3
    constructor (v: int = 5) {
        if (v == 5) {
            this.field = v  // CTE: Changing the value of a readonly field after initialization is not allowed
        }
    }
}
