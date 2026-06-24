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
    A consumer imports the renamed type 'Bar' (exported by the library via
    `export type { Foo as Bar }`) and uses it to type a binding constructed
    from the value 'Foo'. EXPECTED output "1".
files: [lib_types.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "1"
---*/

import { Foo } from '@panda/lib/lib_types'
import type { Bar } from '@panda/lib/lib_types'

let b: Bar = new Foo()
console.log(b.v)
