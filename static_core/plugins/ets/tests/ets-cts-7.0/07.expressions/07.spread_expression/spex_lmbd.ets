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
desc: If an array is spread while calling a function, then an appropriate parameter must be of the spread array kind.
---*/

type F = (p: number) => number

enum Color { Red, Green, Blue }

class ArrayThisClass {
    fld: boolean[]
    constructor(p: boolean[], q: boolean[]) {
        this.fld = [...p, ...q]
    }
    meth(): int {
        return ((a: int, ...p: boolean[]): int => a + p.length)(42, ...this.fld)
    }
}

abstract class AbstractArrayA {
    fld: boolean[]
    constructor(p: boolean[]) {
        this.fld = p
    }
    meth(): boolean[] {
        return this.fld
    }
}

class ConcreteArrayB extends AbstractArrayA {
    constructor(p: boolean[], q: boolean[]) {
        super([...p, ...q])
    }
    bar(): int {
        return ((a: int, ...p: boolean[]): int => a + p.length)(42, ...super.meth())
    }
}

class TupleThisClass {
    fld: [int, char]
    constructor(p: int, q: char) {
        this.fld = [p, q]
    }
    meth(): int {
        return 10 + ((...p: [int, char]): int => 100 + p[0])(...this.fld)
    }
}

abstract class AbstractTupleA {
    fld: [int, char]
    constructor(p: [int, char]) {
        this.fld = p
    }
    meth(): [int, char] {
        return this.fld
    }
}

class ConcreteTupleB extends AbstractTupleA {
    constructor(p: int, q: char) {
        super([p, q])
    }
    meth(): [int, char] {
        this.fld[0] = 10 + ((...p: [int, char]): int => 100 + p[0])(...super.meth())
        return this.fld
    }
}

