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
    Single `import M, { helper, VERSION } from ...` mixes a default
    specifier with named specifiers in one ImportDeclaration. The
    consumer uses all three forms at runtime to verify the binder
    bound each specifier to the right source symbol.
files: [lib_combo.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "default-main"
        - "named-helper"
        - "1.0.0"
---*/

import M, { helper, VERSION } from '@panda/lib/lib_combo'

console.log(new M().label())
console.log(helper())
console.log(VERSION)
