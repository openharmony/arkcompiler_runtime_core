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
    Pins (metadata/GAP1) whether a consumer can reference the type-only re-export
    B of a NON-exported underlying class across a package boundary, used purely
    in type position. EXPECTED output "0".
files: [lib_types.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "0"
---*/

import type { B } from '@panda/lib/lib_types'

let arr: FixedArray<B> = new FixedArray<B>(0)
console.log(arr.length)
