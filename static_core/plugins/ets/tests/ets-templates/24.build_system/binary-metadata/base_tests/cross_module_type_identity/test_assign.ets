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
    Cross-module type identity should work with relative imports.
    lib_child.etscache uses relative import (from ./lib_base).
    Assigning Child to Base should work because the compiler
    should resolve same type identities for Base accessed via package path
    vs through the relative-import declaration file.
files: [lib_base.ets]
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "child:hello"
---*/

import { Base } from '@panda/lib/lib_base'
import { Child } from '@panda/lib/lib_child'

function main(): void {
    let b: Base = new Child("hello")
    if (b instanceof Child) {
        let c = b as Child
        console.log(c.getLabel())
    }
}
