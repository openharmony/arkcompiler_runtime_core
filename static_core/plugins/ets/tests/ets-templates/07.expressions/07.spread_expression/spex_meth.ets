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

type FMeth = (p: number) => number

enum ColorMeth { Red, Green, Blue }

class M01ObjArr {
    foo(...p: Object[]): number {
        return p.length
    }
}

class M02CharArr {
    foo(...p: char[]): number {
        return p.length
    }
}

class M03ByteArr {
    static foo(a: int, ...p: byte[]): int {
        return a + reduce(p, (prev: int, next: byte) => prev + next, 0)
    }
}

class M04ShortArr {
    foo(a: int, ...p: short[]): int {
        return a + reduce(p, (prev: int, next: short) => prev + next, 0)
    }
}

class M05LongArr {
    static foo(a: long, ...p: long[]): long {
        return a + reduce(p, (prev: long, next: long) => prev + next, (0).toLong())
    }
}

class M06FloatArr {
    foo(a: float, ...p: float[]): float {
        return a + reduce(p, (prev: float, next: float) => prev + next, 0.0f)
    }
}

class M07NumberArr {
    static foo(a: number, ...p: number[]): number {
        return a + reduce(p, (prev: number, next: number) => prev + next, 0.0)
    }
}

class M08FArr {
    foo(a: number, ...p: FMeth[]): number {
        let res = a
        for (let el of p) res += el(res)
        return res
    }
}

class M09ColorArr {
    foo(...p: ColorMeth[]): ColorMeth {
        return p.length ? p[p.length - 1] : ColorMeth.Red
    }
}

class M10ParenNum {
    static foo(...p: number[]): number {
        return p.length ? p[p.length - 1] : -1.0
    }
}

abstract class AbsM11ObjArr {
    foo(a: int, ...p: Object[]): number {
        return a + p.length
    }
}

class BM11 extends AbsM11ObjArr {}

interface IM12ObjArr {
    foo(a: int, ...p: Object[]): number {
        return a + p.length
    }
}

class AM12 implements IM12ObjArr {}

class M13NullCoal {
    foo(a: int, ...p: Object[]): number {
        return a + p.length
    }
}

class M14FooBar {
    foo(): number[] {
        return [1.0, 2.0]
    }
    bar(...p: number[]): number {
        return reduce(p, (prev: number, next: number) => prev + next, 0.0)
    }
}

interface IM15Str {
    foo(...p: string[]): string {
        return String.join(p, "")
    }
}

class AM15 implements IM15Str {}

class M16NewExpr {
    foo(...p: number[]): number {
        return p.length ? p[p.length - 1] : -1
    }
}

class M17ArrIdx {
    static foo(...p: double[]): double {
        return p[0] + p[1]
    }
}

class M18TupIdx {
    foo(...p: double[]): double {
        return p[0] + p[1] + p[2]
    }
}

class M19AssignStr {
    foo(...p: string[]): string {
        return String.join(p, "-")
    }
}

class BM19 extends M19AssignStr {}

class M20CondStr {
    foo(...p: string[]): string {
        return String.join(p, "-")
    }
}

class M21ThisBool {
    fld: boolean[]
    constructor(p: boolean[], q: boolean[]) {
        this.fld = [...p, ...q]
    }
    meth(): number {
        return BM21Static.foo(42, ...this.fld)
    }
}

class BM21Static {
    static foo(a: int, ...p: boolean[]): number {
        return a + p.length
    }
}

abstract class AbsM22Bool {
    fld: boolean[]
    constructor(p: boolean[]) {
        this.fld = p
    }
    meth(): boolean[] {
        return this.fld
    }
}

class CM22 {
    foo(a: int, ...p: boolean[]): number {
        return a + p.length
    }
}

class BM22 extends AbsM22Bool {
    obj: CM22
    constructor(p: boolean[], q: boolean[], obj: CM22) {
        super([...p, ...q])
        this.obj = obj
    }
    bar(): number {
        return this.obj.foo(42, ...super.meth())
    }
}

class M23ArrCount {
    foo(...p: number[][]): number {
        let res = 0
        for (let el of p) res += el.length
        return res
    }
}

class M24Flat {
    foo(...p: number[][]): number[] {
        let res: number[] = []
        for (let el of p) res = [...res, ...el]
        return res
    }
    bar(...p: number[]): number {
        let res = 0.0
        for (let el of p) res += el
        return res
    }
}

