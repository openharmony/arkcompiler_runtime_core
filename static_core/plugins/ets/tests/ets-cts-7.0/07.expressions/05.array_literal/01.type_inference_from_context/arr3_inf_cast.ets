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
desc: The type of an array literal can be inferred from the casting context.
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

type UnionArrayType = FixedArray<Number>|FixedArray<String>|FixedArray<Boolean>

function main(): int {
    let suite = new arktest.ArkTestsuite("arr3_inf_cast")

    suite.addTest("FixedArray<boolean>", () => {
        let a = [!true, true, true] as FixedArray<boolean>
        arktest.assertEQ(a[0], false)
        arktest.assertEQ(a[1], true)
        arktest.assertEQ(a[2], true)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<long>", () => {
        let a = [1, 2, 3, -9223372036854775807 - 1] as FixedArray<long>
        arktest.assertEQ(a[3], -9223372036854775808)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<FixedArray<int>>", () => {
        let a = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ] as FixedArray<FixedArray<int>>
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("FixedArray<number>", () => {
        let a = [Number.NaN, 0, 3.14, 1.7976931348623157e+308, ] as FixedArray<number>
        arktest.assertTrue(Number.isNaN(a[0]))
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<Boolean>", () => {
        let a = [false, true, true] as FixedArray<Boolean>
        arktest.assertEQ(a[0], false)
        arktest.assertEQ(a[1], true)
        arktest.assertEQ(a[2], true)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<Long>", () => {
        let a = [1, 2, 3, -9223372036854775808,] as FixedArray<Long>
        arktest.assertEQ(a[3], -9223372036854775808)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<Number>", () => {
        let a = [Number.NaN, 0, 3.14, 1.7976931348623157e+308, ] as FixedArray<Number>
        arktest.assertTrue(Number.isNaN(a[0]))
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<FixedArray<Number>>", () => {
        let a = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ] as FixedArray<FixedArray<Number>>
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("FixedArray<string>", () => {
        let a = ['1', '2', '', '-9223372036854775808', ] as FixedArray<string>
        arktest.assertEQ(a[3], '-9223372036854775808')
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<string>, concatenated", () => {
        let a = ['x', 'x' + '1', 'x' + '2', 'x' + '3', ] as FixedArray<string>
        arktest.assertEQ(a[0], 'x')
        arktest.assertEQ(a[1], 'x1')
        arktest.assertEQ(a[2], 'x2')
        arktest.assertEQ(a[3], 'x3')
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<Char>", () => {
        let a = [bar(42), bar(43), bar(44), ] as FixedArray<Char>
        arktest.assertEQ(a[0], c'+')
        arktest.assertEQ(a[1], c',')
        arktest.assertEQ(a[2], c'-')
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<FixedArray<String>>", () => {
        let s = '1'
        let a = [[s, s + s,], ['', '-9223372036854775808',], ] as FixedArray<FixedArray<String>>
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("FixedArray<bigint>", () => {
        let a = [1n, 2n, 33333333333333333333333333333333333333333333333333n] as FixedArray<bigint>
        arktest.assertEQ(a[1], 2n)
        arktest.assertEQ(a[2], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<FixedArray<bigint>>", () => {
        let a = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]] as FixedArray<FixedArray<bigint>>
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<PlainClass>", () => {
        let a = [{}] as FixedArray<PlainClass>
        arktest.assertTrue(a[0] instanceof PlainClass)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<GenericClass<Number>>", () => {
        let v = new GenericClass<Number>
        let q = new GenericClass<Number>
        let a = [v, q] as Array<GenericClass<Number>>
        arktest.assertEQ(a[0], v)
        arktest.assertEQ(a[1], q)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("FixedArray<FixedArray<Object>>", () => {
        let a = [["a", 1, true, 2n]] as FixedArray<FixedArray<Object>>
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Color>", () => {
        let a = [Color.Red, Color.Blue] as FixedArray<Color>
        arktest.assertEQ(a[0], Color.Red)
        arktest.assertEQ(a[1], Color.Blue)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("FixedArray<FixedArray<Color>>", () => {
        let a = [[Color.Red, Color.Blue]] as FixedArray<FixedArray<Color>>
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<F>", () => {
        let a = [(p: Color): int => p.valueOf(), (p: Color): int => p.valueOf() + 1, (p: Color): int => p.valueOf() + 2] as Array<F>
        arktest.assertEQ(a[0](Color.Red), 0)
        arktest.assertEQ(a[1](Color.Red), 1)
        arktest.assertEQ(a[2](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<Array<F>>", () => {
        let a = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]] as Array<Array<F>>
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<null|undefined|Object>", () => {
        let u: undefined = undefined
        let n: null = null
        let o: Object = {}
        let a = [n, o, o, u] as FixedArray<null|undefined|Object>
        arktest.assertEQ(a[0], null)
        arktest.assertEQ(a[1] as Object, o)
        arktest.assertEQ(a[2] as Object, o)
        arktest.assertEQ(a[3], undefined)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<boolean|int|string|null>", () => {
        let a = [true, 1, 'a', null] as FixedArray<boolean|int|string|null>
        arktest.assertEQ(a[0] as boolean, true)
        arktest.assertEQ(a[1] as int, 1)
        arktest.assertEQ(a[2] as string, 'a')
        arktest.assertEQ(a[3], null)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<FixedArray<boolean|int|string|null>>", () => {
        let a = [[true, 1, 'a', null]] as FixedArray<FixedArray<boolean|int|string|null>>
        arktest.assertEQ(a[0][0] as boolean, true)
        arktest.assertEQ(a[0][1] as int, 1)
        arktest.assertEQ(a[0][2] as string, 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Number|Error|fn|Boolean>", () => {
        let e = new Error
        let a = [e, false, (p: string): string => p + p, 42] as Array<Number|Error|((p: string)=>string)|Boolean>
        arktest.assertEQ(a[0] as Error, e)
        arktest.assertEQ(a[1] as Boolean, false)
        arktest.assertEQ((a[2] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<FixedArray<Number|Error|fn|Boolean>>", () => {
        let e = new Error
        let a = [[e], [false], [(p: string): string => p + p], [42]] as Array<Array<Number|Error|((p: string)=>string)|Boolean>>
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("union of arrays, booleans", () => {
        let a = [false, !false, false] as UnionArrayType
        arktest.assertEQ((a as FixedArray<Boolean>)[0], false)
        arktest.assertEQ((a as FixedArray<Boolean>)[1], true)
        arktest.assertEQ((a as FixedArray<Boolean>)[2], false)
        arktest.assertEQ((a as FixedArray<Boolean>).length, 3)
    })

    suite.addTest("union of arrays, numbers", () => {
        let a = [1, 2, 43] as UnionArrayType
        arktest.assertEQ((a as FixedArray<Number>)[0], 1)
        arktest.assertEQ((a as FixedArray<Number>)[1], 2)
        arktest.assertEQ((a as FixedArray<Number>)[2], 43)
        arktest.assertEQ((a as FixedArray<Number>).length, 3)
    })

    suite.addTest("union of arrays, strings", () => {
        let s = '4'
        let a = ['1', '2', s + '3'] as FixedArray<Number>|FixedArray<String>|FixedArray<Boolean>
        arktest.assertEQ((a as FixedArray<String>)[0], '1')
        arktest.assertEQ((a as FixedArray<String>)[1], '2')
        arktest.assertEQ((a as FixedArray<String>)[2], '43')
        arktest.assertEQ((a as FixedArray<String>).length, 3)
    })

    suite.addTest("FixedArray<Color|Size>", () => {
        let a = [Color.Red, Color.Green, Size.XL] as FixedArray<Color|Size>
        arktest.assertEQ(a[0] as Color, Color.Red)
        arktest.assertEQ(a[1] as Color, Color.Green)
        arktest.assertEQ(a[2] as Size, Size.XL)
        arktest.assertEQ(a.length, 3)
    })

    return suite.run()
}
