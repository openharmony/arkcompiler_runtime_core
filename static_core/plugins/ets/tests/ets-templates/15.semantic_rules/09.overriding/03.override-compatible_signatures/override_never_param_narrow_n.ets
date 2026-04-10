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
    A method parameter typed 'undefined' in the parent must not be narrowed to 'never' in
    the override, even though both share the same assembly descriptor (contravariant
    parameter rule).
tags: [negative, compile-only]
expected_out: |-
    Semantic error ESE0136: Method foo(p: never): Int in B not overriding any method
---*/

class A {
    foo(p: undefined): int { return 1 }
}

class B extends A {
    override foo(p: never): int { return 2 }
}
