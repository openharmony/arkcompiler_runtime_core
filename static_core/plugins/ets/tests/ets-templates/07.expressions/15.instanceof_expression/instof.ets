/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    The result of an instanceof expression is true
    if the actual type of evaluated expr is a subtype of T (see Subtyping).
    Otherwise, the result is false.
---*/

interface I {}

interface J {}

class A implements I {}

class B extends A implements J {}

function main(): int {
    let a: A = new A();
    if (!(a instanceof A)) return 1;
    if (!(a instanceof I)) return 2;
    if (a instanceof B) return 3;
    if (a instanceof J) return 4;

    a = new B();
    if (!(a instanceof A)) return 5;
    if (!(a instanceof I)) return 6;
    if (!(a instanceof B)) return 7;
    if (!(a instanceof J)) return 8;

    let i: I = new A();
    if (!(i instanceof A)) return 9;
    if (!(i instanceof I)) return 10;
    if (i instanceof B) return 11;
    if (i instanceof J) return 12;

    i = new B();
    if (!(i instanceof A)) return 13;
    if (!(i instanceof I)) return 14;
    if (!(i instanceof B)) return 15;
    if (!(i instanceof J)) return 16;

    let b: B = new B();
    if (!(b instanceof A)) return 17;
    if (!(b instanceof I)) return 18;
    if (!(b instanceof B)) return 19;
    if (!(b instanceof J)) return 20;

    let j: J = new B();
    if (!(j instanceof A)) return 21;
    if (!(j instanceof I)) return 22;
    if (!(j instanceof B)) return 23;
    if (!(j instanceof J)) return 24;

    return 0;
}
