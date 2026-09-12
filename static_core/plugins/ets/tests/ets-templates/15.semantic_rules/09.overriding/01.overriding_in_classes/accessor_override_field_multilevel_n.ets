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
desc: Accessor override field multilevel is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE45073: 'field' is defined as a property in class
        'FieldInheritFail037Root', but is overridden here in 'FieldInheritFail037Leaf' as an
        accessor.
    - >-
        Semantic error ESE0136: Method field(): Int in FieldInheritFail037Leaf not overriding any
        method
---*/

'use static'

class FieldInheritFail037Root {
  field: int = 37
}

class FieldInheritFail037Middle extends FieldInheritFail037Root {}

class FieldInheritFail037Leaf extends FieldInheritFail037Middle {
  override get field(): int {
    return 37
  }
}
