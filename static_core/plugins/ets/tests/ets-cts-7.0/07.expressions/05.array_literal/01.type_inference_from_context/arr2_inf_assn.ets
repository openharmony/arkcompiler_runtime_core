/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: The type of an array literal can be inferred from the assignment context.
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

function sum(p: Array<Number>): number {
    let s: number = 0
    for (let v of p)
        s += v
    return s
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Array<T> inference from assignment context")

    suite.addTest("Array<boolean>", () => {
        let a: Array<boolean> = [true, true, true, true, true, true]
        a = [!true, true, false]
        arktest.assertEQ(a[0], false)
        arktest.assertEQ(a[1], true)
        arktest.assertEQ(a[2], false)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<long>", () => {
        let a: Array<long> = []
        a = [1, 2, 3, -9223372036854775807 - 1]
        arktest.assertEQ(a[3], -9223372036854775808)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Array<int>>", () => {
        let a: Array<Array<int>> = []
        a = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("Array<number>", () => {
        let a: Array<number> = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,]
        a = [Number.NaN, 0, 3.14, 1.7976931348623157e+308, ]
        arktest.assertTrue(Number.isNaN(a[0]))
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Boolean>", () => {
        let a: Array<Boolean> = []
        a = [false, true, true]
        arktest.assertEQ(a[0], false)
        arktest.assertEQ(a[1], true)
        arktest.assertEQ(a[2], true)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<Long>", () => {
        let a: Array<Long> = []
        a = [1, 2, 3, -9223372036854775808,]
        arktest.assertEQ(a[3], -9223372036854775808)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Number>", () => {
        let a: Array<Number> = []
        a = [Number.NaN, 0, 3.14, 1.7976931348623157e+308, ]
        arktest.assertTrue(Number.isNaN(a[0]))
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Array<Number>>", () => {
        let a: Array<Array<Number>> = []
        a = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("Array<string>", () => {
        let a: Array<string> = []
        a = ['1', '2', '', '-9223372036854775808', ]
        arktest.assertEQ(a[3], '-9223372036854775808')
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<string> concatenation", () => {
        let a: Array<string> = ['','','','','','','','','','','','','','','',]
        a = ['x', 'x' + '1', 'x' + '2', 'x' + '3', ]
        arktest.assertEQ(a[0], 'x')
        arktest.assertEQ(a[1], 'x1')
        arktest.assertEQ(a[2], 'x2')
        arktest.assertEQ(a[3], 'x3')
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Char>", () => {
        let a: Array<Char> = []
        a = [bar(42), bar(43), bar(44), ]
        arktest.assertEQ(a[0], c'+')
        arktest.assertEQ(a[1], c',')
        arktest.assertEQ(a[2], c'-')
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<Array<String>>", () => {
        let a: Array<Array<String>> = []
        let s = '1'
        a = [[s, s + s,], ['', '-9223372036854775808',], ]
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("Array<bigint>", () => {
        let a: Array<bigint> = []
        a = [1n, 2n, 33333333333333333333333333333333333333333333333333n]
        arktest.assertEQ(a[1], 2n)
        arktest.assertEQ(a[2], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<Array<bigint>>", () => {
        let a: Array<Array<bigint>> = []
        a = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]]
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<PlainClass>", () => {
        let a: Array<PlainClass> = []
        a = [{}]
        arktest.assertTrue(a[0] instanceof PlainClass)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<GenericClass<Number>>", () => {
        let v = new GenericClass<Number>
        let q = new GenericClass<Number>
        let a: Array<GenericClass<Number>> = []
        a = [v, q]
        arktest.assertEQ(a[0], v)
        arktest.assertEQ(a[1], q)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("Array<Array<Object>>", () => {
        let a: Array<Array<Object>> = []
        a = [["a", 1, true, 2n]]
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<Color>", () => {
        let a: Array<Color> = []
        a = [Color.Red, Color.Blue]
        arktest.assertEQ(a[0], Color.Red)
        arktest.assertEQ(a[1], Color.Blue)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("Array<Array<Color>>", () => {
        let a: Array<Array<Color>> = []
        a = [[Color.Red, Color.Blue]]
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<F> functions", () => {
        let a: Array<F> = []
        a = [(p: Color): int => p.valueOf(), (p: Color): int => p.valueOf() + 1, (p: Color): int => p.valueOf() + 2]
        arktest.assertEQ(a[0](Color.Red), 0)
        arktest.assertEQ(a[1](Color.Red), 1)
        arktest.assertEQ(a[2](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<Array<F>> functions", () => {
        let a: Array<Array<F>> = []
        a = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]]
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<null|undefined|Object>", () => {
        let u: undefined = undefined
        let n: null = null
        let o: Object = {}
        let a: Array<null|undefined|Object> = []
        a = [n, o, o, u]
        arktest.assertEQ(a[0], null)
        arktest.assertEQ(a[1] as Object, o)
        arktest.assertEQ(a[2] as Object, o)
        arktest.assertEQ(a[3], undefined)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("sum function", () => {
        let a: Array<number> = [0]
        a = [1, 2, 3, ]
        arktest.assertTrue( a.length == 3 && sum(a) == 6.0 )
        a = [1.1, 2.2, 3.3, 4.4]
        arktest.assertTrue( a.length == 4 && sum(a) == 11.0 )
        a = [0, 0, ]
        arktest.assertTrue( a.length == 2 && sum(a) == 0.0 )
    })

    suite.addTest("Array<boolean|int|string|null>", () => {
        let a: Array<boolean|int|string|null> = []
        a = [true, 1, 'a', null]
        arktest.assertEQ(a[0], true)
        arktest.assertEQ(a[1], 1)
        arktest.assertEQ(a[2], 'a')
        arktest.assertEQ(a[3], null)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Array<boolean|int|string|null>>", () => {
        let a: Array<Array<boolean|int|string|null>> = []
        a = [[true, 1, 'a', null]]
        arktest.assertEQ(a[0][0], true)
        arktest.assertEQ(a[0][1], 1)
        arktest.assertEQ(a[0][2], 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<Number|Error|Function|Boolean>", () => {
        let e = new Error
        let a: Array<Number|Error|((p: string)=>string)|Boolean> = []
        a = [e, false, (p: string): string => p + p, 42]
        arktest.assertEQ(a[0] as Error, e)
        arktest.assertEQ(a[1] as Boolean, false)
        arktest.assertEQ((a[2] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<Array<Number|Error|Function|Boolean>>", () => {
        let e = new Error
        let a: Array<Array<Number|Error|((p: string)=>string)|Boolean>> = []
        a = [[e], [false], [(p: string): string => p + p], [42]]
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("union of arrays, booleans", () => {
        let a: Array<Number>|Array<String>|Array<Boolean>|null
        a = [false, true, false]
        arktest.assertEQ((a as Array<Boolean>)[0], false)
        arktest.assertEQ((a as Array<Boolean>)[1], true)
        arktest.assertEQ((a as Array<Boolean>)[2], false)
        arktest.assertEQ((a as Array<Boolean>).length, 3)
    })

    suite.addTest("union of arrays, numbers", () => {
        let a: Array<Number>|Array<String>|Array<Boolean>|null
        a = [1, 2, 43]
        arktest.assertEQ((a as Array<Number>)[0], 1)
        arktest.assertEQ((a as Array<Number>)[1], 2)
        arktest.assertEQ((a as Array<Number>)[2], 43)
        arktest.assertEQ((a as Array<Number>).length, 3)
    })

    suite.addTest("union of arrays, strings", () => {
        let a: Array<Number>|Array<String>|Array<Boolean>|null
        let s = '4'
        a = ['1', '2', s + '3']
        arktest.assertEQ((a as Array<String>)[0], '1')
        arktest.assertEQ((a as Array<String>)[1], '2')
        arktest.assertEQ((a as Array<String>)[2], '43')
        arktest.assertEQ((a as Array<String>).length, 3)
    })

    suite.addTest("Array<Color|Size>", () => {
        let a: Array<Color|Size> = []
        a = [Color.Red, Color.Green, Size.XL]
        arktest.assertEQ(a[0] as Color, Color.Red)
        arktest.assertEQ(a[1] as Color, Color.Green)
        arktest.assertEQ(a[2] as Size, Size.XL)
        arktest.assertEQ(a.length, 3)
    })

    return suite.run()
}
