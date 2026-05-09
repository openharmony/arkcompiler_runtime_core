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
    Positive: importing a function under an alias that collides with the
    std/core function 'swap' is allowed. Standard library functions can be
    redefined in a module scope, and the imported alias shadows the std/core
    function in this module.
files: [lib_probe.ets]
tags: [compile-only]
name: test_alias_function_n
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
---*/

import { pair as swap } from '@panda/lib/lib_probe'

let t: [int, string] = swap<int, string>(1, "a")
console.log(t[0])
