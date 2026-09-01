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
desc: const forVariable cannot be assigned in loop body
assertion: CI-08-06-03-003-01
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE4001: Cannot assign to a constant variable element
---*/

function main(): void {
    let values: number[] = [1, 2, 3];
    for (const element of values) {
        element = 66;  // Compile-time error: cannot assign to const
    }
}
