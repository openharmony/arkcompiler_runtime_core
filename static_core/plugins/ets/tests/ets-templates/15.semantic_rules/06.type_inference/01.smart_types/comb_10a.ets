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

/*---
desc: >-
    There could be cases when the type of an entity (mostly local variables)
    is a union type and in the particular context of the program the compiler
    can narrow (smart cast) the static type to a more precise type (smart type)
    and allow operations which are specific to such narrowed type.
---*/
class A {}
let a: A|undefined = new A()

let num: Number = 0;
let str: string = "";
let err: Error = new Error();
let bool: Boolean = false;

function foo(p: number|string|undefined|Error|boolean): number {
    return p === undefined ? ((): number => { a = p; return 1 })() // assigned without cast
        : p instanceof Number ? ((): number => { num = p * 2; return 2 })() // numeric operation without cast
            : p instanceof Error ? ((): number => { err = p; str = p.message; return 3 })() // field access without cast
                : p instanceof String ? ((): number => { str = p.substr(0, 1); return 4 })() // string method called without cast
                    : ((): number => { bool = !p; return 5 })()  // boolean operation without cast
}

function main() {
    assertTrue(foo(undefined) == 1 && a === undefined)
    assertTrue(foo(42.0) == 2 && num == 84)
    let e: Error = new Error("xyz")
    assertTrue(foo(e) == 3 && err == e && str == "xyz")
    assertTrue(foo("abc") == 4 && str == "a")
    assertTrue(foo(true) == 5 && !bool)
}
