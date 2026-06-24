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
    Negative test: assigning Cat to Dog variable across packages.
    Triggers check for incompatible type assignment
    when type information comes from serialized declaration.
files: [lib_types.ets]
tags: [compile-only, negative]
name: test_import_n
arktsconfig: arktsconfig-import.json
---*/

import { Cat, Dog, acceptDog } from '@panda/lib/lib_types'

let c: Cat = new Cat("Whiskers")
let d: Dog = c
console.log(acceptDog(c))
