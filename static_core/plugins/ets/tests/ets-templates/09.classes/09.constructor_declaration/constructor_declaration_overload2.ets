/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    A constructor is used in the creation of an object that is an instance of a class.
---*/

class A {
    public p1: int|string|boolean = false;
    public q1: number|number[]|undefined;
    public w1: number[]|undefined;
    
    //constructor for p1 is int type
    constructor(p: int) {}
    constructor(p: boolean|string, q?: number, w?: number[]) {
        this.p1 = p
        this.q1 = q
        this.w1 = w
    }
    constructor(p: boolean, q?: number[]) {
        this.p1 = p
        this.q1 = q
    }
    //default empty constructor
    constructor() {}
}

final class B extends A {
    constructor(p: int) {super(p)}
    constructor(p: boolean|string, q?: number, w?: number[]) {super(p, q, w)}
    constructor(p: boolean, q?: number[]) {super(p, q)}
    constructor() {super()}
}

function main() {
    let v = new B()
    arktest.assertEQ(v.p1, false)
    arktest.assertEQ(v.q1, undefined)
    arktest.assertEQ(v.w1, undefined)

    v = new B("hello")
    arktest.assertEQ(v.p1, "hello")
    arktest.assertEQ(v.q1, undefined)
    arktest.assertEQ(v.w1, undefined)

    v = new B(true)
    arktest.assertEQ(v.p1, true)
    arktest.assertEQ(v.q1, undefined)
    arktest.assertEQ(v.w1, undefined)

    v = new B(42)
    arktest.assertEQ(v.p1, false)
    arktest.assertEQ(v.q1, undefined)
    arktest.assertEQ(v.w1, undefined)

    v = new B(true, [1.0, 2.0, 3.0])
    arktest.assertEQ(v.p1, true)
    let arr = v.q1 as number[]
    arktest.assertEQ(arr[0], 1.0)
    arktest.assertEQ(arr[1], 2.0)
    arktest.assertEQ(arr[2], 3.0)

    v = new B("hello", 3.14, [1.0, 2.0, 3.0])
    arktest.assertEQ(v.p1, "hello")
    arktest.assertEQ(v.q1, 3.14)
    arr = v.w1 as number[]
    arktest.assertEQ(arr[0], 1.0)
    arktest.assertEQ(arr[1], 2.0)
    arktest.assertEQ(arr[2], 3.0)
}
