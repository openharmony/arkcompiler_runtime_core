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

package msig

interface A {}
class B {}
enum E { R }

class F {
    foo() {}
    foo(x: boolean) {}
    foo(x: char) {}
    foo(x: byte) {}
    foo(x: short) {}
    foo(x: int) {}
    foo(x: long) {}
    foo(x: float) {}
    foo(x: double) {}
    foo(x: double, y: A, z: B): E { return E.R }
    foo(p: Partial<A>, a: Array<B>) {}

    $_get(index: number): int { return 0 }

    $_set(index: number, x: boolean) {}
    $_set(index: number, x: char) {}
    $_set(index: number, x: byte) {}
    $_set(index: number, x: short) {}
    $_set(index: number, x: int) {}
    $_set(index: number, x: long) {}
    $_set(index: number, x: float) {}
    $_set(index: number, x: double) {}
    $_set(index: number, s: String) {}

    foo(a: int, b: int): int {return a + b }
}

class G {
    static foo() {}
    static foo(x: boolean) {}
    static foo(x: char) {}
    static foo(x: byte) {}
    static foo(x: short) {}
    static foo(x: int) {}
    static foo(x: long) {}
    static foo(x: float) {}
    static foo(x: double) {}
    static foo(x: double, y: A, z: B): E { return E.R }
    static foo(p: Partial<A>, a: Array<B>) {}

    static foo(a: int, b: int, c: int): int {return a + b + c }
}

function foo() {}
function foo(x: boolean) {}
function foo(x: char) {}
function foo(x: byte) {}
function foo(x: short) {}
function foo(x: int) {}
function foo(x: long) {}
function foo(x: float) {}
function foo(x: double) {}
function foo(x: double, y: A, z: B): E { return E.R }
function foo(p: Partial<A>, a: Array<B>) {}

namespace rls {
    interface A {}
    class B {}
    enum E { R }

    function foo() {}
    function foo(x: boolean) {}
    function foo(x: char) {}
    function foo(x: byte) {}
    function foo(x: short) {}
    function foo(x: int) {}
    function foo(x: long) {}
    function foo(x: float) {}
    function foo(x: double) {}
    function foo(x: double, y: A, z: B): E { return E.R }
    function foo(p: Partial<A>, a: Array<B>) {}
};
