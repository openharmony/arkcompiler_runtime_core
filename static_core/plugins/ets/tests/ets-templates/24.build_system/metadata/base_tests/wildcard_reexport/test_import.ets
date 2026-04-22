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
desc: Wildcard reexport main
files: [lib_origin.ets, lib_reexport.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "25"
        - "0"
---*/

import {Point, distance, origin} from '@panda/reexport/lib_reexport'

let a = new Point(0, 0)
let b = new Point(3, 4)
console.log(distance(a, b))
console.log(origin())
