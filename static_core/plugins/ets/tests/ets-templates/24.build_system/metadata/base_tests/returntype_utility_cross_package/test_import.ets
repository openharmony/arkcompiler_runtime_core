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
    Consumer references ReturnType<StringProducer> where StringProducer
    is an imported type alias for a function signature. The local
    variable type must be deduced to the lambda's return type (string).
files: [lib_funcs.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "greet-ok"
        - "bye-ok"
---*/

import { StringProducer, makeProducer } from '@panda/lib/lib_funcs'

function call(p: StringProducer): ReturnType<StringProducer> {
    return p()
}

console.log(call(makeProducer("greet")))
console.log(call(makeProducer("bye")))
