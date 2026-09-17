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
desc: A spread expression can be used in function calls.
---*/

type FFunc = (p: number) => number

enum ColorFunc { Red, Green, Blue }

class GenAFunc<T> {}
class GenBFunc<T> {}
class GenCFunc<T> {}

class F20ThisBool {
    fld: boolean[]
    constructor(p: boolean[], q: boolean[]) {
        this.fld = [...p, ...q]
    }
    meth(): number {
        return f20BoolFromInt(42, ...this.fld)
    }
}

abstract class F21AbsBool {
    fld: boolean[] = []
    constructor(p: boolean[]) {
        this.fld = p
    }
    meth(): boolean[] {
        return this.fld
    }
}

class F21BoolB extends F21AbsBool {
    constructor(p: boolean[], q: boolean[]) {
        super([...p, ...q])
    }
    bar(): number {
        return f21BoolFromInt(42, ...super.meth())
    }
}

class F40ThisTup {
    fld: [int, char]
    constructor(p: int, q: char) {
        this.fld = [p, q]
    }
    meth(): int {
        return 10 + f40TupFoo(...this.fld)
    }
}

abstract class F41AbsTup {
    fld: [int, char]
    constructor(p: [int, char]) {
        this.fld = p
    }
    meth(): [int, char] {
        return this.fld
    }
}

class F41TupB extends F41AbsTup {
    constructor(p: int, q: char) {
        super([p, q])
    }
    meth(): [int, char] {
        this.fld[0] = 10 + f41TupFoo(...super.meth())
        return this.fld
    }
}

class F69ThisFABool {
    fld: FixedArray<boolean>
    constructor(p: FixedArray<boolean>, q: FixedArray<boolean>) {
        this.fld = [...p, ...q]
    }
    meth(): number {
        return f69BoolFromInt(42, ...this.fld)
    }
}

abstract class F70AbsFABool {
    fld: FixedArray<boolean> = []
    constructor(p: FixedArray<boolean>) {
        this.fld = p
    }
    meth(): FixedArray<boolean> {
        return this.fld
    }
}

class F70FAB extends F70AbsFABool {
    constructor(p: FixedArray<boolean>, q: FixedArray<boolean>) {
        super([...p, ...q])
    }
    bar(): number {
        return f70BoolFromInt(42, ...super.meth())
    }
}

