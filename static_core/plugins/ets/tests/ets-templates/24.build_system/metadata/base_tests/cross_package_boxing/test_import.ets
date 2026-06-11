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
    Test boxing/unboxing conversions across packages.
files: [lib_box.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "42"
        - "hello"
        - "true"
        - "100"
        - "7"
        - "test"
        - "5"
---*/

import { objectToString, byteToInt, Wrapper, Config } from '@panda/lib/lib_box'

let n: Int = 42
console.log(objectToString(n))
console.log(objectToString("hello"))

let b: Object = true
console.log(objectToString(b))

let m: Int = 100
let w = new Wrapper<Int>(m)
console.log(w.describe())

let x: Byte = 7
console.log(byteToInt(x))

let cfg: Config = { label: "test", count: 5 }
console.log(cfg.label)
console.log(cfg.count)
