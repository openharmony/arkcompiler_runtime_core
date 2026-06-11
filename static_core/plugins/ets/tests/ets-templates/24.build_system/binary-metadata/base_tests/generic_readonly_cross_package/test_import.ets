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
    Exercise both Readonly<T> subtyping rules across a
    package boundary:
      (a) `let r: Readonly<Base> = baseValue`   — T <: Readonly<T>
      (b) `let r: Readonly<Base> = derivedValue`— Derived <: Base
                                                  so Readonly<Derived> <: Readonly<Base>
      (c) `readBaseLabel(d)` where d: Derived   — combined assignment + subtyping
files: [lib_cache.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "base"
        - "named"
        - "deep"
        - "fromRO"
---*/

import { Base, Derived, readBaseLabel } from '@panda/lib/lib_cache'

// (a) T <: Readonly<T>: a Base instance flows into Readonly<Base>
let b = new Base()
let rb: Readonly<Base> = b
console.log(rb.label)

b.label = "named"
console.log(readBaseLabel(b))

// (b)/(c) Derived <: Base, so Readonly<Derived> <: Readonly<Base>;
//         and Derived directly flows into Readonly<Base>
let d = new Derived()
d.label = "deep"
d.extra = 99
console.log(readBaseLabel(d))

let rd: Readonly<Derived> = d
rd.label  // covers a property read on Readonly<Derived>
let rbFromRd: Readonly<Base> = rd
console.log(rbFromRd.label === "deep" ? "fromRO" : "x")
