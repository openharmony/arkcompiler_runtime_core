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
desc: >-
  If type used in a context is a fixed-size array type, and type of each
  expression is assignable to an array element type, then an array literal
  is of the specified type. Otherwise, a compile-time error occurs.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0227: Array element at index 0 with type 'Int' is not compatible with the target array element type 'String'"
---*/

function main() {
    let a: FixedArray<string> = [1, 2]
}
