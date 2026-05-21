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
    15.9 Cross-package inherited method call on subclass:
    Dog.getName() where getName is inherited from Animal 
files: [lib_types.ets]
arktsconfig: arktsconfig-types-only.json
expected_out:
    runtime:
        - "Rex"
---*/

import { Dog } from '@panda/types/lib_impls'

function main(): void {
    let d = new Dog("Rex")
    console.log(d.getName())
}
