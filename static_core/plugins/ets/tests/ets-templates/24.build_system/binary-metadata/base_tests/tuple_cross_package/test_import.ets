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
    Cross-package use of tuple shapes: receive a tuple via the imported
    type alias, construct a fresh tuple literal and pass it into an
    imported function.
files: [lib_pair.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "(7,hello)"
        - "(11,world)"
        - "3:tag:true"
---*/

import { IntStr, makePair, describe, takeMixed } from '@panda/lib/lib_pair'

let p1: IntStr = makePair(7, "hello")
console.log(describe(p1))

let p2: IntStr = [11, "world"]
console.log(describe(p2))

console.log(takeMixed([3, "tag", true]))
