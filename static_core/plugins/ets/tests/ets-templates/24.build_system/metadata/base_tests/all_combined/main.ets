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
desc: Simple import lib
files: [l1.ets, l2.ets, l3.ets, l5.ets, l6.ets, l7.ets, ]
name: test_import
arktsconfig: arktsconfig-main.json
expected_out:
    runtime:
        - "9"
        - "8"
        - "40"
        - "ok"
        - "ola"
---*/

import {C6} from "@panda/lib6/l6"
import {Six} from "@panda/lib789/l7"
import {v八} from "@panda/lib789/l8"
import {v99} from "@panda/lib789/l9"

new C6<int>(1)
new C6<string>("abc")
console.log(Six())
console.log(v八 + "k")
console.log(v99)
