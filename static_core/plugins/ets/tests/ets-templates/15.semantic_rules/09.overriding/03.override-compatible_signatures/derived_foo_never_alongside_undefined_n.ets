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
    A derived class declaring a method 'foo(p: never)' alongside an inherited
    'foo(p: undefined)' must be rejected: at the type-system level the signatures are
    not override-compatible (no 'override' modifier and 'never' is not 'undefined'),
    yet at runtime the assembly descriptors collide ('N'), producing two methods with
    the same descriptor in class B.
tags: [negative, compile-only]
expected_out: |-
    Semantic error ESE0131: Function foo with this assembly signature already declared.
---*/

class A {
    foo(p: undefined): int { return 1 }
}

class B extends A {
    foo(p: never): int { return 2 }
}
