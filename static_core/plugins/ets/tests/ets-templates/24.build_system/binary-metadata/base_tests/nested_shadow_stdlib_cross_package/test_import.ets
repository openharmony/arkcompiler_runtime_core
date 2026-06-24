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
    Positive: spec 13.10 allows redefining a stdlib/predefined simple name
    in a NESTED scope (it is an error only at module scope). Here the
    predefined type name 'Array' is legally shadowed by a local value
    binding inside a function body, alongside a cross-package import; the
    innermost binding takes precedence (spec 4.3) while the predefined
    Array type stays usable at module scope outside the function.
files: [lib_shadow.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "nested-ok"
        - "1"
---*/

import { LABEL } from '@panda/lib/lib_shadow'

function shadowed(): string {
    // 'Array' is a predefined/std type name; legally shadowed by a local
    // value binding inside this nested (function) scope.
    let Array: string = LABEL
    return Array
}

// Outside the function the predefined Array type is unaffected.
let nums: FixedArray<int> = [1, 2, 3]

console.log(shadowed())
console.log(nums[0])