class M25Union {
    foo(...p: (Double|Short|Byte)[]): double {
        let res = 0.0
        for (let el of p) res += (el as Numeric).toDouble()
        return res
    }
}

class M26UnionRef {
    foo(...p: (Int|Number|String|undefined)[]): number {
        return p.length
    }
}

class GenAMeth<T> {}
class GenBMeth<T> {}
class GenCMeth<T> {}

class ZM27<T, U, V> {
    foo(...p: (GenAMeth<T>|GenBMeth<U>|GenCMeth<V>|null)[]): number {
        return p.length
    }
}

class M28StrLambda {
    foo(...p: string[]): string {
        return String.join(p, "")
    }
}

class M30TupSwap {
    foo(...p: [number, boolean]): [boolean, number] {
        return [p[1], p[0]]
    }
}

class M31TupTo4 {
    foo(...p: [number, boolean]): [number, number, boolean, boolean] {
        return [p[0], p[0], p[1], p[1]]
    }
}

class M32AllValTup {
    foo(...p: [byte, short, int, long, float, double, boolean, char, string]): [byte, short, int, long, float, double, boolean, char, string, int] {
        return [...p, 42]
    }
}

class M33ObjErrTup {
    static foo(...p: [Object, Error]): int {
        return p[0] != p[1] ? 3 : 0
    }
}

class M34ObjErrTupNpe {
    foo(...p: [Object, Error]): int {
        return p[0] != p[1] ? 3 : 0
    }
}

class M35ObjErrTupCoal {
    foo(...p: [Object, Error]): int {
        return p[0] != p[1] ? 3 : 0
    }
}

class M36ObjErrTupCoalNpe {
    foo(...p: [Object, Error]): int {
        return p[0] != p[1] ? 3 : 0
    }
}

interface IM37Tup {
    foo(): [number, string] {
        return [1.0, "X"]
    }
}

class AM37 implements IM37Tup {
    bar(...p: [number, string]): [string, number] {
        return [p[1], p[0]]
    }
}

class M38TupDoubleSum {
    foo(...p: [double, double]): double {
        return p[0] + p[1]
    }
}

class M39TupAssign {
    static foo(...p: [string, int]): int {
        return p[1]
    }
}

class M40TupCond {
    foo(...p: [int, int]): int {
        return p[0] + p[1]
    }
}

class ZM41 {
    foo(...p: [int, char]): int {
        return 100 + p[0]
    }
}

class M41ThisTup {
    fld: [int, char]
    obj: ZM41
    constructor(p: int, q: char, obj: ZM41) {
        this.fld = [p, q]
        this.obj = obj
    }
    meth(): int {
        return 10 + this.obj.foo(...this.fld)
    }
}

abstract class AbsM42Tup {
    fld: [int, char]
    constructor(p: [int, char]) {
        this.fld = p
    }
    meth(): [int, char] {
        return this.fld
    }
}

class ZM42 {
    static foo(...p: [int, char]): int {
        return 100 + p[0]
    }
}

class BM42 extends AbsM42Tup {
    constructor(p: int, q: char) {
        super([p, q])
    }
    meth(): [int, char] {
        this.fld[0] = 10 + ZM42.foo(...super.meth())
        return this.fld
    }
}

class ZM43Gen<T, U> {
    foo(...p: [GenAMeth<T>, GenBMeth<U>]): boolean {
        return p[0] == p[1]
    }
}

class M44TupLambda {
    foo(...p: [int, int]): string {
        return "" + p[0] + p[1]
    }
}

interface IfaceMapA {}
interface IfaceMapTest1 {
    t: IfaceMapA[]
}
interface IfaceMapB1 {
    unsorted: IfaceMapA[]
}
class ClassMapC1 {
    static foo(...arg: IfaceMapB1[]): void {
        arktest.assertEQ(arg.length, 3)
    }
}
class ClassMapC2 {
    static foo(...arg: IfaceMapB1[]): void {
        arktest.assertEQ(arg.length, 3)
        arktest.assertEQ(arg[arg.length - 1].unsorted.length, 6)
    }
}

