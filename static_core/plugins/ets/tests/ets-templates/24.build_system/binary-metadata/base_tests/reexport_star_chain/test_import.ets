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
    Consumer imports symbols at the top of a 3-package `export *` chain:
    Shape/area/TAU originate in lib_c, pass through lib_b (`export *`),
    then lib_a (`export *`), and are pulled here from @panda/pkga/lib_a.
    Each lib also contributes its own symbol.
files: [lib_c.ets, lib_b.ets, lib_a.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "shape:4"
        - "16"
        - "6"
        - "3"
        - "from-a"
---*/

import {Shape, area, TAU, pi3, tag} from '@panda/pkga/lib_a'

let square = new Shape(4)
console.log(square.describe())
console.log(area(4))
console.log(TAU)
console.log(pi3())
console.log(tag())
