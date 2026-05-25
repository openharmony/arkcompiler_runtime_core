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
desc: Cross-package consumption of every primitive value type plus Any
files: [test_lib.ets]
name: test_import
arktsconfig: arktsconfig-import.json
expected_out:
    runtime:
        - "11"
        - "101"
        - "3.5"
        - "Z"
        - "value=7"
        - "42"
        - "1|2|3.5|X|abc"
---*/

import {takeShort, takeLong, takeFloat, takeChar, takeAny, makeAny, PrimBag} from '@panda/lib/test_lib'

function main(): void {
    console.log(takeShort(Int.toShort(10)))
    console.log(takeLong(100))
    console.log(takeFloat(2.5f))
    console.log(takeChar(c'Z'))
    console.log(takeAny("value=7"))
    console.log(takeAny(makeAny()))

    let bag = new PrimBag(Int.toShort(1), 2, 3.5f, c'X', "abc")
    console.log(bag.summary())
}
