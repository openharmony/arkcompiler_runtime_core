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
    Three flows that all expect Partial<Animal>:
      (1) value of type Partial<Dog>           -> Partial<Animal>
      (2) Dog instance with extra subclass fields -> Partial<Animal>
      (3) Animal object literal                -> Partial<Animal>
    Cases (1) and (2) require the subtyping rule -
    "Partial<U> is a subtype of Partial<T> when U is a subtype of T"
    to hold across a package boundary. Existing
    partial_utility_cross_package only exercises (3).
files: [lib_box.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "kind=canine"
        - "kind=husky"
        - "kind=cat"
---*/

import { Animal, Dog, describeAnimal } from '@panda/lib/lib_box'

let pd: Partial<Dog> = { kind: "canine", breed: "lab" }
console.log(describeAnimal(pd))

let d = new Dog()
d.kind = "husky"
d.breed = "siberian"
console.log(describeAnimal(d))

let pa: Partial<Animal> = { kind: "cat" }
console.log(describeAnimal(pa))