function f00ObjArr(...p: Object[]): number { return p.length }
function f01CharArr(...p: char[]): number { return p.length }
function f02ByteArr(a: int, ...p: byte[]): int { return a + reduce(p, (prev: int, next: byte) => prev + next, 0) }
function f03ShortArr(a: int, ...p: short[]): int { return a + reduce(p, (prev: int, next: short) => prev + next, 0) }
function f04LongArr(a: long, ...p: long[]): long { return a + reduce(p, (prev: long, next: long) => prev + next, (0).toLong()) }
function f05FloatArr(a: float, ...p: float[]): float { return a + reduce(p, (prev: float, next: float) => prev + next, 0.0f) }
function f06NumberArr(a: number, ...p: number[]): number { return a + reduce(p, (prev: number, next: number) => prev + next, 0.0) }
function f07FArr(a: number, ...p: FFunc[]): number {
    let res = a
    for (let el of p) res += el(res)
    return res
}
function f08ColorArr(...p: ColorFunc[]): ColorFunc { return p.length ? p[p.length - 1] : ColorFunc.Red }
function f09ParenNum(...p: number[]): number { return p.length ? p[p.length - 1] : -1.0 }
function f10ObjEnsure(a: int, ...p: Object[]): number { return a + p.length }
function f11ObjNpe(a: int, ...p: Object[]): number { return a + p.length }
function f12NullCoalAny(a: int, ...p: Any[]): number { return a + p.length }
function f13aFooNumArr(): number[] { return [1.0, 2.0] }
function f13bBarNumArr(...p: number[]): number { return reduce(p, (prev: number, next: number) => prev + next, 0.0) }
function f14StrJoin(...p: string[]): string { return String.join(p, '') }
function f15NewExpr(...p: number[]): number { return p.length ? p[p.length - 1] : -1 }
function f16ArrIdx(...p: double[]): double { return p[0] + p[1] }
function f17TupIdx(...p: double[]): double { return p[0] + p[1] + p[2] }
function f18AssignStr(...p: string[]): string { return String.join(p, '-') }
function f19CondStr(...p: string[]): string { return String.join(p, '-') }
function f20BoolFromInt(a: int, ...p: boolean[]): number { return a + p.length }
function f21BoolFromInt(a: int, ...p: boolean[]): number { return a + p.length }
function f22ArrCount(...p: number[][]): number {
    let res = 0
    for (let el of p) res += el.length
    return res
}
function f23aFlat(...p: number[][]): number[] {
    let res: number[] = []
    for (let el of p) res = [...res, ...el]
    return res
}
function f23bSum(...p: number[]): number {
    let res = 0.0
    for (let el of p) res += el
    return res
}
function f24Union(...p: (Double|Short|Byte)[]): double {
    let res = 0.0
    for (let el of p) res += (el as Numeric).toDouble()
    return res
}
function f25UnionRef(...p: (Int|Number|String|undefined)[]): number { return p.length }
function f26Generics<T, U, V>(...p: (GenAFunc<T>|GenBFunc<U>|GenCFunc<V>|null)[]): number { return p.length }
function f27StrLambda(...p: string[]): string { return String.join(p, '') }
function f29TupSwap(...p: [number, boolean]): [boolean, number] { return [p[1], p[0]] }
function f30TupTo4(...p: [number, boolean]): [number, number, boolean, boolean] { return [p[0], p[0], p[1], p[1]] }
function f31AllValTup(...p: [byte, short, int, long, float, double, boolean, char, string]): [byte, short, int, long, float, double, boolean, char, string, int] {
    return [...p, 42]
}
function f32ObjErrTup(...p: [Object, Error]): int { return p[0] != p[1] ? 3 : 0 }
function f36aFooTup(): [number, string] { return [1.0, 'X'] }
function f36bBarTup(...p: [number, string]): [string, number] { return [p[1], p[0]] }
function f37TupDoubleSum(...p: [double, double]): double { return p[0] + p[1] }
function f38TupAssign(...p: [string, int]): int { return p[1] }
function f39TupCond(...p: [int, int]): int { return p[0] + p[1] }
function f40TupFoo(...p: [int, char]): int { return 100 + p[0] }
function f41TupFoo(...p: [int, char]): int { return 100 + p[0] }
function f42TupGen<T, U>(...p: [GenAFunc<T>, GenBFunc<U>]): boolean { return p[0] == p[1] }
function f43TupLambda(...p: [int, int]): string { return '' + p[0] + p[1] }
function f44RecArr(...p: Record<int, string>[]): number {
    arktest.assertEQ(p[0][1], '1')
    return p.length
}

