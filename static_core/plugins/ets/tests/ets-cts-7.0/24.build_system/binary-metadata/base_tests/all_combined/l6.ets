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
desc: Simple import lib6
tags: [not-a-test, compile-only]
name: l6
arktsconfig: arktsconfig-6.json
es2panda_options: ['emit-metadata']
---*/

import {E as E6} from "@panda/lib1/l1"
import { V } from "@panda/lib34/l4"

export interface I {
    n: int
}

export class C6<T extends string|int> implements I {
    // cf6: NiceA = 1
    n: int = 4
    constructor(x: T) {
        let b: boolean = x instanceof int;
        console.log(E6.Four.valueOf() + this.n + (b ? 1 : 0))
    }
}
