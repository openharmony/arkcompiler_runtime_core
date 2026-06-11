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
    Single `import E, * as Lib from ...` mixes the default specifier
    with a namespace specifier in one ImportDeclaration. Uses both at
    runtime: E for the default class, Lib.* for named symbols.
files: [lib_combo.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "default-engine"
        - "18"
        - "built"
        - "X1"
---*/

import E, * as Lib from '@panda/lib/lib_combo'

console.log(new E().spec())
console.log(new Lib.Wheel().diameter)
console.log(Lib.build())
console.log(Lib.MODEL)
