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
    Cross-package polymorphic array: Dog and Cat in Animal[]
files: [lib_types.ets]
arktsconfig: arktsconfig-types-only.json
expected_out:
    runtime:
        - "Rex:woof,Mimi:meow"
---*/

import { Animal } from '@panda/types/lib_types'
import { Dog, Cat } from '@panda/types/lib_impls'

function main(): void {
    let animals: Animal[] = [new Dog("Rex"), new Cat("Mimi")]
    let result = ""
    for (let i = 0; i < animals.length; i++) {
        if (i > 0) result += ","
        result += animals[i].speak()
    }
    console.log(result)
}
