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
desc: Function reference parameter contravariance is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for (Base, Int)
    - >-
        Semantic error ESE0046: Type 'Base' is not compatible with type 'Derived' at index 1
---*/

'use static'

class Base {}
class Derived extends Base {}

function acceptsBase(p: Base, n: number): Base {
  return p
}

function acceptsDerived(p: Derived, n: number): Base {
  return p
}

let reference = acceptsDerived
reference = acceptsBase
reference(new Base(), 1)
