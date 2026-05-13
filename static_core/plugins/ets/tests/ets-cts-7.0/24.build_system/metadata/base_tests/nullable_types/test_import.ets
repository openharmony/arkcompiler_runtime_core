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
desc: Nullable types main
files: [test_lib.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "true"
        - "false"
        - "hello"
        - "fallback"
---*/

import {NullableBox, unwrap} from '@panda/lib/test_lib'

let b1 = new NullableBox<String>()
console.log(b1.isEmpty())
b1.put("hello")
console.log(b1.isEmpty())
console.log(unwrap(b1, "default"))

let b2 = new NullableBox<String>()
console.log(unwrap(b2, "fallback"))
