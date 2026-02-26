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

abstract class A {
    public p1: int|string|boolean = 0;
    public q1: number|undefined;
    public w1: number[]|undefined;

    constructor(p: int) {
        this.p1 = p
    }
    constructor(p: int|string, q: number, w?: number[]) {
        if (p instanceof int)
            this.p1 = p as int
        else if (p instanceof string)
            this.p1 = p as string

        this.q1 = q

        this.w1 = w

    }
    constructor(p: boolean, q?: number[]) {}
    constructor(...p: Any[]) {
        if (p.length > 0) {
            if (p[0] instanceof int)
                this.p1 = p[0] as int
            else if (p[0] instanceof string)
                this.p1 = p[0] as string
            else this.p1 = p[0] as boolean
        }
        if (p.length > 1) {
            this.q1 = p[1] as number
        }
        if (p.length > 2) {
            this.w1 = undefined
        }
    }
}


class B extends A {
    constructor fromInt(p: int) {
        super(p)
    }

    constructor fromString(p: string, j: number) {
        super(p , j)
    }

    constructor fromArray(p: boolean, w?: number[]) {
        super(p, w)
    }

    overload constructor { fromInt, fromString, fromArray }

}

function main() {
    let v = new B(42)
    arktest.assertEQ(v.p1, 42)
    arktest.assertEQ(v.q1, undefined)
    arktest.assertEQ(v.w1, undefined)

    let x = new B("hello", 1.0)
    arktest.assertEQ(x.p1, "hello")
    arktest.assertEQ(x.q1, 1.0)
    arktest.assertEQ(x.w1, undefined)
    
    //empty super constructor
    let y = new B(true, [1.0, 2.0, 3.0])
    arktest.assertEQ(y.p1, 0)
    arktest.assertEQ(y.q1, undefined)
    arktest.assertEQ(y.w1, undefined)
}
