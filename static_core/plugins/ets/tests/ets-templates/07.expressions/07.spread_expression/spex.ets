/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: A spread expression can be used only within the array literal or argument passing.
---*/

type F = (p: number) => number

enum Color { Red, Green, Blue }

interface I {}
interface J {}

class GenA<T> {}
class GenB<T> {}
class GenC<T> {}

class BoolArrThis {
    fld: boolean[]
    constructor(p: boolean[], q: boolean[]) {
        this.fld = [...p, ...q]
    }
    meth(): boolean[] {
        return [true, true, false, ...this.fld]
    }
}

abstract class BoolAbstractA {
    fld: boolean[] = []
    constructor(p: boolean[]) {
        this.fld = p
    }
    meth(): boolean[] {
        return [true, true, false, ...this.fld]
    }
}

class BoolConcreteB extends BoolAbstractA {
    constructor(p: boolean[], q: boolean[]) {
        super([...p, ...q])
    }
    meth(): boolean[] {
        return [...super.meth(), true]
    }
}

class TupleThis {
    fld: [int, char]
    constructor(p: int, q: char) {
        this.fld = [p, q]
    }
    meth(): [int, int, char] {
        return [0, ...this.fld]
    }
}

abstract class TupleAbstractA {
    fld: [int, char]
    constructor(p: [int, char]) {
        this.fld = p
    }
    meth(): [int, int, char] {
        return [0, ...this.fld]
    }
}

class TupleConcreteB extends TupleAbstractA {
    constructor(p: int, q: char) {
        super([p, q])
    }
    meth(): [int, int, char] {
        return [...super.meth()]
    }
}

class IntFieldC {
    x: int = 13
}

function nullObjArr(): Object[]|null {
    return null
}

