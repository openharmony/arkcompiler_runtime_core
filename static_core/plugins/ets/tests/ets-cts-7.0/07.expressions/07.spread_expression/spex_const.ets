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

type FConst = (p: number) => number

enum ColorConst { Red, Green, Blue }

class ConsObjArr {
    fld: number
    constructor(...p: Object[]) {
        this.fld = p.length
    }
}

class ConsCharArr {
    fld: number
    constructor(...p: char[]) {
        this.fld = p.length
    }
}

class ConsByteArr {
    fld: int
    constructor(a: int, ...p: byte[]) {
        this.fld = a + reduce(p, (prev: int, next: byte) => prev + next, 0)
    }
}

class ConsShortArr {
    fld: int
    constructor(a: int, ...p: short[]) {
        this.fld = a + reduce(p, (prev: int, next: short) => prev + next, 0)
    }
}

class ConsLongArr {
    fld: long
    constructor(a: long, ...p: long[]) {
        this.fld = a + reduce(p, (prev: long, next: long) => prev + next, (0).toLong())
    }
}

class ConsFloatArr {
    fld: float
    constructor(a: float, ...p: float[]) {
        this.fld = a + reduce(p, (prev: float, next: float) => prev + next, 0.0f)
    }
}

class ConsNumberArr {
    fld: number
    constructor(a: number, ...p: number[]) {
        this.fld = a + reduce(p, (prev: number, next: number) => prev + next, 0.0)
    }
}

class ConsFArr {
    fld: number
    constructor(a: number, ...p: FConst[]) {
        let res = a
        for (let el of p) res += el(res)
        this.fld = res
    }
}

class ConsColorArr {
    fld: ColorConst
    constructor(...p: ColorConst[]) {
        this.fld = p.length ? p[p.length - 1] : ColorConst.Red
    }
}

class ConsParenNum {
    fld: number
    constructor(...p: number[]) {
        this.fld = p.length ? p[p.length - 1] : -1.0
    }
}

class ConsObjBase {
    fld: number
    constructor(a: int, ...p: Object[]) {
        this.fld = a + p.length
    }
}

class ConsObjB extends ConsObjBase {
    constructor(a: int, ...p: Object[]) {
        super(a, ...p)
    }
}

class AmethFoo {
    foo(): number[] {
        return [1.0, 2.0]
    }
}

class BmethCons {
    fld: number
    constructor(...p: number[]) {
        this.fld = reduce(p, (prev: number, next: number) => prev + next, 0.0)
    }
}

class ConsStrJoin {
    fld: string
    constructor(...p: string[]) {
        this.fld = String.join(p, "")
    }
}

class ConsNewExpr {
    fld: number
    constructor(...p: number[]) {
        this.fld = p.length ? p[p.length - 1] : -1
    }
}

class ConsArrIdx {
    fld: double
    constructor(...p: double[]) {
        this.fld = p[0] + p[1]
    }
}

class ConsTupIdx {
    fld: double
    constructor(...p: double[]) {
        this.fld = p[0] + p[1] + p[2]
    }
}

class ConsAssignStr {
    fld: string
    constructor(...p: string[]) {
        this.fld = String.join(p, "-")
    }
}

class AthisBoolArr {
    fld: boolean[]
    constructor(p: boolean[], q: boolean[]) {
        this.fld = [...p, ...q]
    }
    meth(): number {
        return new BboolFromInt(42, ...this.fld).fld
    }
}

class BboolFromInt {
    fld: number
    constructor(a: int, ...p: boolean[]) {
        this.fld = a + p.length
    }
}

abstract class AbsBoolArr {
    fld: boolean[]
    constructor(p: boolean[]) {
        this.fld = p
    }
    meth(): boolean[] {
        return this.fld
    }
}

class BboolSuper extends AbsBoolArr {
    constructor(p: boolean[], q: boolean[]) {
        super([...p, ...q])
    }
    bar(): number {
        return new CboolFromInt(42, ...super.meth()).fld
    }
}

class CboolFromInt {
    fld: number
    constructor(a: int, ...p: boolean[]) {
        this.fld = a + p.length
    }
}

class ConsArrCount {
    fld: number
    constructor(...p: number[][]) {
        let res = 0
        for (let el of p) res += el.length
        this.fld = res
    }
}

class ConsArrFlat {
    fld: number[]
    constructor(...p: number[][]) {
        let res: number[] = []
        for (let el of p) res = [...res, ...el]
        this.fld = res
    }
}

class ConsNumSum {
    fld: number
    constructor(...p: number[]) {
        let res = 0.0
        for (let el of p) res += el
        this.fld = res
    }
}