function f46FAObjArr(...p: Array<Object>): number { return p.length }
function f47FACharArr(...p: FixedArray<char>): number { return p.length }
function f48FAByteArr(a: int, ...p: Array<byte>): int { return a + reduce(p, (prev: int, next: byte) => prev + next, 0) }
function f49FAShortArr(a: int, ...p: FixedArray<short>): int { return a + reduce(p, (prev: int, next: short) => prev + next, 0) }
function f50FALongArr(a: long, ...p: FixedArray<long>): long { return a + reduce(p, (prev: long, next: long) => prev + next, (0).toLong()) }
function f51FAFloatArr(a: float, ...p: FixedArray<float>): float { return a + reduce(p, (prev: float, next: float) => prev + next, 0.0f) }
function f52FANumArr(a: number, ...p: number[]): number { return a + reduce(p, (prev: number, next: number) => prev + next, 0.0) }
function f54FAColorArr(...p: ColorFunc[]): ColorFunc { return p.length ? p[p.length - 1] : ColorFunc.Red }
function f55FAParen(...p: FixedArray<Object>): number { return (p.length ? p[p.length - 1] : -1.0) as number }
function f56FAEnsure(a: int, ...p: FixedArray<Object>): int { return a + p.length }
function f57FANpe(a: int, ...p: FixedArray<Object>): int { return a + p.length }
function f58FANullCoal(a: int, ...p: FixedArray<Object>): int { return a + p.length }
function f59aFAFooNum(): FixedArray<number> { return [1.0, 2.0] }
function f59bFABarNum(...p: FixedArray<number>): number { return reduce(p, (prev: number, next: number) => prev + next, 0.0) }
function f60FAFilterInt(...p: FixedArray<int>): int { return reduce(p, (p: int, c: int) => p + c, 0) }
function f61FANewExpr(...p: FixedArray<number>): number { return p.length ? p[p.length - 1] : -1 }
function f62FAArrIdx(...p: FixedArray<double>): double { return p[0] + p[1] }
function f63FATupIdx(...p: Array<double>): double { return p[0] + p[1] + p[2] }
function f64FAAssignLong(...p: long[]): string { return reduce(p, (p: string, c: long) => p + c, '') }
function f65FACondByte(...p: FixedArray<byte>): string { return reduce(p, (p: string, c: byte) => p + c, '') }
function f69BoolFromInt(a: int, ...p: FixedArray<boolean>): number { return a + p.length }
function f70BoolFromInt(a: int, ...p: FixedArray<boolean>): number { return a + p.length }
function f71FAArrCount(...p: FixedArray<FixedArray<number>>): number {
    let res = 0
    for (let el of p) res += el.length
    return res
}
function f72aFAFlat(...p: FixedArray<FixedArray<number>>): FixedArray<number> {
    let res: FixedArray<number> = []
    for (let el of p) res = [...res, ...el]
    return res
}
function f72bFASum(...p: FixedArray<number>): number {
    let res = 0.0
    for (let el of p) res += el
    return res
}
function f73FAUnion(...p: Array<Double|Short|Byte>): double {
    let res = 0.0
    for (let el of p) res += (el as Numeric).toDouble()
    return res
}
function f74FAUnionRef(...p: FixedArray<Int|Number|String|undefined>): number { return p.length }
function f76FAStrLambda(...p: FixedArray<string>): string { return [...p].reduce((p: string, c: string) => p + c, '') }
function f77FAStrLambda2(...p: Array<string>): string { return p.reduce((p: string, c: string) => p + c, '') }

