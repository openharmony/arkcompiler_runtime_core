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
    Two call sites bind the type parameter U to different concrete types
    (string, int). Each binding forces a fresh substitution through
    Producer<U> and ReturnType<Producer<U>>.
files: [lib_factory.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "hello"
        - "42"
---*/

import { Producer, callAndReturn, constantProducer } from '@panda/lib/lib_factory'

let ps: Producer<string> = constantProducer<string>("hello")
let pi: Producer<int> = constantProducer<int>(42)
console.log(callAndReturn<string>(ps))
console.log(callAndReturn<int>(pi))
