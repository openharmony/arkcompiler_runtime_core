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
desc: Overload no matching function is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for overloadFail001Value(Boolean)
    - >-
        Semantic error ESE0046: Type 'Boolean' is not compatible with type 'String' at index 1
---*/

'use static'

function overloadFail001Value(p: int): int {
  return p
}

function overloadFail001Value(p: string): string {
  return p
}

let overloadFail001Result: int = overloadFail001Value(true)
