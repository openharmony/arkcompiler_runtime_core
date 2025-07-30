/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

{%- for c in cases %}

/*---
desc: >-
    An overriding member can keep or extend the access modifier (see Access Modifiers) of the inherited or implemented
    member. Otherwise, a compile-time error occurs.
    Only accessible (see Accessible) methods are subject for overloading and overriding. For example, neither
    overriding nor overloading is considered if a superclass contains a private method, and a subclass has a method
    with the same name. In case of overriding the same rules are applied for accessors as well.

    {{c.doc}}
tags: [{{c.tags}}]
---*/

let res: number = 0;

class Base {
    public public_foo() { res = 1;}
    protected protected_foo(i: Number) { res = 2;}
    internal internal_foo() { res = 3;}
    private private_foo() { res = 4;}
}

interface Interface {
    public_foo() { res = 5;}
}

class Gen<T> {
    protected foo(i: Gen<T>) {}
}

class Gen2<T,S> {
    protected foo(i: T) {}
}

{{c.decl}}

function main() {
    {{c.use}}
}

{%- endfor %}
