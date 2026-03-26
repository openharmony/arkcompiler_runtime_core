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
  A compile-time error occurs if the type specified by context is not one
  of the following: Any, Object, Tuple type, Fixed-size array type,
  Resizable array type, Superinterface of a resizable array type, or
  Union type that contains at least one constituent type from the above list.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0318: Type 'Array<Int>' cannot be assigned to type 'MyClass'"
# NOTE: Current error is an assignability error (ESE0318) because the compiler
# infers Array<Int> from elements first, then fails at assignment. If the compiler
# later adds a specific "invalid array literal context" diagnostic, expected_out
# may need updating.
---*/

class MyClass {}

function main() {
    let a: MyClass = [1, 2, 3]
}
