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
    Test default import (import Calc from ...).
    Covers importDefaultSpecifier.cpp and exportDefaultDeclaration.cpp code paths.
files: [lib_default.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "7"
        - "12"
---*/

import Calc from '@panda/lib/lib_default'

let c = new Calc()
console.log(c.add(3, 4))
console.log(c.mul(3, 4))