interface IfaceMapA2 {
    cnt?: int
}
interface IfaceMapTest2 {
    t: IfaceMapA2[]
}
interface IfaceMapB2 {
    unsorted: IfaceMapA2[]
}
class ClassMapC3 {
    static foo(...arg: IfaceMapB2[]): void {
        arktest.assertEQ(arg.length, 3)
        arktest.assertEQ(7, reduce(arg,
            (p: int, c: IfaceMapB2) => p + reduce(c.unsorted,
                (p: int, c: IfaceMapA2) => p + (c.cnt ? c.cnt! : 0), 0), 0
            )
        )
    }
}

function nullObjArr(): Object[]|null {
    return null
}

function nullTupleObjErr(): [Object, Error]|null {
    return null
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression in method calls")

    suite.addTest("empty arrays", () => {
        let a1: Object[] = [], a2: Error[] = []
        arktest.assertEQ(new M01ObjArr().foo(...[...a1, ...a2, new Object(), ...a1]), 1)
    })

    suite.addTest("spread of char array", () => {
        let a: char[] = [c'b', c'c']
        arktest.assertEQ(new M02CharArr().foo(...a), 2)
    })

    suite.addTest("spread of byte array", () => {
        let a: byte[] = [(1).toByte(), (2).toByte(), (3).toByte()]
        arktest.assertEQ(M03ByteArr.foo(42, ...a), 48)
    })

    suite.addTest("spread of short array", () => {
        let a: short[] = [(1).toShort(), (2).toShort(), (3).toShort()]
        arktest.assertEQ(new M04ShortArr().foo(42, ...a), 48)
    })

    suite.addTest("spread of long array", () => {
        let a: long[] = [(1).toLong(), (2).toLong(), (3).toLong()]
        arktest.assertEQ(M05LongArr.foo((42).toLong(), ...a), 48)
    })

    suite.addTest("spread of float array", () => {
        let a: float[] = [1.0f, 2.0f, 3.0f]
        arktest.assertEQ(new M06FloatArr().foo(42.0f, ...a), 48.0f)
    })

    suite.addTest("spread of number array", () => {
        let a: number[] = [1.0, 2.0, 3.0]
        arktest.assertEQ(M07NumberArr.foo(42, ...a), 48.0)
    })

    suite.addTest("spread of function array", () => {
        let a: FMeth[] = [(p: number) => p * 2, (p: number) => p * 3]
        let b: FMeth[] = [(p: number) => p * 4, (p: number) => p * 5]
        let obj = new M08FArr
        arktest.assertEQ(obj.foo(1.0, ...[...a, ...b]), 360.0)
    })

    suite.addTest("spread of enums", () => {
        let a: ColorMeth[] = [ColorMeth.Red, ColorMeth.Blue, ColorMeth.Green]
        let obj = new M09ColorArr
        arktest.assertEQ(obj.foo(...a), ColorMeth.Green)
    })

    suite.addTest("expression in parentheses", () => {
        arktest.assertEQ(M10ParenNum.foo(...([1.0, 2.0])), 2.0)
    })

    suite.addTest("ensure-not-nullish expression (array OK)", () => {
        let a1: Object[]|null = [new Object, new Object]
        let obj = new BM11
        arktest.assertEQ(obj.foo(5, ...a1!), 7)
    })

    suite.addTest("ensure-not-nullish expression NPE (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        let obj = new AM12
        arktest.expectThrow(() => {obj.foo(5, ...a1!) == 7}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (array)", () => {
        let a1: Object[]|null = nullObjArr()
        let a2: Object[]|null = [new Object, new Object]
        arktest.assertEQ(new M13NullCoal().foo(5, ...(a1 ?? a2)!), 7)
    })

    suite.addTest("method call in expression", () => {
        let obj = new M14FooBar
        arktest.assertEQ(obj.bar(...obj.foo()), 3.0)
    })

    suite.addTest("method call in expression (str_filter)", () => {
        let obj = new AM15
        let s = ["h", "", "el", "", "", "l", "", "o"]
        arktest.assertEQ(obj.foo(...((s: string[]): string[] => {
            let res: string[] = []
            for (let el of s) if (el.length > 0) res = [...res, el]
            return res
        })(s)), "hello")
    })

    suite.addTest("new expression in expression", () => {
        arktest.assertEQ((new M16NewExpr).foo(...(new number[256](0))), 0)
    })

    suite.addTest("array index expression", () => {
        let a: double[][] = [[2.0, 3.0], []]
        arktest.assertEQ(M17ArrIdx.foo(...a[0]), 5)
    })

    suite.addTest("tuple index expression (array element)", () => {
        let a: [string, double[], string] = ["X", [2.0, 3.0, 4.0], "Z"]
        let obj = new M18TupIdx()
        arktest.assertEQ(obj.foo(...a[1]), 9)
    })

    suite.addTest("assignment expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        let obj = new BM19()
        arktest.assertEQ((obj).foo(...(a1 = a2)), "a-b")
    })

    suite.addTest("conditional expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        let a2 = ["a", "b"]
        arktest.assertEQ(new M20CondStr().foo(...(a1.length > a2.length ? a1 : a2)), "1-2-3")
    })

    suite.addTest("use this expression (array)", () => {
        let a: M21ThisBool = new M21ThisBool([false, false], [true, true])
        arktest.assertEQ(a.meth(), 46)
    })

    suite.addTest("use super expression (array)", () => {
        let b: BM22 = new BM22([false, false], [true, true], new CM22)
        arktest.assertEQ(b.bar(), 46)
    })

    suite.addTest("spread of arrays", () => {
        let a1: number[][] = [[1.0, 2.0], [], [3.0, 4.0], [5.0]]
        arktest.assertEQ((new M23ArrCount).foo(...a1), 5)
    })

    suite.addTest("spread of arrays, flatten array", () => {
        let a1: number[][] = [[1.0, 2.0], [3.0, 4.0]]
        let a2: number[][] = [[5.0, 6.0], []]
        let obj = new M24Flat()
        arktest.assertEQ(obj.bar(...[...obj.foo(...a1), ...obj.foo(...a2)]), 21.0)
    })

    suite.addTest("union of value types in result array", () => {
        let a1: Double[] = [1, 2, 3], a2: Short[] = [4, 5], a3: Byte[] = [6]
        let ar = [-10.0, ...a1, ...a1, ...a2, ...a2, ...a3, 10.0, ...a3]
        let obj = new M25Union
        arktest.assertEQ(obj.foo(...ar), 42.0)
    })

    suite.addTest("union of reference types in result array", () => {
        let a1: (Int|Number|String|undefined)[] = [new Int(), new Int(), undefined, "Z", 42.0]
        arktest.assertEQ(new M26UnionRef().foo(...a1), 5)
    })

    suite.addTest("use generics (array)", () => {
        let a1: (GenAMeth<String>|GenBMeth<Long>|GenCMeth<Error>|null)[] = [new GenAMeth<String>, new GenBMeth<Long>, new GenCMeth<Error>]
        let obj = new ZM27<String, Long, Error>()
        arktest.assertEQ(obj.foo(...a1), 3)
    })

    suite.addTest("lambda expression in expression (array)", () => {
        let a1 = ["1", "2", "3"]
        arktest.assertEQ(new M28StrLambda().foo(...((): string[] => a1)()), "123")
    })

    suite.addTest("lambda expression in expression (concat)", () => {
        let a1 = ["1", "2", "3"]
        arktest.assertEQ(new M28StrLambda().foo(...((p: string[], q: string[]): string[] => [...p, ...q])(a1, a1)), "123123")
    })

    suite.addTest("spread a tuple to same type tuple", () => {
        let obj = new M30TupSwap()
        let t1: [number, boolean] = [1.0, true]
        let r: [boolean, number] = obj.foo(...t1)
        arktest.assertTrue(r[0] && r[1] == 1.0)
    })

    suite.addTest("spread tuples to different types", () => {
        let obj = new M31TupTo4()
        let r: [number, number, boolean, boolean] = obj.foo(...[1.0, true])
        arktest.assertTrue(r[0] == 1.0 && r[1] == 1.0 && r[2] && r[3])
    })

    suite.addTest("spread tuples of all value types", () => {
        let r: [byte, short, int, long, float, double, boolean, char, string] =
            [(1).toByte(), (2).toShort(), 3, (4).toLong(), 5.0f, 6.0, true, c'Z', "X"]
        arktest.assertEQ(new M32AllValTup().foo(...r)[9], 42)
    })

    suite.addTest("ensure-not-nullish expression (tuple OK)", () => {
        let a1: [Object, Error]|null = [new Object, new Error]
        arktest.assertEQ(M33ObjErrTup.foo(...a1!), 3)
    })

    suite.addTest("ensure-not-nullish expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let obj = new M34ObjErrTupNpe
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {obj.foo(...a1!) == 3}, expectedException)
    })

    suite.addTest("nullish-coalescing expression (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = [new Object, new Error]
        let obj = new M35ObjErrTupCoal
        arktest.assertEQ(obj.foo(...(a1 ?? a2)!), 3)
    })

    suite.addTest("nullish-coalescing expression NPE (tuple)", () => {
        let a1: [Object, Error]|null = nullTupleObjErr()
        let a2: [Object, Error]|null = nullTupleObjErr()
        let obj = new M36ObjErrTupCoalNpe()
        let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
        arktest.expectThrow(() => {obj.foo(...(a1 ?? a2)!) == 3}, expectedException)
    })

    suite.addTest("function call in expression (tuple)", () => {
        let obj = new AM37()
        let r: [string, number] = obj.bar(...obj.foo())
        arktest.assertTrue(r[0] == "X" && r[1] == 1)
    })

    suite.addTest("tuple index expression (nested tuple)", () => {
        let a: [[double, double], [char, char]] = [[2.0, 3.0], [c'Z', c'X']]
        let obj = new M38TupDoubleSum
        arktest.assertEQ(obj.foo(...a[0]), 5.0)
    })

    suite.addTest("assignment expression in expression (tuple)", () => {
        let a1: [string, int] = ["1", 10]
        let a2: [string, int] = ["2", 20]
        arktest.assertEQ(M39TupAssign.foo(...(a1 = a2)), 20)
    })

    suite.addTest("conditional expression in expression (tuple)", () => {
        let a1: [int, int] = [1, 2]
        let a2: [int, int] = [5, 6]
        let r = new M40TupCond().foo(...(a1[0] > a2[0] ? a1 : a2))
        arktest.assertEQ(r, 11)
    })

    suite.addTest("use this expression (tuple)", () => {
        let a: M41ThisTup = new M41ThisTup(1, c'Z', new ZM41)
        arktest.assertEQ(a.meth(), 111)
    })

    suite.addTest("use super expression (tuple)", () => {
        let b: BM42 = new BM42(1, c'Z')
        arktest.assertEQ(b.meth()[0], 111)
    })

    suite.addTest("use generics (tuple)", () => {
        let obj = new ZM43Gen<Long, Error>
        let a1: [GenAMeth<Long>, GenBMeth<Error>] = [new GenAMeth<Long>, new GenBMeth<Error>]
        arktest.assertFalse(obj.foo(...a1))
    })

    suite.addTest("lambda expression in expression (tuple)", () => {
        let a2: [int, int] = [6, 5]
        let r = new M44TupLambda().foo(...((p: [int, int]): [int, int] => [p[1], p[0]])(a2))
        arktest.assertEQ(r, "56")
    })

    suite.addTest("spread of map function (basic)", () => {
        const tests: IfaceMapTest1[] = [{ t: [{}, {}] }, { t: [{}] }, { t: [{}, {}, {}] }]
        ClassMapC1.foo(
            ...tests.map(
                (test): IfaceMapB1 => ({
                    unsorted: [...test.t.map((m): IfaceMapA => ({}))],
                })
            )
        )
    })

    suite.addTest("spread of map function (concat in map)", () => {
        const tests: IfaceMapTest1[] = [{ t: [{}, {}] }, { t: [{}] }, { t: [{}, {}, {}] }]
        ClassMapC2.foo(
            ...tests.map(
                (test): IfaceMapB1 => ({
                    unsorted: [...test.t.map((m): IfaceMapA => ({})), ...test.t.map((m): IfaceMapA => ({}))],
                })
            )
        )
    })

    suite.addTest("spread of map function (with index field)", () => {
        const tests: IfaceMapTest2[] = [{ t: [{}, {}, {}, {}] }, { t: [{}] }, { t: [{}, {}] }]
        ClassMapC3.foo(
            ...tests.map(
                (test): IfaceMapB2 => ({
                    unsorted: [...test.t.map((m, i): IfaceMapA2 => ({ cnt: i }))],
                })
            )
        )
    })

    return suite.run()
}