class ConsUnionVal {
    fld: double
    constructor(...p: (Double|Short|Byte)[]) {
        let res = 0.0
        for (let el of p) res += (el as Numeric).toDouble()
        this.fld = res
    }
}

class ConsUnionRef {
    fld: number
    constructor(...p: (Int|Number|String|undefined)[]) {
        this.fld = p.length
    }
}

class GenAConst<T> {}
class GenBConst<T> {}
class GenCConst<T> {}

class ZGenTUV<T, U, V> {
    fld: number
    constructor(...p: (GenAConst<T>|GenBConst<U>|GenCConst<V>|null)[]) {
        this.fld = p.length
    }
}

class ConsTupSwap {
    fld: [boolean, number]
    constructor(...p: [number, boolean]) {
        this.fld = [p[1], p[0]]
    }
}

class ConsTupTo4 {
    fld: [number, number, boolean, boolean]
    constructor(...p: [number, boolean]) {
        this.fld = [p[0], p[0], p[1], p[1]]
    }
}

class ConsAllValTup {
    fld: [byte, short, int, long, float, double, boolean, char, string, int]
    constructor(...p: [byte, short, int, long, float, double, boolean, char, string]) {
        this.fld = [...p, 42]
    }
}

class ConsObjErrTup {
    fld: int
    constructor(...p: [Object, Error]) {
        this.fld = p[0] != p[1] ? 3 : 0
    }
}

class ConsTupStaticFoo {
    fld: [string, number]
    constructor(...p: [number, string]) {
        this.fld = [p[1], p[0]]
    }
    static foo(): [number, string] {
        return [1.0, "X"]
    }
}

class ConsTupDoubleSum {
    fld: double
    constructor(...p: [double, double]) {
        this.fld = p[0] + p[1]
    }
}

class ConsTupAssign {
    fld: int
    constructor(...p: [string, int]) {
        this.fld = p[1]
    }
}

class ConsTupCond {
    fld: int
    constructor(...p: [int, int]) {
        this.fld = p[0] + p[1]
    }
}

class AtupThis {
    fld: [int, char]
    constructor(p: int, q: char) {
        this.fld = [p, q]
    }
    meth(): int {
        return 10 + new ZtupCons(...this.fld).fld
    }
}

class ZtupCons {
    fld: int
    constructor(...p: [int, char]) {
        this.fld = 100 + p[0]
    }
}

abstract class AbsTup {
    fld: [int, char]
    constructor(p: [int, char]) {
        this.fld = p
    }
    meth(): [int, char] {
        return this.fld
    }
}

class BtupSuper extends AbsTup {
    constructor(p: int, q: char) {
        super([p, q])
    }
    meth(): [int, char] {
        this.fld[0] = 10 + new ZtupCons2(...super.meth()).fld
        return this.fld
    }
}

class ZtupCons2 {
    fld: int
    constructor(...p: [int, char]) {
        this.fld = 100 + p[0]
    }
}

class ZTupGen<T, U> {
    fld: boolean
    constructor(...p: [GenAConst<T>, GenBConst<U>]) {
        this.fld = p[0] == p[1]
    }
}

class ConsTupLambda {
    fld: string
    constructor(...p: [int, int]) {
        this.fld = "" + p[0] + p[1]
    }
}

function nullObjArr(): Object[]|null {
    return null
}

