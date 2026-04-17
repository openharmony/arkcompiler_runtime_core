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
  The type of an array literal can be inferred from the declaration context
---*/

function bar(i: Int): Char {
    let v: Int = i + 1
    return v.toChar()
}

class PlainClass {}

class GenericClass<T> {}

enum Color { Red, Green, Blue }

enum Size { S, M, L, XL }

type F = (p: Color) => int

function main(): int {
    let suite = new arktest.ArkTestsuite("arr_inf_decl")

    suite.addTest("boolean[] from literal", () => {
        let a: boolean[] = [!true, true, true]
        arktest.assertEQ(a[0], false)
        arktest.assertEQ(a[1], true)
        arktest.assertEQ(a[2], true)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("long[] from literal", () => {
        let a: long[] = [1, 2, 3, -9223372036854775807 - 1]
        arktest.assertEQ(a[3], -9223372036854775808)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("int[][] nested", () => {
        let a: int[][] = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("number[] from literal", () => {
        let a: number[] = [Number.NaN, 0, 3.14, 1.7976931348623157e+308, ]
        arktest.assertTrue(Number.isNaN(a[0]))
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Boolean[] from literal", () => {
        let a: Boolean[] = [false, true, true]
        arktest.assertEQ(a[0], false)
        arktest.assertEQ(a[1], true)
        arktest.assertEQ(a[2], true)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Long[] from literal", () => {
        let a: Long[] = [1, 2, 3, -9223372036854775808,]
        arktest.assertEQ(a[3], -9223372036854775808)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Number[] from literal", () => {
        let a: Number[] = [Number.NaN, 0, 3.14, 1.7976931348623157e+308, ]
        arktest.assertTrue(Number.isNaN(a[0]))
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Number[][] nested", () => {
        let a: Number[][] = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("string[] from literal", () => {
        let a: string[] = ['1', '2', '', '-9223372036854775808', ]
        arktest.assertEQ(a[3], '-9223372036854775808')
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("string[] with concatenation", () => {
        let a: string[] = ['x', 'x' + '1', 'x' + '2', 'x' + '3', ]
        arktest.assertEQ(a[0], 'x')
        arktest.assertEQ(a[1], 'x1')
        arktest.assertEQ(a[2], 'x2')
        arktest.assertEQ(a[3], 'x3')
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Char[] from function", () => {
        let a: Char[] = [bar(42), bar(43), bar(44), ]
        arktest.assertEQ(a[0], c'+')
        arktest.assertEQ(a[1], c',')
        arktest.assertEQ(a[2], c'-')
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("String[][] nested", () => {
        let s = '1'
        let a: String[][] = [[s, s + s,], ['', '-9223372036854775808',], ]
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("bigint[] from literal", () => {
        let a: bigint[] = [1n, 2n, 33333333333333333333333333333333333333333333333333n]
        arktest.assertEQ(a[1], 2n)
        arktest.assertEQ(a[2], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("bigint[][] nested", () => {
        let a: bigint[][] = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]]
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("array of PlainClass", () => {
        let a: PlainClass[] = [{}]
        arktest.assertTrue(a[0] instanceof PlainClass)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("array of GenericClass<Number>", () => {
        let v = new GenericClass<Number>
        let q = new GenericClass<Number>
        let a: GenericClass<Number>[] = [v, q]
        arktest.assertEQ(a[0], v)
        arktest.assertEQ(a[1], q)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("Object[][] mixed types", () => {
        let a: Object[][] = [["a", 1, true, 2n]]
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("array of enums", () => {
        let a: Color[] = [Color.Red, Color.Blue]
        arktest.assertEQ(a[0], Color.Red)
        arktest.assertEQ(a[1], Color.Blue)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("array of array of enums", () => {
        let a: Color[][] = [[Color.Red, Color.Blue]]
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("array of functions", () => {
        let a: F[] = [(p: Color): int => p.valueOf(), (p: Color): int => p.valueOf() + 1, (p: Color): int => p.valueOf() + 2]
        arktest.assertEQ(a[0](Color.Red), 0)
        arktest.assertEQ(a[1](Color.Red), 1)
        arktest.assertEQ(a[2](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("array of array of functions", () => {
        let a: F[][] = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]]
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("array of unions with null and undefined", () => {
        let u: undefined = undefined
        let n: null = null
        let o: Object = {}
        let a: (null|undefined|Object)[] = [n, o, o, u]
        arktest.assertEQ(a[0], null)
        arktest.assertEQ(a[1] as Object, o)
        arktest.assertEQ(a[2] as Object, o)
        arktest.assertEQ(a[3], undefined)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("array of unions with primitives", () => {
        let a: (boolean|int|string|null)[] = [true, 1, 'a', null]
        arktest.assertEQ(a[0] as boolean, true)
        arktest.assertEQ(a[1] as int, 1)
        arktest.assertEQ(a[2] as string, 'a')
        arktest.assertEQ(a[3], null)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("array of array of unions with primitives", () => {
        let a: (boolean|int|string|null)[][] = [[true, 1, 'a', null]]
        arktest.assertEQ(a[0][0] as boolean, true)
        arktest.assertEQ(a[0][1] as int, 1)
        arktest.assertEQ(a[0][2] as string, 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("array of union with value types", () => {
        let e = new Error
        let a: (Number|Error|((p: string)=>string)|Boolean)[] = [e, false, (p: string): string => p + p, 42]
        arktest.assertEQ(a[0] as Error, e)
        arktest.assertEQ(a[1] as Boolean, false)
        arktest.assertEQ((a[2] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("array of array of union with value types", () => {
        let e = new Error
        let a: (Number|Error|((p: string)=>string)|Boolean)[][] = [[e], [false], [(p: string): string => p + p], [42]]
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("union of arrays, booleans", () => {
        let a: Number[]|String[]|Boolean[] = [false, !false, false]
        arktest.assertEQ((a as Boolean[])[0], false)
        arktest.assertEQ((a as Boolean[])[1], true)
        arktest.assertEQ((a as Boolean[])[2], false)
        arktest.assertEQ((a as Boolean[]).length, 3)
    })

    suite.addTest("union of arrays, numbers", () => {
        let a: Number[]|String[]|Boolean[] = [1.0, 2.0, 43.0]
        arktest.assertEQ((a as Number[])[0], 1.0)
        arktest.assertEQ((a as Number[])[1], 2.0)
        arktest.assertEQ((a as Number[])[2], 43.0)
        arktest.assertEQ((a as Number[]).length, 3)
    })

    suite.addTest("union of arrays, strings", () => {
        let s = '4'
        let a: Number[]|String[]|Boolean[] = ['1', '2', s + '3']
        arktest.assertEQ((a as string[])[0], '1')
        arktest.assertEQ((a as string[])[1], '2')
        arktest.assertEQ((a as string[])[2], '43')
        arktest.assertEQ((a as string[]).length, 3)
    })

    suite.addTest("array of union of enums", () => {
        let a: (Color|Size)[] = [Color.Red, Color.Green, Size.XL]
        arktest.assertEQ(a[0] as Color, Color.Red)
        arktest.assertEQ(a[1] as Color, Color.Green)
        arktest.assertEQ(a[2] as Size, Size.XL)
        arktest.assertEQ(a.length, 3)
    })

    return suite.run()
}