function nullTuple(): [Object, Error]|null {
    return null
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression with lambda calls (arrays and tuples)")

    suite.addTest("empty arrays", () => {
        let a1: Object[] = [], a2: Error[] = []
        arktest.assertEQ(((...p: Object[]): int => p.length)(...[...a1, ...a2, new Object(), ...a1]), 1)
    })

    suite.addTest("spread of char array", () => {
        let a: char[] = [c'b', c'c']
        arktest.assertEQ(((...p: char[]): int => p.length)(...a), 2)
    })

    suite.addTest("spread of byte array", () => {
        let a: byte[] = [(1).toByte(), (2).toByte(), (3).toByte()]
        arktest.assertEQ(((a: int, ...p: byte[]): int => a + reduce(p, (prev: int, next: byte) => prev + next, 0))(42, ...a), 48)
    })

    suite.addTest("spread of short array", () => {
        let a: short[] = [(1).toShort(), (2).toShort(), (3).toShort()]
        arktest.assertEQ(((a: int, ...p: short[]): int => a + reduce(p, (prev: int, next: short) => prev + next, 0))(42, ...a), 48)
    })

    suite.addTest("spread of long array", () => {
        let a: long[] = [(1).toLong(), (2).toLong(), (3).toLong()]
        arktest.assertEQ(((a: long, ...p: long[]): long => a + reduce(p, (prev: long, next: long) => prev + next, (0).toLong()))(42, ...a), 48)
    })

    suite.addTest("spread of float array", () => {
        let a: float[] = [1.0f, 2.0f, 3.0f]
        arktest.assertEQ(((a: float, ...p: float[]): float => a + reduce(p, (prev: float, next: float) => prev + next, 0.0f))(42.0f, ...a), 48.0f)
    })

    suite.addTest("spread of number array", () => {
        let a: number[] = [1.0, 2.0, 3.0]
        arktest.assertEQ(((a: number, ...p: number[]): number => a + reduce(p, (prev: number, next: number) => prev + next, 0.0))(42, ...a), 48.0)
    })

    suite.addTest("spread of function array", () => {
        let a: F[] = [(p: number) => p * 2, (p: number) => p * 3]
        let b: F[] = [(p: number) => p * 4, (p: number) => p * 5]
        arktest.assertTrue(((a: number, ...p: F[]): number => {
            let res = a
            for (let el of p) res += el(res)
            return res
        })(1.0, ...[...a, ...b]) == 360.0)
    })

    suite.addTest("spread of enums", () => {
        let a: Color[] = [Color.Red, Color.Blue, Color.Green]
        arktest.assertEQ(((...p: Color[]): Color => p.length ? p[p.length - 1] : Color.Red)(...a), Color.Green)
    })

    suite.addTest("expression in parentheses", () => {
        arktest.assertEQ(((...p: number[]): number => p.length ? p[p.length - 1] : -1.0)(...([1.0, 2.0])), 2.0)
    })

    suite.addTest("ensure-not-nullish expression (array)", () => {
        let a1: Object[]|null = [new Object, new Object]
        arktest.assertEQ(((a: int, ...p: Object[]): int => a + p.length)(5, ...a1!), 7)
    })

    suite.addTest("ensure-not-nullish expression NPE (array)", () => {
        let a1: Object[]|null = null
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {((a: int, ...p: Object[]): int => a + p.length)(5, ...a1!)}, expectedException)
    })

    suite.addTest("nullish expression (array)", () => {
        let a1: Object[]|null = [new Object, new Object]
        arktest.assertEQ(((a: int, ...p: Object[]): int => a + p.length)(5, ...a1!), 7)
    })

    suite.addTest("nullish-coalescing expression (array)", () => {
        let a1: Object[]|null = null
        let a2: Object[]|null = [new Object, new Object]
        arktest.assertEQ(((a: int, ...p: Object[]): int => a + p.length)(5, ...(a1 ?? a2)!), 7)
    })

    suite.addTest("function call in expression (number[])", () => {
        let foo = (): number[] => [1.0, 2.0]
        arktest.assertEQ(((...p: number[]): number => reduce(p, (prev: number, next: number) => prev + next, 0.0))(...foo()), 3.0)
    })

    suite.addTest("function call in expression (string[])", () => {
        let s = ["h", "", "el", "", "", "l", "", "o"]
        arktest.assertEQ(((...p: string[]): string => String.join(p, ""))(...((s: string[]): string[] => {
            let res: string[] = []
            for (let el of s) if (el.length > 0) res = [...res, el]
            return res
        })(s)), "hello")
    })

    suite.addTest("new expression in expression", () => {
        arktest.assertEQ(((...p: number[]): number =>  p.length ? p[p.length - 1] : -1)(...(new number[256](0))), 0)
    })

    suite.addTest("array index expression", () => {
        let a: double[][] = [[2.0, 3.0], []]
        arktest.assertEQ(((...p: double[]): double => p[0] + p[1])(...a[0]), 5)
    })

    suite.addTest("tuple index expression (array element)", () => {
        let a: [string, double[], string] = ["X", [2.0, 3.0, 4.0], "Z"]
        arktest.assertEQ(((...p: double[]): double => p[0] + p[1] + p[2])(...a[1]), 9)
    })

    suite.addTest("assignment expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        arktest.assertEQ(((...p: string[]): string => String.join(p, "-"))(...(a1 = a2)), "a-b")
    })

    suite.addTest("conditional expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        arktest.assertEQ(((...p: string[]): string => String.join(p, "-"))(...(a1.length > a2.length ? a1 : a2)), "1-2-3")
    })

    suite.addTest("use this expression (array)", () => {
        let a: ArrayThisClass = new ArrayThisClass([false, false], [true, true])
        arktest.assertEQ(a.meth(), 46)
    })

    suite.addTest("use super expression (array)", () => {
        let b: ConcreteArrayB = new ConcreteArrayB([false, false], [true, true])
        arktest.assertEQ(b.bar(), 46)
    })

    suite.addTest("spread of arrays", () => {
        let a1: number[][] = [[1.0, 2.0], [], [3.0, 4.0], [5.0]]
        arktest.assertEQ(((...p: number[][]): int => {
            let res = 0
            for (let el of p) res += el.length
            return res
        })(...a1), 5)
    })

    suite.addTest("spread of arrays, flatten array", () => {
        let bar = (...p: number[]): number => {
            let res = 0.0
            for (let el of p) res += el
            return res
        }
        let a1: number[][] = [[1.0, 2.0], [3.0, 4.0]]
        let a2: number[][] = [[5.0, 6.0], []]
        arktest.assertEQ(bar(...[
            ...((...p: number[][]): number[] => {
                let res: number[] = []
                for (let el of p) res = [...res, ...el]
                return res
            })(...a1),
            ...((...p: number[][]): number[] => {
                let res: number[] = []
                for (let el of p) res = [...res, ...el]
                return res
            })(...a2)
        ]), 21.0)
    })

    suite.addTest("union of value types in result array", () => {
        let a1: Double[] = [1, 2, 3], a2: Short[] = [4, 5], a3: Byte[] = [6]
        let ar = [-10.0, ...a1, ...a1, ...a2, ...a2, ...a3, 10.0, ...a3]
        arktest.assertEQ(((...p: (Double|Short|Byte)[]): double => {
            let res = 0.0
            for (let el of p) res += (el as Numeric).toDouble()
            return res
        })(...ar), 42.0)
    })

    suite.addTest("union of reference types in result array", () => {
        let a1: (Int|Number|String|undefined)[] = [new Int(), new Int(), undefined, "Z", 42.0]
        arktest.assertEQ(((...p: (Int|Number|String|undefined)[]): int => p.length)(...a1), 5)
    })

    suite.addTest("lambda expression in expression (string[])", () => {
        let a1 = ["1", "2", "3"]
        arktest.assertEQ(((...p: string[]): string => String.join(p, ""))(...((): string[] => a1)()), "123")
    })

    suite.addTest("lambda expression in expression (concat)", () => {
        let a1 = ["1", "2", "3"]
        arktest.assertEQ(((...p: string[]): string => String.join(p, ""))(...((p: string[], q: string[]): string[] => [...p, ...q])(a1, a1)), "123123")
    })

    suite.addTest("spread a tuple to same type tuple", () => {
        let t1: [number, boolean] = [1.0, true]
        let r: [boolean, number] = ((...p: [number, boolean]): [boolean, number] => [p[1], p[0]])(...t1)
        arktest.assertTrue(r[0] && r[1] == 1.0)
    })

    suite.addTest("spread tuples to different types", () => {
        let r: [number, number, boolean, boolean] = ((...p: [number, boolean]): [number, number, boolean, boolean] => [p[0], p[0], p[1], p[1]])(...[1.0, true])
        arktest.assertTrue(r[0] == 1.0 && r[1] == 1.0 && r[2] && r[3])
    })

    suite.addTest("spread tuples of all value types", () => {
        let r: [byte, short, int, long, float, double, boolean, char, string] =
            [(1).toByte(), (2).toShort(), 3, (4).toLong(), 5.0f, 6.0, true, c'Z', "X"]
        arktest.assertEQ(((...p: [byte, short, int, long, float, double, boolean, char, string]):
            [byte, short, int, long, float, double, boolean, char, string, int] =>
            [...p, 42])(...r)[9], 42)
    })

    suite.addTest("ensure-not-nullish expression (tuple)", () => {
        let a1: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ(((...p: [Object, Error]): int => p[0] != p[1] ? 3 : 0)(...a1!), 3)
    })

    suite.addTest("ensure-not-nullish expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = null
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {((...p: [Object, Error]): int => p[0] != p[1] ? 3 : 0)(...a1!)}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (tuple OK)", () => {
        let a1: [Object, Error]|null = null
        let a2: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ(((...p: [Object, Error]): int => p[0] != p[1] ? 3 : 0)(...(a1 ?? a2)!), 3)
    })

    suite.addTest("nullish-coalescing expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTuple()
        let a2: [Object, Error]|null = nullTuple()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {((...p: [Object, Error]): int => p[0] != p[1] ? 3 : 0)(...(a1 ?? a2)!)}, expectedException)
    })

    suite.addTest("function call in expression (tuple)", () => {
        let foo = (): [number, string] => [1.0, "X"]
        let r: [string, number] = ((...p: [number, string]): [string, number] => [p[1], p[0]])(...foo())
        arktest.assertTrue(r[0] == "X" && r[1] == 1)
    })

    suite.addTest("tuple index expression (nested tuple)", () => {
        let a: [[double, double], [char, char]] = [[2.0, 3.0], [c'Z', c'X']]
        arktest.assertEQ(((...p: [double, double]): double => p[0] + p[1])(...a[0]), 5.0)
    })

    suite.addTest("assignment expression in expression (tuple)", () => {
        let a1: [string, int] = ["1", 10]
        let a2: [string, int] = ["2", 20]
        arktest.assertEQ(((...p: [string, int]): int => p[1])(...(a1 = a2)), 20)
    })

    suite.addTest("conditional expression in expression (tuple)", () => {
        let a1: [int, int] = [1, 2]
        let a2: [int, int] = [5, 6]
        let r = ((...p: [int, int]): int => p[0] + p[1])(...(a1[0] > a2[0] ? a1 : a2))
        arktest.assertEQ(r, 11)
    })

    suite.addTest("use this expression (tuple)", () => {
        let a: TupleThisClass = new TupleThisClass(1, c'Z')
        arktest.assertEQ(a.meth(), 111)
    })

    suite.addTest("use super expression (tuple)", () => {
        let b: ConcreteTupleB = new ConcreteTupleB(1, c'Z')
        arktest.assertEQ(b.meth()[0], 111)
    })

    suite.addTest("lambda expression in expression (tuple)", () => {
        let a2: [int, int] = [6, 5]
        let r = ((...p: [int, int]): string => "" + p[0] + p[1])(...((p: [int, int]): [int, int] => [p[1], p[0]])(a2))
        arktest.assertEQ(r, "56")
    })

    return suite.run()
}
