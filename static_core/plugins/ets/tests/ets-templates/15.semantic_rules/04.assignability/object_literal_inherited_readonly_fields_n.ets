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
desc: Object literal inherited readonly fields is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE4002: Cannot assign to a readonly field id
---*/

'use static'

class Base {
  public readonly id: int = 1
}

class Mid extends Base {
  public readonly code: int = 10
}

class Derived extends Mid {
  public name: string = 'derived'
  public level: int = 0
}

function main(): void {
  let value: Derived = { id: 2, name: 'test', level: 1 }
}
