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
    Consumer imports Container/unbox at the top of a 3-package NAMED re-export
    chain. Symbol resolution must walk two hops of named-export
    indirection (a -> b -> c) before reaching the original definition.
files: [lib_c.ets, lib_b.ets, lib_a.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "7"
---*/

import { Container, unbox } from '@panda/pkga/lib_a'

let b = new Container(7)
console.log(unbox(b))
