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
    The type of an array literal can be inferred from the assignment context.
---*/

enum Color { Red, Green, Blue }

type F = (p: Color) => int

function main(): int {
    let suite = new arktest.ArkTestsuite("arr4_inf_assn")

    suite.addTest("FixedArray<Array<int>>, int values", () => {
        let a: FixedArray<Array<int>>
        a = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("FixedArray<int[]>, int values", () => {
        let a: FixedArray<int[]>
        a = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("Array<FixedArray<int>>, int values", () => {
        let a: Array<FixedArray<int>>
        a = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("FixedArray<int>[], int values", () => {
        let a: FixedArray<int>[]
        a = [[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[2][2], 2147483647)
    })

    suite.addTest("FixedArray<Array<number>>, number values", () => {
        let a: FixedArray<Array<number>>
        a = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("FixedArray<number[]>, number values", () => {
        let a: FixedArray<number[]>
        a = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("Array<FixedArray<number>>, number values", () => {
        let a: Array<FixedArray<number>>
        a = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("FixedArray<number>[], number values", () => {
        let a: FixedArray<number>[]
        a = [[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ]
        arktest.assertEQ(a.length, 4)
        arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
        arktest.assertEQ(a[2][2], 6.0)
    })

    suite.addTest("FixedArray<Array<string>>, string values", () => {
        let a: FixedArray<Array<string>>
        let s = '1'
        a = [[s, s + s,], ['', '-9223372036854775808',], ]
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("FixedArray<string[]>, string values", () => {
        let a: FixedArray<string[]>
        let s = '1'
        a = [[s, s + s,], ['', '-9223372036854775808',], ]
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("Array<FixedArray<String>>, string values", () => {
        let a: Array<FixedArray<String>>
        let s = '1'
        a = [[s, s + s,], ['', '-9223372036854775808',], ]
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("FixedArray<String>[], string values", () => {
        let a: FixedArray<String>[]
        let s = '1'
        a = [[s, s + s,], ['', '-9223372036854775808',], ]
        arktest.assertEQ(a[0][0], '1')
        arktest.assertEQ(a[0][1], '11')
        arktest.assertEQ(a[1][0], '')
        arktest.assertEQ(a[1][1], '-9223372036854775808')
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("FixedArray<Array<bigint>>, bigint values", () => {
        let a: FixedArray<Array<bigint>>
        a = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]]
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<bigint[]>, bigint values", () => {
        let a: FixedArray<bigint[]>
        a = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]]
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<FixedArray<bigint>>, bigint values", () => {
        let a: Array<FixedArray<bigint>>
        a = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]]
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<bigint>[], bigint values", () => {
        let a: FixedArray<bigint>[]
        a = [[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]]
        arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
        arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<Array<Any>>, mixed values", () => {
        let a: FixedArray<Array<Any>>
        a = [["a", 1, true, 2n, null, undefined]]
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a[0][4], null)
        arktest.assertEQ(a[0][5], undefined)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Any[]>, mixed values", () => {
        let a: FixedArray<Any[]>
        a = [["a", 1, true, 2n, null, undefined]]
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a[0][4], null)
        arktest.assertEQ(a[0][5], undefined)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<FixedArray<Any>>, mixed values", () => {
        let a: Array<FixedArray<Any>>
        a = [["a", 1, true, 2n, null, undefined]]
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a[0][4], null)
        arktest.assertEQ(a[0][5], undefined)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Any>[], mixed values", () => {
        let a: FixedArray<Any>[]
        a = [["a", 1, true, 2n, null, undefined]]
        arktest.assertEQ(a[0][0] as string,  "a")
        arktest.assertEQ(a[0][1] as int,  1)
        arktest.assertEQ(a[0][2] as boolean,  true)
        arktest.assertEQ(a[0][3] as bigint,  2n)
        arktest.assertEQ(a[0][4], null)
        arktest.assertEQ(a[0][5], undefined)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Array<Color>>, enum values", () => {
        let a: FixedArray<Array<Color>>
        a = [[Color.Red, Color.Blue]]
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Color[]>, enum values", () => {
        let a: FixedArray<Color[]>
        a = [[Color.Red, Color.Blue]]
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<FixedArray<Color>>, enum values", () => {
        let a: Array<FixedArray<Color>>
        a = [[Color.Red, Color.Blue]]
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Color>[], enum values", () => {
        let a: FixedArray<Color>[]
        a = [[Color.Red, Color.Blue]]
        arktest.assertEQ(a[0][0], Color.Red)
        arktest.assertEQ(a[0][1], Color.Blue)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Array<F>>, function values", () => {
        let a: FixedArray<Array<F>>
        a = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]]
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<F[]>, function values", () => {
        let a: FixedArray<F[]>
        a = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]]
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("Array<FixedArray<F>>, function values", () => {
        let a: Array<FixedArray<F>>
        a = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]]
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<F>[], function values", () => {
        let a: FixedArray<F>[]
        a = [[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]]
        arktest.assertEQ(a[0][0](Color.Red), 0)
        arktest.assertEQ(a[1][0](Color.Red), 1)
        arktest.assertEQ(a[2][0](Color.Blue), 4)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("FixedArray<Array<boolean|int|string|null>>, union values", () => {
        let a: FixedArray<Array<boolean|int|string|null>>
        a = [[true, 1, 'a', null]]
        arktest.assertEQ(a[0][0], true)
        arktest.assertEQ(a[0][1], 1)
        arktest.assertEQ(a[0][2], 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<(boolean|int|string|null)[]>, union values", () => {
        let a: FixedArray<(boolean|int|string|null)[]>
        a = [[true, 1, 'a', null]]
        arktest.assertEQ(a[0][0], true)
        arktest.assertEQ(a[0][1], 1)
        arktest.assertEQ(a[0][2], 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("Array<FixedArray<boolean|int|string|null>>, union values", () => {
        let a: Array<FixedArray<boolean|int|string|null>>
        a = [[true, 1, 'a', null]]
        arktest.assertEQ(a[0][0], true)
        arktest.assertEQ(a[0][1], 1)
        arktest.assertEQ(a[0][2], 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<boolean|int|string|null>[], union values", () => {
        let a: FixedArray<boolean|int|string|null>[]
        a = [[true, 1, 'a', null]]
        arktest.assertEQ(a[0][0], true)
        arktest.assertEQ(a[0][1], 1)
        arktest.assertEQ(a[0][2], 'a')
        arktest.assertEQ(a[0][3], null)
        arktest.assertEQ(a.length, 1)
    })

    suite.addTest("FixedArray<Array<Number|Error|func|Boolean>>, complex union", () => {
        let e = new Error
        let a: FixedArray<Array<Number|Error|((p: string)=>string)|Boolean>>
        a = [[e], [false], [(p: string): string => p + p], [42]]
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<(Number|Error|func|Boolean)[]>, complex union", () => {
        let e = new Error
        let a: FixedArray<(Number|Error|((p: string)=>string)|Boolean)[]>
        a = [[e], [false], [(p: string): string => p + p], [42]]
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("Array<FixedArray<Number|Error|func|Boolean>>, complex union", () => {
        let e = new Error
        let a: Array<FixedArray<Number|Error|((p: string)=>string)|Boolean>>
        a = [[e], [false], [(p: string): string => p + p], [42]]
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<Number|Error|func|Boolean>[], complex union", () => {
        let e = new Error
        let a: FixedArray<Number|Error|((p: string)=>string)|Boolean>[]
        a = [[e], [false], [(p: string): string => p + p], [42]]
        arktest.assertEQ(a[0][0] as Error, e)
        arktest.assertEQ(a[1][0] as Boolean, false)
        arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
        arktest.assertEQ(a[3][0] as Number, 42)
        arktest.assertEQ(a.length, 4)
    })

    suite.addTest("FixedArray<Number>|...|Array<Boolean>|null, boolean values", () => {
        let a: FixedArray<Number>|FixedArray<String>|Array<Boolean>|null
        a = [false, true, false]
        let x = a as boolean[]
        arktest.assertEQ(x[0], false)
        arktest.assertEQ(x[1], true)
        arktest.assertEQ(x[2], false)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<Number>|...|Boolean[]|null, boolean values", () => {
        let a: FixedArray<Number>|FixedArray<String>|Boolean[]|null
        a = [false, true, false]
        let x = a as Array<boolean>
        arktest.assertEQ(x[0], false)
        arktest.assertEQ(x[1], true)
        arktest.assertEQ(x[2], false)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<number>|...|null, number values", () => {
        let a: Array<number>|FixedArray<String>|FixedArray<Boolean>|null
        a = [1, 2, 43]
        let x = a as number[]
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 43)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("number[]|...|null, number values", () => {
        let a: number[]|FixedArray<String>|FixedArray<Boolean>|null
        a = [1, 2, 43]
        let x = a as Array<number>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 43)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<Number>|Array<string>|...|null, string values", () => {
        let s = '4'
        let a: FixedArray<Number>|Array<string>|FixedArray<Boolean>|null
        a = ['1', '2', s + '3']
        let x = a as string[]
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '43')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<Number>|string[]|...|null, string values", () => {
        let s = '4'
        let a: FixedArray<Number>|string[]|Array<Boolean>|null
        a = ['1', '2', s + '3']
        let x = a as Array<String>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '43')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<string>|...|null, string values", () => {
        let s = '4'
        let a: FixedArray<string>|FixedArray<Boolean>|null
        a = ['1', '2', s + '3']
        let x = a as FixedArray<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '43')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<boolean>|string|FixedArray<string>|null, string values", () => {
        let s = '4'
        let a: FixedArray<boolean>|string|FixedArray<string>|null
        a = ['1', '2', s + '3']
        let x = a as FixedArray<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '43')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<string>|Array<int>, int literal", () => {
        let a: Array<string>|Array<int>
        a = [1, 2, 3]
        let x = a as Array<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<string>|FixedArray<int>, int literal", () => {
        let a: Array<string>|FixedArray<int>
        a = [1, 2, 3]
        let x = a as FixedArray<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<string>|Array<int>, int literal", () => {
        let a: FixedArray<string>|Array<int>
        a = [1, 2, 3]
        let x = a as Array<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<string>|FixedArray<int>, int literal", () => {
        let a: FixedArray<string>|FixedArray<int>
        a = [1, 2, 3]
        let x = a as FixedArray<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<int>|Array<string>, int literal", () => {
        let a: Array<int>|Array<string>
        a = [1, 2, 3]
        let x = a as Array<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<int>|Array<string>, int literal", () => {
        let a: FixedArray<int>|Array<string>
        a = [1, 2, 3]
        let x = a as FixedArray<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<int>|FixedArray<string>, int literal", () => {
        let a: Array<int>|FixedArray<string>
        a = [1, 2, 3]
        let x = a as Array<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<int>|FixedArray<string>, int literal", () => {
        let a: FixedArray<int>|FixedArray<string>
        a = [1, 2, 3]
        let x = a as FixedArray<int>
        arktest.assertEQ(x[0], 1)
        arktest.assertEQ(x[1], 2)
        arktest.assertEQ(x[2], 3)
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<string>|Array<int>, string literal", () => {
        let a: Array<string>|Array<int>
        a = ['1', '2', '3']
        let x = a as Array<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<string>|FixedArray<int>, string literal", () => {
        let a: Array<string>|FixedArray<int>
        a = ['1', '2', '3']
        let x = a as Array<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<string>|Array<int>, string literal", () => {
        let a: FixedArray<string>|Array<int>
        a = ['1', '2', '3']
        let x = a as FixedArray<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<string>|FixedArray<int>, string literal", () => {
        let a: FixedArray<string>|FixedArray<int>
        a = ['1', '2', '3']
        let x = a as FixedArray<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<int>|Array<string>, string literal", () => {
        let a: Array<int>|Array<string>
        a = ['1', '2', '3']
        let x = a as Array<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<int>|Array<string>, string literal", () => {
        let a: FixedArray<int>|Array<string>
        a = ['1', '2', '3']
        let x = a as Array<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("Array<int>|FixedArray<string>, string literal", () => {
        let a: Array<int>|FixedArray<string>
        a = ['1', '2', '3']
        let x = a as FixedArray<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    suite.addTest("FixedArray<int>|FixedArray<string>, string literal", () => {
        let a: FixedArray<int>|FixedArray<string>
        a = ['1', '2', '3']
        let x = a as FixedArray<string>
        arktest.assertEQ(x[0], '1')
        arktest.assertEQ(x[1], '2')
        arktest.assertEQ(x[2], '3')
        arktest.assertEQ(x.length, 3)
    })

    return suite.run()
}
