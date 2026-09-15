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
desc: Enum array missing element initializer is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0124: Expected 2 arguments, got 0.
    - >-
        Semantic error ESE0127: No matching construct signature for
        enum_array_missing_element_initializer_n.Color()
    - >-
        Syntax error ESY63526: Expression to initialize array element is missed.
---*/

'use static'

enum Color { Red, Green, Blue }
let size = 3

function main() {
  let colors: Color[] = new Color[size]
}
