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
desc: Callable same parameters different return type is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0130: Function $_instantiate is already declared.
    - >-
        Warning W2323: The order of entities in an overload set implies that the overloaded entity
        with the signature '(factory: (() => CallableFail006)) => Int' will never be selected for a
        call, as the preceding entity with the signature
        '(factory: (() => CallableFail006)) => CallableFail006' prevails.
---*/

'use static'

class CallableFail006 {
  static $_instantiate(factory: () => CallableFail006): CallableFail006 {
    return factory()
  }

  static $_instantiate(factory: () => CallableFail006): int {
    return 1
  }
}
