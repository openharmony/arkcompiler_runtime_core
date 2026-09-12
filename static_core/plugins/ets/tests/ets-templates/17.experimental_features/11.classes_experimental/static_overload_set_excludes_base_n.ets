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
desc: Static overload set excludes base is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE854298: Class static member foo must be accessed with the class name
        StaticFail006Base qualification only.
---*/

'use static'

class StaticFail006Base {
  static foo(p: int): int {
    return p
  }
}

class StaticFail006Child extends StaticFail006Base {
  static foo(p: string): string {
    return p
  }
}

function main(): void {
  StaticFail006Child.foo(1)
}
