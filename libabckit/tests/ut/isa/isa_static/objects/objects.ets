/**
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
class A {
    constructor() {}
    constructor(a: int, b: int) {
        console.log("A " + a + b)
    }
    foo() {
        console.log("A")
    }
}

class B {}

class C {
    constructor() {
        console.log("C")
    }
    constructor(a: int) {
        console.log("C " + a)
    }
    constructor(a: int, b: int) {
        console.log("C " + a + b)
    }
    constructor(a: int, b: int, c: int) {
        console.log("C " + a + b + c)
    }
    constructor(a: int, b: int, c: int, d:int) {
        console.log("C " + a + b + c + d)
    }
    constructor(a: int, b: int, c: int, d: int, e: int) {
        console.log("C " + a + b + c + d + e)
    }
}

function bar(b: B) {
    console.log("B")
}

function main() {
    let a = new A()
    a.foo()
}
