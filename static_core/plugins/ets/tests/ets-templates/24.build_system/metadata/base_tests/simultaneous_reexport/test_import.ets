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
desc: Verify re-exports work in simultaneous compilation
files: [lib_a.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "42"
        - "100"
---*/

import {getVal} from '@panda/lib/lib_c'
import {getCalc} from '@panda/lib/lib_c'

console.log(getVal())
console.log(getCalc())
