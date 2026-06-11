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
    Consumer reads, mutates and replaces a tuple-typed field on a
    cross-package class.
files: [lib_coord.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "(1,2)"
        - "(11,22)"
        - "(11,22,start)"
        - "(100,200)"
---*/

import { Coord } from '@panda/lib/lib_coord'

let c = new Coord(1, 2, "start")
console.log("(" + c.pos[0] + "," + c.pos[1] + ")")

let after: [int, int] = c.move(10, 20)
console.log("(" + after[0] + "," + after[1] + ")")

let triple: [int, int, string] = c.raw()
console.log("(" + triple[0] + "," + triple[1] + "," + triple[2] + ")")

c.pos = [100, 200]
console.log("(" + c.pos[0] + "," + c.pos[1] + ")")