function nullTupleObjErr(): [Object, Error]|null {
    return null
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression in array literals and tuples")

    suite.addTest("empty arrays", () => {
        let a1: Object[] = [], a2: Error[] = []
        arktest.assertEQ([...a1, ...a2, new Object(), ...a1].length, 1)
    })

    suite.addTest("spread of char array", () => {
        let a: char[] = [c'b', c'c']
        let r = [c'a', ...a, c'd']
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(new String(r), "a,b,c,d")
    })

    suite.addTest("spread of byte array", () => {
        let a = [(2).toByte(), (3).toByte()]
        let r = [(1).toByte(), ...a, (4).toByte()]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
        arktest.assertEQ(r[2], 3)
        arktest.assertEQ(r[3], 4)
    })

    suite.addTest("spread of short array", () => {
        let a = [(2).toShort(), (3).toShort()]
        let r = [(1).toShort(), ...a, (4).toShort()]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
        arktest.assertEQ(r[2], 3)
        arktest.assertEQ(r[3], 4)
    })

    suite.addTest("spread of int array", () => {
        let a = [2, 3]
        let r = [1, ...a, 4]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
        arktest.assertEQ(r[2], 3)
        arktest.assertEQ(r[3], 4)
    })

    suite.addTest("spread of long array", () => {
        let a: long[] = [(2).toLong(), (3).toLong()]
        let r = [(1).toLong(), ...a, (4).toLong()]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
        arktest.assertEQ(r[2], 3)
        arktest.assertEQ(r[3], 4)
    })

    suite.addTest("spread of float array", () => {
        let a: float[] = [2.0f, 3.0f]
        let r = [1.0f, ...a, 4.0f]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1.f)
        arktest.assertEQ(r[1], 2.f)
        arktest.assertEQ(r[2], 3.f)
        arktest.assertEQ(r[3], 4.f)
    })

    suite.addTest("spread of double array", () => {
        let a = [2.0, 3.0]
        let r = [1.0, ...a, 4.0]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1.)
        arktest.assertEQ(r[1], 2.)
        arktest.assertEQ(r[2], 3.)
        arktest.assertEQ(r[3], 4.)
    })

    suite.addTest("spread of function array", () => {
        let a: F[] = [(p: number) => p * 2, (p: number) => p * 3]
        let b: F[] = [(p: number) => p * 4, (p: number) => p * 5]
        let res = 1.0
        for (let el of [...a, ...b]) res += el(res)
        arktest.assertEQ(res, 360.0)
    })

    suite.addTest("spread of enums", () => {
        let a: Color[] = [Color.Red, Color.Green]
        let b: Color[] = [Color.Blue]
        let r = [...a, ...b]
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], Color.Red)
        arktest.assertEQ(r[1], Color.Green)
        arktest.assertEQ(r[2], Color.Blue)
    })

    suite.addTest("spread of enums (numeric value)", () => {
        let a: Color[] = [Color.Red, Color.Green]
        let b: Color[] = [Color.Blue]
        let r = [...a, ...b]
        arktest.assertEQ(r.length, 3)
        arktest.assertEQ(r[0], 0)
        arktest.assertEQ(r[1], 1)
        arktest.assertEQ(r[2], 2)
    })

    suite.addTest("expression in parentheses", () => {
        let a1: Number[] = [1.0, 2.0]
        let r = [...(a1)]
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
    })

    suite.addTest("ensure-not-nullish expression (array OK)", () => {
        let a1: Object[]|null = [new Object]
        arktest.assertEQ([...a1!, new Object].length, 2)
    })

    suite.addTest("ensure-not-nullish expression NPE (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {[...a1!, new Object].length == 2}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let a2: Object[]|null = [new Object, new Object]
        arktest.assertEQ([...(a1 ?? a2)!, new Object].length, 3)
    })

    suite.addTest("function call in expression (number[])", () => {
        let foo = (): number[] => [1.0, 2.0]
        let r = [...foo()]
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
    })

    suite.addTest("function call in expression (str_filter)", () => {
        let strFilter = (s: string[], f: (p: string) => boolean): string[] => {
            let res: string[] = []
            for (let el of s) if (f(el)) res = [...res, el]
            return res
        }
        let s = ["h", "", "el", "", "", "l", "", "o"]
        let r = [...strFilter(s, (item: string) => item.length > 0), "!"]
        arktest.assertEQ(r.length, 5)
        arktest.assertEQ(String.join(r, ""), "hello!")
    })

    suite.addTest("new expression in expression", () => {
        let r = [...(new number[256](0))]
        arktest.assertEQ(r.length, 256)
        arktest.assertEQ(r[0], 0)
        arktest.assertEQ(r[255], 0)
    })

    suite.addTest("array index expression", () => {
        let a: double[][] = [[2.0, 3.0], []]
        let r = [1.0, ...a[0], ...a[1], 4.0]
        arktest.assertEQ(r.length, 4)
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
        arktest.assertEQ(r[2], 3)
        arktest.assertEQ(r[3], 4)
    })

    suite.addTest("tuple index expression", () => {
        let a: [double[], string] = [[2.0, 3.0, 4.0], "X"]
        let r = [0.0, 1.0, ...a[0]]
        arktest.assertEQ(r.reduce((sum: double, item: double) => sum + item, 0.0), 10.0)
    })

    suite.addTest("assignment expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        let r = [...a1, ...(a1 = a2), ...a1]
        arktest.assertEQ("".concat(...r), "123abab")
    })

    suite.addTest("conditional expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        let r = [...(a1.length > a2.length ? a1 : a2), "!"]
        arktest.assertEQ(String.join(r, ""), "123!")
    })

    suite.addTest("use 'this' expression (array)", () => {
        let a: BoolArrThis = new BoolArrThis([false, false], [true, true])
        arktest.assertEQ(a.meth().length, 7)
    })

    suite.addTest("use 'super' expression (array)", () => {
        let b: BoolConcreteB = new BoolConcreteB([false, false], [true, true])
        arktest.assertEQ(b.meth().length, 8)
    })

    suite.addTest("spread of arrays", () => {
        let a1: number[][] = [[1.0, 2.0], [3.0, 4.0]]
        let a2: number[][] = [[5.0, 6.0], []]
        let r: number[][] = [...a1, [], ...a2]
        arktest.assertEQ(r.length, 5)
    })

    suite.addTest("spread of arrays, flatten array", () => {
        let a1: number[][] = [[1.0, 2.0], [3.0, 4.0]]
        let a2: number[][] = [[5.0, 6.0], []]
        let res: number[] = []
        for (let el of [...a1, ...a2, [7.0]]) res = [...res, ...el]
        arktest.assertEQ(res.length, 7)
    })

    suite.addTest("union of types in result array", () => {
        let a1: Double[] = [1, 2, 3], a2: Short[] = [4, 5], a3: Byte[] = [6]
        let ar = [-10.0, ...a1, ...a1, ...a2, ...a2, ...a3, 10.0, ...a3]
        let res = 0.0
        for (let el of ar) res += (el as Numeric).toDouble()
        arktest.assertEQ(res, 42.0)
    })

    suite.addTest("union of reference types in result array", () => {
        let a1: (Object|undefined)[] = [new Int(), new Int(), undefined]
        let a2: (Number|undefined)[] = [42.0, 42.0, undefined]
        let r = [...a1, ...a2, 42.0]
        arktest.assertEQ(r[0], 0)
        arktest.assertEQ(r[1], 0)
        arktest.assertEQ(r[2], undefined)
        arktest.assertEQ(r[3], 42)
        arktest.assertEQ(r[4], 42)
        arktest.assertEQ(r[5], undefined)
        arktest.assertEQ(r[6], 42)
    })

    suite.addTest("union of reference types I/J in result array", () => {
        let a1: Array<I|null> = [{}, {}, null]
        let a2: Array<J|null> = [null, {}, {}]
        let r = [...a1, ...a2, 42.0]
        arktest.assertTrue(r[0] instanceof I)
        arktest.assertTrue(r[1] instanceof I)
        arktest.assertEQ(r[2], null)
        arktest.assertEQ(r[3], null)
        arktest.assertTrue(r[4] instanceof J)
        arktest.assertTrue(r[5] instanceof J)
        arktest.assertEQ(r[6], 42)
    })

    suite.addTest("use generics (array)", () => {
        let a1: (null|undefined|never|byte|int|long|double)[] = [1, 2, 3, 4.0]
        let a2: (GenA<String>|GenB<Long>|GenC<Error>|int)[] = [5, new GenA<String>, new GenB<Long>, new GenC<Error>]
        let r = [...a1, null, ...a2]
        arktest.assertEQ(r.length, 9)
        arktest.assertEQ(r[0], 1)
        arktest.assertTrue(r[8] instanceof GenC)
    })

    suite.addTest("lambda expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let r = [...((): string[] => a1)(), "!"]
        arktest.assertEQ(String.join(r, ""), "123!")
    })

    suite.addTest("lambda expression in expression (concat)", () => {
        let a1 = ["1", "2", "3"]
        let r = [...((p: string[], q: string[]): string[] => [...p, ...q])(a1, a1), "!"]
        arktest.assertEQ(String.join(r, ""), "123123!")
    })

    suite.addTest("spread a tuple to same type tuple", () => {
        let t1: [number, boolean] = [1.0, true]
        let r: [number, boolean] = [...t1]
        arktest.assertEQ(r[0], 1.0)
        arktest.assertTrue(r[1])
    })

    suite.addTest("spread tuples to different types", () => {
        let t1: [number] = [1.0]
        let t2: [boolean] = [true]
        let r: [number, number, boolean, boolean] = [...t1, ...t1, ...t2, ...t2]
        arktest.assertEQ(r[0], 1.0)
        arktest.assertEQ(r[1], 1.0)
        arktest.assertTrue(r[2] && r[3])
    })

    suite.addTest("spread tuples of all value types", () => {
        let t1: [byte, short] = [(1).toByte(), (2).toShort()]
        let t2: [int, long] = [3, (4).toLong()]
        let t3: [float, double, boolean] = [5.0f, 6.0, true]
        let t4: [char, string] = [c'Z', "X"]
        let r: [byte, short, int, long, float, double, boolean, char, string] = [...t1, ...t2, ...t3, ...t4]
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], 2)
        arktest.assertEQ(r[2], 3)
        arktest.assertEQ(r[3], 4)
        arktest.assertEQ(r[4], 5.0f)
        arktest.assertEQ(r[5], 6.0)
        arktest.assertEQ(r[6], true)
        arktest.assertEQ(r[7], c'Z')
        arktest.assertEQ(r[8], "X")
    })

    suite.addTest("ensure-not-nullish expression (tuple OK)", () => {
        let a1: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ([...a1!, new Object].length, 3)
    })

    suite.addTest("ensure-not-nullish expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        let len: int
        arktest.expectThrow(
            () => { len = [...a1!, new Object].length },
            expectedException
        )
    })

    suite.addTest("nullish-coalescing expression (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ([...(a1 ?? a2)!, new Object].length, 3)
    })

    suite.addTest("nullish-coalescing expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = nullTupleObjErr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        let len: int
        arktest.expectThrow(
            () =>  { len = [...(a1 ?? a2)!, new Object].length },
            expectedException
        )
    })

    suite.addTest("function call in expression (tuple)", () => {
        let foo = (): [number, string] => [1.0, "X"]
        let r: [number, string] = [...foo()]
        arktest.assertEQ(r[0], 1)
        arktest.assertEQ(r[1], "X")
    })

    suite.addTest("assignment expression in expression (tuple)", () => {
        let a1: [string, int] = ["1", 10]
        let a2: [string, int] = ["2", 20]
        let r: [string, int, string, int, string, int] = [...a1, ...(a1 = a2), ...a1]
        arktest.assertEQ(r[0], "1")
        arktest.assertEQ(r[1], 10)
        arktest.assertEQ(r[2], "2")
        arktest.assertEQ(r[3], 20)
        arktest.assertEQ(r[4], "2")
        arktest.assertEQ(r[5], 20)
    })

    suite.addTest("conditional expression in expression (tuple)", () => {
        let a1: [int, int] = [1, 2]
        let a2: [int, int] = [5, 6]
        let r: [int, int] = [...(a1[0] > a2[0] ? a1 : a2)]
        arktest.assertEQ(r[0], 5)
        arktest.assertEQ(r[1], 6)
    })

    suite.addTest("use this expression (tuple)", () => {
        let a: TupleThis = new TupleThis(1, c'Z')
        let r = a.meth()
        arktest.assertEQ(r[0], 0)
        arktest.assertEQ(r[1], 1)
        arktest.assertEQ(r[2], c'Z')
    })

    suite.addTest("use super expression (tuple)", () => {
        let b: TupleConcreteB = new TupleConcreteB(1, c'Z')
        let r = b.meth()
        arktest.assertEQ(r[0], 0)
        arktest.assertEQ(r[1], 1)
        arktest.assertEQ(r[2], c'Z')
    })

    suite.addTest("use generics (tuple)", () => {
        let a1: [int|GenA<String>, double|GenB<Long>] = [1, new GenB<Long>]
        let a2: [int|GenA<Long>, double|GenB<String>] = [new GenA<Long>, new GenB<String>]
        let r: [GenA<String>|int, GenB<Long>|double, GenA<Long>|int, GenB<String>|double] = [...a1, ...a2]
        arktest.assertTrue(r[0] == 1 && r[1] instanceof GenB && r[2] instanceof GenA && r[3] instanceof GenB)
    })

    suite.addTest("lambda expression in expression (tuple)", () => {
        let a2: [int, int] = [6, 5]
        let r: [int, int] = [...((p: [int, int]): [int, int] => [p[1], p[0]])(a2)]
        arktest.assertEQ(r[0], 5)
        arktest.assertEQ(r[1], 6)
    })

    suite.addTest("FixedArray and Array in spread expression", () => {
        let fa1: FixedArray<int> = [1]
        let a2: int[] = [2, 3, 4]
        let fa3: FixedArray<int> = [...fa1, ...a2]
        arktest.assertEQ(fa3.length, 4)
        arktest.assertEQ(fa3[0], 1)
        arktest.assertEQ(fa3[3], 4)
    })

    suite.addTest("FixedArray with class in spread expression", () => {
        let a: FixedArray<IntFieldC> = [new IntFieldC]
        let c = [...a]
        arktest.assertEQ(c.length, 1)
        arktest.assertEQ(c[0].x, 13)
    })

    suite.addTest("FixedArray with lambda in spread expression", () => {
        let a: int[] = [100, 101]
        let fa: FixedArray<int> = [
            ...((p: int[]): int[] => [p[1], p[0]])(a),
            10, 11,
            ...a]
        arktest.assertEQ(fa.length, 6)
        arktest.assertEQ(fa[0], 101)
        arktest.assertEQ(fa[2], 10)
        arktest.assertEQ(fa[4], 100)
    })

    suite.addTest("spread tuple of strings to tuple of Object", () => {
        let a: [string, string] = ['1', '2']
        let r: [Object, Object] = [...a]
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], '2')
    })

    suite.addTest("spread array of strings to array of Object", () => {
        let a: string[] = ['1', '2']
        let r: Array<Object> = [...a]
        arktest.assertEQ(r.length, 2)
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], '2')
    })

    suite.addTest("spread fixed array of strings to array of Object", () => {
        let a: FixedArray<string> = ['1', '2']
        let r: Array<Object> = [...a]
        arktest.assertEQ(r.length, 2)
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], '2')
    })

    suite.addTest("spread array of strings to fixed array of Object", () => {
        let a: Array<string> = ['1', '2']
        let r: FixedArray<Object> = [...a]
        arktest.assertEQ(r.length, 2)
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], '2')
    })

    suite.addTest("spread tuple to array", () => {
        let a: [string, string] = ['1', '2']
        let r: string[] = [...a]
        arktest.assertEQ(r.length, 2)
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], '2')
    })

    suite.addTest("spread tuple to FixedArray", () => {
        let a: [string, string] = ['1', '2']
        let r: FixedArray<string> = [...a]
        arktest.assertEQ(r.length, 2)
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], '2')
    })

    suite.addTest("spread tuple to array of Any", () => {
        let a: [string, int] = ['1', 2]
        let r: Any[] = [...a]
        arktest.assertEQ(r.length, 2)
        arktest.assertEQ(r[0], '1')
        arktest.assertEQ(r[1], 2)
    })

    suite.addTest("spread int[] to number[]", () => {
        let source = [1, 2, 3, 4]
        let target: number[] = [...source]
        arktest.assertEQ(4, target.length)
        arktest.assertEQ(2.0, target[1])
    })

    suite.addTest("spread int[] to float[]", () => {
        let source = [1, 2, 3, 4]
        let target: float[] = [...source]
        arktest.assertEQ(4, target.length)
        arktest.assertEQ(2.0f, target[1])
    })

    suite.addTest("spread int[] to double[]", () => {
        let source = [1, 2, 3, 4]
        let target: double[] = [...source]
        arktest.assertEQ(4, target.length)
        arktest.assertEQ(2.0, target[1])
    })

    suite.addTest("spread int[] to long[]", () => {
        let source = [1, 2, 3, 4]
        let target: long[] = [...source]
        arktest.assertEQ(4, target.length)
        arktest.assertEQ(2, target[1])
    })

    suite.addTest("spread float[] to double[]", () => {
        let source = [1.0f, 2.0f, 3.0f, 4.0f]
        let target: double[] = [...source]
        arktest.assertEQ(4, target.length)
        arktest.assertEQ(2.0, target[1])
    })

    return suite.run()
}
