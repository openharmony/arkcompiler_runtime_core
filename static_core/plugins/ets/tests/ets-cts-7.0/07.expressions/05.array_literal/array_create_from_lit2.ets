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
desc: Create arrays from array literals, including nested arrays, spread operators and covariant subtypes.
---*/

class Base {
    v: int = 0
    constructor(v: int = 0) {
        this.v = v
    }
}
class Derived extends Base {
    constructor(v: int = 0) {
        super(-v)
    }
}

class Animal {}
class Dog extends Animal {
    name: string = ""
    bark(): string {
        return this.name
    }
}
class Cat extends Animal {}

class FuncBase {
    v: int
    constructor(v: int = 0) {
        this.v = v
    }
}
class FuncDerived extends FuncBase {
    constructor(v: int = 0) {
        super(-v)
    }
    f(): int {
        return 13
    }
}
type HandlerDerivedInt = (a: FuncDerived, b: int) => FuncBase
type HandlerBaseInt = (a: FuncBase, b: int) => FuncDerived
const handlerBaseInt: HandlerBaseInt = (a: FuncBase, b: int): FuncDerived => {
    if (a instanceof FuncDerived) {
        return new FuncDerived(a.f() + b)
    } else {
        return new FuncDerived(b)
    }
}
type HandlerBase = (a: FuncBase) => FuncBase
const handlerBase: HandlerBase = (a: FuncBase): FuncBase => {
    if (a instanceof FuncDerived) {
        return new FuncBase(a.f())
    } else {
        return new FuncBase
    }
}
type HandlerNothing = () => FuncDerived
const handlerNothing: HandlerNothing = (): FuncDerived => {
    return new FuncDerived(567)
}

