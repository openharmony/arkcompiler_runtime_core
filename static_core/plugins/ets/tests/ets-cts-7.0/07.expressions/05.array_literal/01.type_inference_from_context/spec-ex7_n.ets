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
    If a type used in a context is a union type, then Array Literal Type
    Inference from Context is taken repetitively trying to use each type of
    the union type as the context. If only a single type is inferred, then
    this single type is used as the type of the literal. Otherwise, a
    compile-time error occurs.
tags: [compile-only, negative]
expected_out: |-
    Semantic error ESE0318: Type 'Array<Int|String>' cannot be assigned to type 'Array<Double>|Array<String>'
---*/

function main() {
    let incorrect_union_of_arrays: number[] | string[] = [1, 2, "string"]
}
