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
desc: Invariant record value type object assign is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for assign(Record<String,Double>,
        Record<String,Double>)
    - >-
        Semantic error ESE0046: Type 'Record<String,Double>' is not compatible with type
        'Record<String,Any>' at index 1
---*/

'use static'

let target: Record<string, number> = {
  'foo': 1
}
let target2: Record<string, number> = {
  'a': 2
}

let result = Object.assign(target, target2)