function main(): int {
    let suite = new arktest.ArkTestsuite("array_create_from_lit2")

    suite.addTest("Create empty 1D Array", () => {
        let a: number[] = []
        arktest.assertEQ(a.length, 0)
    })

    suite.addTest("Create 1D Array with the data type int", () => {
        let a: int[] = [5, 3, 7, 9]
        arktest.assertEQ(a.length, 4)
        arktest.assertTrue(a[0] instanceof int)
        arktest.assertEQ(a[0], 5)
        arktest.assertEQ(a[1], 3)
        arktest.assertEQ(a[2], 7)
        arktest.assertEQ(a[3], 9)
    })

    suite.addTest("Create a 2D Array with the data type number", () => {
        let a : number[][] = [[5, 3.7], [7, 8.5], [3, 4]]
        arktest.assertEQ(a.length, 3)
        arktest.assertEQ(a[0].length, 2)
        arktest.assertEQ(a[1].length, 2)
        arktest.assertEQ(a[2].length, 2)
        arktest.assertTrue(a[0][0] instanceof number)
        arktest.assertEQ(a[0][0], 5)
        arktest.assertEQ(a[0][1], 3.7)
        arktest.assertEQ(a[1][0], 7)
        arktest.assertEQ(a[1][1], 8.5)
        arktest.assertEQ(a[2][0], 3)
        arktest.assertEQ(a[2][1], 4)
    })

    suite.addTest("Create a 3D Array with the data type string", () => {
        let a : string[][][] = [
          [["_a", "_b"], ["_c", "_d"]],
          [["_e", "_f"], ["_g", "_h"]],
          [["_i", "_j"], ["_k", "_l"]]
        ]
        arktest.assertEQ(a.length, 3)
        arktest.assertEQ(a[0].length, 2)
        arktest.assertEQ(a[0][0].length, 2)
        arktest.assertEQ(a[0][1].length, 2)
        arktest.assertEQ(a[1].length, 2)
        arktest.assertEQ(a[1][0].length, 2)
        arktest.assertEQ(a[1][1].length, 2)
        arktest.assertEQ(a[2].length, 2)
        arktest.assertEQ(a[2][0].length, 2)
        arktest.assertEQ(a[2][1].length, 2)
        arktest.assertTrue(a[0][0][0] instanceof string)
        arktest.assertEQ(a[0][0][0], "_a")
        arktest.assertEQ(a[0][0][1], "_b")
        arktest.assertEQ(a[0][1][0], "_c")
        arktest.assertEQ(a[0][1][1], "_d")
        arktest.assertEQ(a[1][0][0], "_e")
        arktest.assertEQ(a[1][0][1], "_f")
        arktest.assertEQ(a[1][1][0], "_g")
        arktest.assertEQ(a[1][1][1], "_h")
        arktest.assertEQ(a[2][0][0], "_i")
        arktest.assertEQ(a[2][0][1], "_j")
        arktest.assertEQ(a[2][1][0], "_k")
        arktest.assertEQ(a[2][1][1], "_l")
    })

    suite.addTest("Create 1D Array by shallow copy with spread operator", () => {
        let a: int[] = [1, 2]
        let f: FixedArray<int> = [3, 4]
        let ea: int[] = []
        let ef: FixedArray<int> = []
        let t: int[] = [
          ...ea, ...a, ...ef,
          ...f, ...ea, ...a, ...ef
        ]
        t[0] = 7
        t[3] = 9
        t[4] = 5
        arktest.assertEQ(a.length, 2)
        arktest.assertEQ(f.length, 2)
        arktest.assertEQ(ea.length, 0)
        arktest.assertEQ(ef.length, 0)
        arktest.assertEQ(t.length, 6)
        arktest.assertTrue(t[0] instanceof int)
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(f[0], 3)
        arktest.assertEQ(f[1], 4)
        arktest.assertEQ(t[0], 7)
        arktest.assertEQ(t[1], 2)
        arktest.assertEQ(t[2], 3)
        arktest.assertEQ(t[3], 9)
        arktest.assertEQ(t[4], 5)
        arktest.assertEQ(t[5], 2)
    })

    suite.addTest("Create 2D Array by shallow copy with spread operator", () => {
        let a: number[] = [4, 2]
        let f: FixedArray<number> = [7, 8]
        let ea: number[] = []
        let ef: FixedArray<number> = []
        let t: number[][] = [
          [...ea, ...a, ...ef, ...f, ...ea],
          [...ef, ...f, ...ea, ...a, ...ef],
        ]
        t[0][3] = 9
        t[1][2] = 5
        arktest.assertEQ(a.length, 2)
        arktest.assertEQ(f.length, 2)
        arktest.assertEQ(ea.length, 0)
        arktest.assertEQ(ef.length, 0)
        arktest.assertEQ(t.length, 2)
        arktest.assertEQ(t[0].length, 4)
        arktest.assertEQ(t[1].length, 4)
        arktest.assertTrue(t[0][0] instanceof number)
        arktest.assertEQ(a[0], 4)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(f[0], 7)
        arktest.assertEQ(f[1], 8)
        arktest.assertEQ(t[0][0], 4)
        arktest.assertEQ(t[0][1], 2)
        arktest.assertEQ(t[0][2], 7)
        arktest.assertEQ(t[0][3], 9)
        arktest.assertEQ(t[1][0], 7)
        arktest.assertEQ(t[1][1], 8)
        arktest.assertEQ(t[1][2], 5)
        arktest.assertEQ(t[1][3], 2)
    })

    suite.addTest("Create 1D Array by shallow copy with spread operator (class hierarchy)", () => {
        let aA: Base[] = [new Base(2), new Base(4), new Derived(7)]
        let fB: FixedArray<Derived> = [new Derived(5), new Derived(9)]
        let fA: FixedArray<Base> = [new Derived(13), new Base(10)]
        let eaA: Base[] = []
        let efB: FixedArray<Derived> = []
        let t: Base[] = [
          ...eaA, ...fB, ...efB,
          ...aA, ...eaA, ...fA, ...efB
        ]
        t[0] = new Base(8)
        t[3] = new Base(15)
        t[6] = new Derived(5)
        arktest.assertEQ(aA.length, 3)
        arktest.assertEQ(fB.length, 2)
        arktest.assertEQ(fA.length, 2)
        arktest.assertEQ(eaA.length, 0)
        arktest.assertEQ(efB.length, 0)
        arktest.assertEQ(t.length, 7)
        arktest.assertTrue(t[0] instanceof Base)
        arktest.assertEQ(aA[0].v, 2)
        arktest.assertEQ(aA[1].v, 4)
        arktest.assertEQ(aA[2].v, -7)
        arktest.assertEQ(fB[0].v, -5)
        arktest.assertEQ(fB[1].v, -9)
        arktest.assertEQ(fA[0].v, -13)
        arktest.assertEQ(fA[1].v, 10)
        arktest.assertEQ(t[0].v, 8)
        arktest.assertTrue(t[1] instanceof Derived)
        arktest.assertEQ(t[1].v, -9)
        arktest.assertEQ(t[2].v, 2)
        arktest.assertEQ(t[3].v, 15)
        arktest.assertEQ(t[4].v, -7)
        arktest.assertTrue(t[5] instanceof Derived)
        arktest.assertEQ(t[5].v, -13)
        arktest.assertEQ(t[6].v, -5)
    })

    suite.addTest("Create 1D Array with covariant subtypes", () => {
        let animals: Animal[] = [new Animal, {name: "dog"} as Dog, new Cat]
        arktest.assertTrue(animals[0] instanceof Animal)
        arktest.assertTrue(animals[1] instanceof Dog)
        arktest.assertEQ((animals[1] as Dog).bark(), "dog")
        arktest.assertTrue(animals[2] instanceof Cat)
    })

    suite.addTest("Create 1D Array with covariant function subtypes", () => {
        let a: HandlerDerivedInt[] = [handlerBaseInt, handlerBase, handlerNothing]
        arktest.assertEQ(a[0](new FuncDerived, 7).v, -20)
        arktest.assertEQ(a[1](new FuncDerived, 7).v, 13)
        arktest.assertEQ(a[2](new FuncDerived, 7).v, -567)
    })

    return suite.run()
}
