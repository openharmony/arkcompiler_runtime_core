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

{% for c in cases %}

/*---
desc: >-
    A method whose parent declares 'never' as its parameter type may be overridden by a
    method declaring any other parameter type. 'never' is the bottom type, so any type
    is its supertype and the contravariant parameter rule is satisfied.
tags: []
---*/

class A {
    foo(p: never): int { throw new Error("unreachable") }
}

class B extends A {
    override foo(p: {{c.derived_param}}): int { return 42 }
}

function main(): void {
    let b: B = new B()
    arktest.assertEQ(b.foo({{c.arg}}), 42)
}

{% endfor %}
