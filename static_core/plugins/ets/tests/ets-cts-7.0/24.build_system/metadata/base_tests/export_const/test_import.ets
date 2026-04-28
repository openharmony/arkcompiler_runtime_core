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
desc: Export const main
files: [test_lib.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "100"
        - "Hello"
        - "1"
        - "2"
---*/

import {MAX_SIZE, GREETING, nextCounter} from '@panda/lib/test_lib'

console.log(MAX_SIZE)
console.log(GREETING)
console.log(nextCounter())
console.log(nextCounter())