function nullTupleObjErr(): [Object, Error]|null {
    return null
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression in constructor calls")

    suite.addTest("empty arrays", () => {
        let a1: Object[] = [], a2: Error[] = []
        arktest.assertEQ(new ConsObjArr(...[...a1, ...a2, new Object(), ...a1]).fld, 1)
    })

    suite.addTest("spread of char array", () => {
        let a: char[] = [c'b', c'c']
        arktest.assertEQ(new ConsCharArr(...a).fld, 2)
    })

    suite.addTest("spread of byte array", () => {
        let a: byte[] = [(1).toByte(), (2).toByte(), (3).toByte()]
        arktest.assertEQ(new ConsByteArr(42, ...a).fld, 48)
    })

    suite.addTest("spread of short array", () => {
        let a: short[] = [(1).toShort(), (2).toShort(), (3).toShort()]
        arktest.assertEQ(new ConsShortArr(42, ...a).fld, 48)
    })

    suite.addTest("spread of long array", () => {
        let a: long[] = [(1).toLong(), (2).toLong(), (3).toLong()]
        arktest.assertEQ(new ConsLongArr((42).toLong(), ...a).fld, 48)
    })

    suite.addTest("spread of float array", () => {
        let a: float[] = [1.0f, 2.0f, 3.0f]
        arktest.assertEQ(new ConsFloatArr(42.0f, ...a).fld, 48.0f)
    })

    suite.addTest("spread of number array", () => {
        let a: number[] = [1.0, 2.0, 3.0]
        arktest.assertEQ(new ConsNumberArr(42, ...a).fld, 48.0)
    })

    suite.addTest("spread of function array", () => {
        let a: FConst[] = [(p: number) => p * 2, (p: number) => p * 3]
        let b: FConst[] = [(p: number) => p * 4, (p: number) => p * 5]
        let obj = new ConsFArr(1.0, ...[...a, ...b])
        arktest.assertEQ(obj.fld, 360.0)
    })

    suite.addTest("spread of enums", () => {
        let a: ColorConst[] = [ColorConst.Red, ColorConst.Blue, ColorConst.Green]
        let obj = new ConsColorArr(...a)
        arktest.assertEQ(obj.fld, ColorConst.Green)
    })

    suite.addTest("expression in parentheses", () => {
        arktest.assertEQ(new ConsParenNum(...([1.0, 2.0])).fld, 2.0)
    })

    suite.addTest("ensure-not-nullish expression (array OK)", () => {
        let a1: Object[]|null = [new Object, new Object]
        let obj = new ConsObjB(5, ...a1!)
        arktest.assertEQ(obj.fld, 7)
    })

    suite.addTest("ensure-not-nullish expression NPE (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {new ConsObjBase(5, ...a1!)}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let a2: Object[]|null = [new Object, new Object]
        arktest.assertEQ(new ConsObjBase(5, ...(a1 ?? a2)!).fld, 7)
    })

    suite.addTest("method call in expression (number[])", () => {
        let a = new AmethFoo()
        let b = new BmethCons(...a.foo())
        arktest.assertEQ(b.fld, 3.0)
    })

    suite.addTest("method call in expression (str_filter)", () => {
        let s = ["h", "", "el", "", "", "l", "", "o"]
        arktest.assertEQ(new ConsStrJoin(...((s: string[]): string[] => {
            let res: string[] = []
            for (let el of s) if (el.length > 0) res = [...res, el]
            return res
        })(s)).fld, "hello")
    })

    suite.addTest("new expression in expression", () => {
        arktest.assertEQ(new ConsNewExpr(...(new number[256](0))).fld, 0)
    })

    suite.addTest("array index expression", () => {
        let a: double[][] = [[2.0, 3.0], []]
        arktest.assertEQ(new ConsArrIdx(...a[0]).fld, 5)
    })

    suite.addTest("tuple index expression (array element)", () => {
        let a: [string, double[], string] = ["X", [2.0, 3.0, 4.0], "Z"]
        let obj = new ConsTupIdx(...a[1])
        arktest.assertEQ(obj.fld, 9)
    })

    suite.addTest("assignment expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        let obj = new ConsAssignStr(...(a1 = a2))
        arktest.assertEQ((obj).fld, "a-b")
    })

    suite.addTest("conditional expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        arktest.assertEQ(new ConsAssignStr(...(a1.length > a2.length ? a1 : a2)).fld, "1-2-3")
    })

    suite.addTest("use this expression (array)", () => {
        let a: AthisBoolArr = new AthisBoolArr([false, false], [true, true])
        arktest.assertEQ(a.meth(), 46)
    })

    suite.addTest("use super expression (array)", () => {
        let b: BboolSuper = new BboolSuper([false, false], [true, true])
        arktest.assertEQ(b.bar(), 46)
    })

    suite.addTest("spread of arrays", () => {
        let a1: number[][] = [[1.0, 2.0], [], [3.0, 4.0], [5.0]]
        arktest.assertEQ(new ConsArrCount(...a1).fld, 5)
    })

    suite.addTest("spread of arrays, flatten array", () => {
        let a1: number[][] = [[1.0, 2.0], [3.0, 4.0]]
        let a2: number[][] = [[5.0, 6.0], []]
        arktest.assertEQ(new ConsNumSum(...[...new ConsArrFlat(...a1).fld, ...new ConsArrFlat(...a2).fld]).fld, 21.0)
    })

    suite.addTest("union of value types in result array", () => {
        let a1: Double[] = [1, 2, 3], a2: Short[] = [4, 5], a3: Byte[] = [6]
        arktest.assertEQ(new ConsUnionVal(...[-10.0, ...a1, ...a1, ...a2, ...a2, ...a3, 10.0, ...a3]).fld, 42.0)
    })

    suite.addTest("union of reference types in result array", () => {
        let a1: (Int|Number|String|undefined)[] = [new Int(), new Int(), undefined, "Z", 42.0]
        arktest.assertEQ(new ConsUnionRef(...a1).fld, 5)
    })

    suite.addTest("use generics (array)", () => {
        let a1: (GenAConst<String>|GenBConst<Long>|GenCConst<Error>|null)[] = [new GenAConst<String>, new GenBConst<Long>, new GenCConst<Error>]
        let obj = new ZGenTUV<String, Long, Error>(...a1)
        arktest.assertEQ(obj.fld, 3)
    })

    suite.addTest("lambda expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        arktest.assertEQ(new ConsStrJoin(...((): string[] => a1)()).fld, "123")
    })

    suite.addTest("lambda expression in expression (concat)", () => {
        let a1 = ["1", "2", "3"]
        arktest.assertEQ(new ConsStrJoin(...((p: string[], q: string[]): string[] => [...p, ...q])(a1, a1)).fld, "123123")
    })

    suite.addTest("spread a tuple to same type tuple", () => {
        let t1: [number, boolean] = [1.0, true]
        let r: [boolean, number] = new ConsTupSwap(...t1).fld
        arktest.assertTrue(r[0] && r[1] == 1.0)
    })

    suite.addTest("spread tuples to different types", () => {
        let t: [number, boolean] = [1.0, true]
        let r: [number, number, boolean, boolean] = new ConsTupTo4(...t).fld
        arktest.assertTrue(r[0] == 1.0 && r[1] == 1.0 && r[2] && r[3])
    })

    suite.addTest("spread tuples of all value types", () => {
        let r: [byte, short, int, long, float, double, boolean, char, string] =
            [(1).toByte(), (2).toShort(), 3, (4).toLong(), 5.0f, 6.0, true, c'Z', "X"]
        arktest.assertEQ(new ConsAllValTup(...r).fld[9], 42)
    })

    suite.addTest("ensure-not-nullish expression (tuple OK)", () => {
        let a1: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ(new ConsObjErrTup(...a1!).fld, 3)
    })

    suite.addTest("ensure-not-nullish expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {new ConsObjErrTup(...a1!).fld == 3}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = [new Object, new Error]
        let obj = new ConsObjErrTup(...(a1 ?? a2)!)
        arktest.assertEQ(obj.fld, 3)
    })

    suite.addTest("nullish-coalescing expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = nullTupleObjErr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {new ConsObjErrTup(...(a1 ?? a2)!).fld == 3}, expectedException)
    })

    suite.addTest("function call in expression (tuple)", () => {
        let r: [string, number] = new ConsTupStaticFoo(...ConsTupStaticFoo.foo()).fld
        arktest.assertTrue(r[0] == "X" && r[1] == 1)
    })

    suite.addTest("tuple index expression (nested tuple)", () => {
        let a: [[double, double], [char, char]] = [[2.0, 3.0], [c'Z', c'X']]
        let obj = new ConsTupDoubleSum(...a[0])
        arktest.assertEQ(obj.fld, 5.0)
    })

    suite.addTest("assignment expression in expression (tuple)", () => {
        let a1: [string, int] = ["1", 10]
        let a2: [string, int] = ["2", 20]
        arktest.assertEQ(new ConsTupAssign(...(a1 = a2)).fld, 20)
    })

    suite.addTest("conditional expression in expression (tuple)", () => {
        let a1: [int, int] = [1, 2]
        let a2: [int, int] = [5, 6]
        let r = new ConsTupCond(...(a1[0] > a2[0] ? a1 : a2)).fld
        arktest.assertEQ(r, 11)
    })

    suite.addTest("use this expression (tuple)", () => {
        let a: AtupThis = new AtupThis(1, c'Z')
        arktest.assertEQ(a.meth(), 111)
    })

    suite.addTest("use super expression (tuple)", () => {
        let b: BtupSuper = new BtupSuper(1, c'Z')
        arktest.assertEQ(b.meth()[0], 111)
    })

    suite.addTest("use generics (tuple)", () => {
        let a1: [GenAConst<Long>, GenBConst<Error>] = [new GenAConst<Long>, new GenBConst<Error>]
        arktest.assertFalse(new ZTupGen<Long, Error>(...a1).fld)
    })

    suite.addTest("lambda expression in expression (tuple)", () => {
        let a2: [int, int] = [6, 5]
        let r = new ConsTupLambda(...((p: [int, int]): [int, int] => [p[1], p[0]])(a2)).fld
        arktest.assertEQ(r, "56")
    })

    return suite.run()
}
