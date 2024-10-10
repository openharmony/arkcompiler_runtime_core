/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    Using instanceof to test a reference type expression allows to find out if
    the class of the object referenced by the run-time value of the expression
    is convertible to some other reference type.
---*/

interface I {}

interface J {}

class A implements I {}

class B extends A implements J {}

function main(): int {
    let a: A = new A();
    if (!(a instanceof A)) return 1;
    if (!(a instanceof I)) return 1;
    if (a instanceof B) return 1;
    if (a instanceof J) return 1;

    a = new B();
    if (!(a instanceof A)) return 1;
    if (!(a instanceof I)) return 1;
    if (!(a instanceof B)) return 1;
    if (!(a instanceof J)) return 1;

    let i: I = new A();
    if (!(i instanceof A)) return 1;
    if (!(i instanceof I)) return 1;
    if (i instanceof B) return 1;
    if (i instanceof J) return 1;

    i = new B();
    if (!(i instanceof A)) return 1;
    if (!(i instanceof I)) return 1;
    if (!(i instanceof B)) return 1;
    if (!(i instanceof J)) return 1;

    let b: B = new B();
    if (!(b instanceof A)) return 1;
    if (!(b instanceof I)) return 1;
    if (!(b instanceof B)) return 1;
    if (!(b instanceof J)) return 1;

    let j: J = new B();
    if (!(j instanceof A)) return 1;
    if (!(j instanceof I)) return 1;
    if (!(j instanceof B)) return 1;
    if (!(j instanceof J)) return 1;

    return 0;
}