function nullObjArr(): Object[]|null { return null }
function nullTupleObjErr(): [Object, Error]|null { return null }
function nullFAObj(): FixedArray<Object>|null { return null }

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression in function calls")

    suite.addTest("empty arrays", () => {
        let a1: Object[] = [], a2: Error[] = []
        arktest.assertEQ(f00ObjArr(...[...a1, ...a2, new Object(), ...a1]), 1)
    })

    suite.addTest("spread of char array", () => {
        let a: char[] = [c'b', c'c']
        arktest.assertEQ(f01CharArr(...a), 2)
    })

    suite.addTest("spread of byte array", () => {
        let a: byte[] = [(1).toByte(), (2).toByte(), (3).toByte()]
        arktest.assertEQ(f02ByteArr(42, ...a), 48)
    })

    suite.addTest("spread of short array", () => {
        let a: short[] = [(1).toShort(), (2).toShort(), (3).toShort()]
        arktest.assertEQ(f03ShortArr(42, ...a), 48)
    })

    suite.addTest("spread of long array", () => {
        let a: long[] = [(1).toLong(), (2).toLong(), (3).toLong()]
        arktest.assertEQ(f04LongArr((42).toLong(), ...a), 48)
    })

    suite.addTest("spread of float array", () => {
        let a: float[] = [1.0f, 2.0f, 3.0f]
        arktest.assertEQ(f05FloatArr(42.0f, ...a), 48.0f)
    })

    suite.addTest("spread of number array", () => {
        let a: number[] = [1.0, 2.0, 3.0]
        arktest.assertEQ(f06NumberArr(42, ...a), 48.0)
    })

    suite.addTest("spread of function array", () => {
        let a: FFunc[] = [(p: number) => p * 2, (p: number) => p * 3]
        let b: FFunc[] = [(p: number) => p * 4, (p: number) => p * 5]
        arktest.assertEQ(f07FArr(1.0, ...[...a, ...b]), 360.0)
    })

    suite.addTest("spread of enums array", () => {
        let a: ColorFunc[] = [ColorFunc.Red, ColorFunc.Blue, ColorFunc.Green]
        arktest.assertEQ(f08ColorArr(...a), ColorFunc.Green)
    })

    suite.addTest("expression in parentheses", () => {
        arktest.assertEQ(f09ParenNum(...([1.0, 2.0])), 2.0)
    })

    suite.addTest("ensure-not-nullish expression (array OK)", () => {
        let a1: Object[]|null = [new Object, new Object]
        arktest.assertEQ(f10ObjEnsure(5, ...a1!), 7)
    })

    suite.addTest("ensure-not-nullish expression NPE (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {f11ObjNpe(5, ...a1!) == 7}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let a2: Object[]|null = [new Object, new Object]
        arktest.assertEQ(f12NullCoalAny(5, ...(a1 ?? a2)!), 7)
    })

    suite.addTest("function call in expression (number[])", () => {
        arktest.assertEQ(f13bBarNumArr(...f13aFooNumArr()), 3.0)
    })

    suite.addTest("function call in expression (string filter)", () => {
        let s = ['h', '', 'el', '', '', 'l', '', 'o']
        arktest.assertEQ(f14StrJoin(...((s: string[]): string[] => {
            let res: string[] = []
            for (let el of s) if (el.length > 0) res = [...res, el]
            return res
        })(s)), 'hello')
    })

    suite.addTest("new expression in expression", () => {
        arktest.assertEQ(f15NewExpr(...(new number[256](0))), 0)
    })

    suite.addTest("array index expression", () => {
        let a: double[][] = [[2.0, 3.0], []]
        arktest.assertEQ(f16ArrIdx(...a[0]), 5)
    })

    suite.addTest("tuple index expression (array element)", () => {
        let a: [string, double[], string] = ['X', [2.0, 3.0, 4.0], 'Z']
        arktest.assertEQ(f17TupIdx(...a[1]), 9)
    })

    suite.addTest("assignment expression in expression (array)", () => {
        let a1 = ['1', '2', '3']
        let a2 = ['a', 'b']
        arktest.assertEQ(f18AssignStr(...(a1 = a2)), 'a-b')
    })

    suite.addTest("conditional expression in expression (array)", () => {
        let a1 = ['1', '2', '3']
        let a2 = ['a', 'b']
        arktest.assertEQ(f19CondStr(...(a1.length > a2.length ? a1 : a2)), '1-2-3')
    })

    suite.addTest("use this expression (array)", () => {
        let a: F20ThisBool = new F20ThisBool([false, false], [true, true])
        arktest.assertEQ(a.meth(), 46)
    })

    suite.addTest("use super expression (array)", () => {
        let b: F21BoolB = new F21BoolB([false, false], [true, true])
        arktest.assertEQ(b.bar(), 46)
    })

    suite.addTest("spread of arrays", () => {
        let a1: number[][] = [[1.0, 2.0], [], [3.0, 4.0], [5.0]]
        arktest.assertEQ(f22ArrCount(...a1), 5)
    })

    suite.addTest("spread of arrays, flatten array", () => {
        let a1: number[][] = [[1.0, 2.0], [3.0, 4.0]]
        let a2: number[][] = [[5.0, 6.0], []]
        arktest.assertEQ(f23bSum(...[...f23aFlat(...a1), ...f23aFlat(...a2)]), 21.0)
    })

    suite.addTest("union of value types in result array", () => {
        let a1: Double[] = [1, 2, 3], a2: Short[] = [4, 5], a3: Byte[] = [6]
        let ar = [-10.0, ...a1, ...a1, ...a2, ...a2, ...a3, 10.0, ...a3]
        arktest.assertEQ(f24Union(...ar), 42.0)
    })

    suite.addTest("union of reference types in result array", () => {
        let a1: (Int|Number|String|undefined)[] = [new Int(), new Int(), undefined, 'Z', 42.0]
        arktest.assertEQ(f25UnionRef(...a1), 5)
    })

    suite.addTest("use generics (array)", () => {
        let a1: (GenAFunc<String>|GenBFunc<Long>|GenCFunc<Error>|null)[] = [new GenAFunc<String>, new GenBFunc<Long>, new GenCFunc<Error>]
        arktest.assertEQ(f26Generics<String, Long, Error>(...a1), 3)
    })

    suite.addTest("lambda expression in expression (array)", () => {
        let a1 = ['1', '2', '3']
        arktest.assertEQ(f27StrLambda(...((): string[] => a1)()), '123')
    })

    suite.addTest("lambda expression in expression (concat)", () => {
        let a1 = ['1', '2', '3']
        arktest.assertEQ(f27StrLambda(...((p: string[], q: string[]): string[] => [...p, ...q])(a1, a1)), '123123')
    })

    suite.addTest("spread a tuple to same type tuple", () => {
        let t1: [number, boolean] = [1.0, true]
        let r: [boolean, number] = f29TupSwap(...t1)
        arktest.assertTrue(r[0] && r[1] == 1.0)
    })

    suite.addTest("spread tuples to different types", () => {
        let r: [number, number, boolean, boolean] = f30TupTo4(...[1.0, true])
        arktest.assertTrue(r[0] == 1.0 && r[1] == 1.0 && r[2] && r[3])
    })

    suite.addTest("spread tuples of all value types", () => {
        let r: [byte, short, int, long, float, double, boolean, char, string] =
            [(1).toByte(), (2).toShort(), 3, (4).toLong(), 5.0f, 6.0, true, c'Z', 'X']
        arktest.assertEQ(f31AllValTup(...r)[9], 42)
    })

    suite.addTest("ensure-not-nullish expression (tuple OK)", () => {
        let a1: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ(f32ObjErrTup(...a1!), 3)
    })

    suite.addTest("ensure-not-nullish expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {f32ObjErrTup(...a1!) == 3}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ(f32ObjErrTup(...(a1 ?? a2)!), 3)
    })

    suite.addTest("nullish-coalescing expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = nullTupleObjErr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {f32ObjErrTup(...(a1 ?? a2)!) == 3}, expectedException)
    })

    suite.addTest("function call in expression (tuple)", () => {
        let r: [string, number] = f36bBarTup(...f36aFooTup())
        arktest.assertTrue(r[0] == 'X' && r[1] == 1)
    })

    suite.addTest("tuple index expression (nested tuple)", () => {
        let a: [[double, double], [char, char]] = [[2.0, 3.0], [c'Z', c'X']]
        arktest.assertEQ(f37TupDoubleSum(...a[0]), 5.0)
    })

    suite.addTest("assignment expression in expression (tuple)", () => {
        let a1: [string, int] = ['1', 10]
        let a2: [string, int] = ['2', 20]
        arktest.assertEQ(f38TupAssign(...(a1 = a2)), 20)
    })

    suite.addTest("conditional expression in expression (tuple)", () => {
        let a1: [int, int] = [1, 2]
        let a2: [int, int] = [5, 6]
        let r = f39TupCond(...(a1[0] > a2[0] ? a1 : a2))
        arktest.assertEQ(r, 11)
    })

    suite.addTest("use this expression (tuple)", () => {
        let a: F40ThisTup = new F40ThisTup(1, c'Z')
        arktest.assertEQ(a.meth(), 111)
    })

    suite.addTest("use super expression (tuple)", () => {
        let b: F41TupB = new F41TupB(1, c'Z')
        arktest.assertEQ(b.meth()[0], 111)
    })

    suite.addTest("use generics (tuple)", () => {
        let a1: [GenAFunc<Long>, GenBFunc<Error>] = [new GenAFunc<Long>, new GenBFunc<Error>]
        arktest.assertFalse(f42TupGen<Long, Error>(...a1))
    })

    suite.addTest("lambda expression in expression (tuple)", () => {
        let a2: [int, int] = [6, 5]
        let r = f43TupLambda(...((p: [int, int]): [int, int] => [p[1], p[0]])(a2))
        arktest.assertEQ(r, '56')
    })

    suite.addTest("spread of array of records", () => {
        let r1: Record<int, string> = {1: '1', 3: '3'}
        let r2: Record<int, string> = {2: '2'}
        arktest.assertEQ(f44RecArr(...[r1, r2]), 2)
    })

    suite.addTest("FixedArray empty arrays", () => {
        let a1: FixedArray<Object> = [], a2: FixedArray<Error> = []
        arktest.assertEQ(f46FAObjArr(...[...a1, ...a2, new Object(), ...a1]), 1)
    })

    suite.addTest("FixedArray spread of char array", () => {
        let a: FixedArray<char> = [c'b', c'c']
        arktest.assertEQ(f47FACharArr(...a), 2)
    })

    suite.addTest("FixedArray spread of byte array", () => {
        let a: FixedArray<byte> = [(1).toByte(), (2).toByte(), (3).toByte()]
        arktest.assertEQ(f48FAByteArr(42, ...a), 48)
    })

    suite.addTest("FixedArray spread of short array", () => {
        let a: FixedArray<short> = [(1).toShort(), (2).toShort(), (3).toShort()]
        arktest.assertEQ(f49FAShortArr(42, ...a), 48)
    })

    suite.addTest("FixedArray spread of long array", () => {
        let a: FixedArray<long> = [(1).toLong(), (2).toLong(), (3).toLong()]
        arktest.assertEQ(f50FALongArr((42).toLong(), ...a), 48)
    })

    suite.addTest("FixedArray spread of float array", () => {
        let a: FixedArray<float> = [1.0f, 2.0f, 3.0f]
        arktest.assertEQ(f51FAFloatArr(42.0f, ...a), 48.0f)
    })

    suite.addTest("FixedArray spread of number array", () => {
        let a: FixedArray<number> = [1.0, 2.0, 3.0]
        arktest.assertEQ(f52FANumArr(42, ...a), 48.0)
    })

    suite.addTest("FixedArray spread of enums array", () => {
        let a: FixedArray<ColorFunc> = [ColorFunc.Red, ColorFunc.Blue, ColorFunc.Green]
        arktest.assertEQ(f54FAColorArr(...a), ColorFunc.Green)
    })

    suite.addTest("FixedArray expression in parentheses", () => {
        arktest.assertEQ(f55FAParen(...([1.0, 2.0])), 2.0)
    })

    suite.addTest("FixedArray ensure-not-nullish expression OK", () => {
        let a1: FixedArray<Object>|null = [new Object, new Object]
        arktest.assertEQ(f56FAEnsure(5, ...a1!), 7)
    })

    suite.addTest("FixedArray ensure-not-nullish expression NPE", () => {
        let a1: FixedArray<Object>|null = nullFAObj()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {f57FANpe(5, ...a1!) == 7}, expectedException)
    })

    suite.addTest("FixedArray nullish-coalescing expression", () => {
        let a1: FixedArray<Object>|null = nullFAObj()
        let a2: FixedArray<Object>|null = [new Object, new Object]
        arktest.assertEQ(f58FANullCoal(5, ...(a1 ?? a2)!), 7)
    })

    suite.addTest("FixedArray function call in expression", () => {
        arktest.assertEQ(f59bFABarNum(...f59aFAFooNum()), 3.0)
    })

    suite.addTest("FixedArray filter through function", () => {
        let s: FixedArray<int> = [1, -1, 2, -2, -3, 3, -4, 4]
        arktest.assertEQ(f60FAFilterInt(...((s: FixedArray<int>): FixedArray<int> => {
            let res: FixedArray<int> = []
            for (let el of s) if (el > 0) res = [...res, el]
            return res
        })(s)), 10)
    })

    suite.addTest("FixedArray new expression in expression", () => {
        arktest.assertEQ(f61FANewExpr(...new FixedArray<number>(256)), 0)
    })

    suite.addTest("FixedArray array index expression", () => {
        let a: FixedArray<FixedArray<double>> = [[2.0, 3.0], []]
        arktest.assertEQ(f62FAArrIdx(...a[0]), 5)
    })

    suite.addTest("FixedArray tuple index expression", () => {
        let a: [string, FixedArray<double>, string] = ['X', [2.0, 3.0, 4.0], 'Z']
        arktest.assertEQ(f63FATupIdx(...a[1]), 9)
    })

    suite.addTest("FixedArray assignment expression", () => {
        let a1: FixedArray<long> = [1, 2, 3]
        let a2: FixedArray<long> = [4, 5]
        arktest.assertEQ(f64FAAssignLong(...(a1 = a2)), '45')
    })

    suite.addTest("FixedArray conditional expression", () => {
        let a1: FixedArray<byte> = [1, 2, 3]
        let a2: FixedArray<byte> = [4, 5]
        arktest.assertEQ(f65FACondByte(...(a1.length > a2.length ? a1 : a2)), '123')
    })

    suite.addTest("FixedArray use this expression", () => {
        let a: F69ThisFABool = new F69ThisFABool([false, false], [true, true])
        arktest.assertEQ(a.meth(), 46)
    })

    suite.addTest("FixedArray use super expression", () => {
        let b: F70FAB = new F70FAB([false, false], [true, true])
        arktest.assertEQ(b.bar(), 46)
    })

    suite.addTest("FixedArray spread of arrays", () => {
        let a1: FixedArray<FixedArray<number>> = [[1.0, 2.0], [], [3.0, 4.0], [5.0]]
        arktest.assertEQ(f71FAArrCount(...a1), 5)
    })

    suite.addTest("FixedArray spread of arrays, flatten", () => {
        let a1: FixedArray<FixedArray<number>> = [[1.0, 2.0], [3.0, 4.0]]
        let a2: FixedArray<FixedArray<number>> = [[5.0, 6.0], []]
        arktest.assertEQ(f72bFASum(...[...f72aFAFlat(...a1), ...f72aFAFlat(...a2)]), 21.0)
    })

    suite.addTest("FixedArray union of value types", () => {
        let a1: FixedArray<Double> = [1, 2, 3], a2: FixedArray<Short> = [4, 5], a3: FixedArray<Byte> = [6]
        let ar = [-10.0, ...a1, ...a1, ...a2, ...a2, ...a3, 10.0, ...a3]
        arktest.assertEQ(f73FAUnion(...ar), 42.0)
    })

    suite.addTest("FixedArray union of reference types", () => {
        let a1: FixedArray<Int|Number|String|undefined> = [new Int(), new Int(), undefined, 'Z', 42.0]
        arktest.assertEQ(f74FAUnionRef(...a1), 5)
    })

    suite.addTest("FixedArray lambda expression", () => {
        let a1: FixedArray<string> = ['1', '2', '3']
        arktest.assertEQ(f76FAStrLambda(...((): FixedArray<string> => a1)()), '123')
    })

    suite.addTest("FixedArray lambda expression (concat)", () => {
        let a1: FixedArray<string> = ['1', '2', '3']
        arktest.assertEQ(f77FAStrLambda2(...((p: FixedArray<string>, q: FixedArray<string>): Array<string> => [...p, ...q])(a1, a1)), '123123')
    })

    return suite.run()
}
