/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    An instance method is defined in a subclass with the same name as the instance method in a superclass.
    If signatures are override-compatible, then overriding is used. Otherwise, overloading is used.
tags: [{{c.tags}}]
---*/

let res: number = 0;

class A {}
class B extends A {}
class C extends B {}

type FUN_AB = (i: A) => B;
type FUN_BA = (i: B) => A;
type FUN_AC = (i: A) => C;
type FUN_CA = (i: C) => A;
type FUN_BC = (i: B) => C;
type FUN_CB = (i: C) => B;

function faa(i: A) { return new A(); }
function fab(i: A) { return new B(); }
function fac(i: A) { return new C(); }
function fba(i: B) { return new A(); }
function fbb(i: B) { return new B(); }
function fbc(i: B) { return new C(); }
function fca(i: C) { return new A(); }
function fcb(i: C) { return new B(); }
function fcc(i: C) { return new C(); }

class Base {
    foo({{c.base}}) { res = 1;}
}

class Derived extends Base {
    foo({{c.derived}}) { res = 2;}
}

function main() {
    new Derived().foo({{c.call}});
    assert(res == {{c.res}});
}

{%- endfor %}


