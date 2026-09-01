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
desc: Label identifier cannot be used in lambda expressions within loop body
assertion: CI-08-06-001-01
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE49316: Continue or break target can't be outside the function
---*/

function main(): void {
    outerLoop: for (let i = 0; i < 10; i++) {
        const f = () => {
            break outerLoop;  // Compile-time error: label used in lambda
        };
    }
}
