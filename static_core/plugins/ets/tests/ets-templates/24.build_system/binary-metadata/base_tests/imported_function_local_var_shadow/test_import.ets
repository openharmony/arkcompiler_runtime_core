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
    Positive: importing a function and then declaring a local non-ambient
    entity of the same name is allowed, and the local declaration takes
    precedence. Because cross-package declarations are loaded from
    storage as AMBIENT (`declare`), here 'helper' resolves
    to the local `let helper`, so `console.log(helper)` prints its value;
    the imported function is superseded (calling helper(1) would be a
    compile error, 'Int has no call signatures').
files: [lib_helper.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "5"
---*/

import { helper } from '@panda/lib/lib_helper'

let helper: int = 5

console.log(helper)
