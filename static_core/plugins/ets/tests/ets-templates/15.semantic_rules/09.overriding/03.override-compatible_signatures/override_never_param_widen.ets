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
    A method parameter typed 'never' in the parent may be widened to 'undefined' in the
    override (contravariant parameter rule: 'never' is the bottom type, 'undefined' is its
    supertype).
tags: []
---*/

class A {
    foo(p: never): int { throw new Error("unreachable") }
}

class B extends A {
    override foo(p: undefined): int { return 42 }
}

function main(): void {
    let b: B = new B()
    arktest.assertEQ(b.foo(undefined), 42)
}
