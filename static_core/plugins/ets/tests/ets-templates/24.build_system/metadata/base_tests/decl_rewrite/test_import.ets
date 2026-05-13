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
    Imports from both @panda/lib (test_lib) and 
    @panda/consumer (test_consumer).
files: [test_lib.ets, test_consumer.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "42"
        - "200"
        - "42"
---*/

import { Data, getValue } from '@panda/lib/test_lib'
import { createData, processData } from '@panda/consumer/test_consumer'

let d = new Data()
d.value = 42
console.log(getValue(d))

let d2 = createData(100)
console.log(processData(d2))

console.log(getValue(d))
