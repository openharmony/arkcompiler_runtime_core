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
    Pins (metadata/GAP2a) whether a consumer can import and use a type alias
    whose definition references a type the aliasing package imported but did NOT
    re-export. EXPECTED output "1" if resolvable transitively.
files: [lib_widget.ets, lib_alias.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "1"
es2panda_options: ['read-metadata']
---*/

import { makeAlias } from '@panda/alias/lib_alias'
import type { Alias } from '@panda/alias/lib_alias'

let a: Alias = makeAlias()
console.log(a.w)
