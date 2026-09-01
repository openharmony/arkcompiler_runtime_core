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
desc: for-of expression type must be iterable
assertion: CI-08-06-03-001-01
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0076: 'For-of' statement source expression is not of iterable type.
    Semantic error ESE0258: Object type doesn't have proper iterator method.
---*/

function main(): void {
    let notIterable: number = 42;
    for (const item of notIterable) {  // Compile-time error: number is not iterable
        console.log(item);
    }
}
