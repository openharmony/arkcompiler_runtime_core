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
    A method may override a parent method with any return type by declaring 'never' as
    its return type. 'never' is the bottom type and is return-type-substitutable for any
    type.
tags: []
---*/

class A {
    foo(): {{c.parent_ret}} { {{c.parent_body}} }
}

class B extends A {
    override foo(): never { throw new Error("never") }
}

function main(): void {
    let a: A = new B()
    arktest.expectThrow(() => { a.foo() }, (e: Error): boolean => e instanceof Error)
}

{% endfor %}
