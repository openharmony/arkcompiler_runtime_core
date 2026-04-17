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
  The type of a tuple literal can be inferred from the casting context.
---*/

class A {}

enum Color { Red, Green, Blue }
enum Size { S, L, M, XL }

type S = (p: string) => string
type N = (p: number) => number

function main(): int {
    let suite = new arktest.ArkTestsuite("tup_inf_cast")

    suite.addTest("[short, int, int, long]", () => {
        let a = [1, 2, 3, -9223372036854775807-1] as [short, int, int, long]
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[3], -9223372036854775807-1)
    })

    suite.addTest("[number, byte, float, number]", () => {
        let a = [Number.NaN, 0, 3.14f, 1.7976931348623157e+308, ] as [number, byte, float, number]
        arktest.assertTrue(Number.isNaN(a[0].toDouble()))
        arktest.assertEQ(a[1], 0)
        arktest.assertEQ(a[2], 3.14f)
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
    })

    suite.addTest("[Short, Int, Byte, Long]", () => {
        let a = [0b1, 2, 0x7f, -9223372036854775807-1,] as [Short, Int, Byte, Long]
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 127)
        arktest.assertEQ(a[3], -9223372036854775807-1)
    })

    suite.addTest("[Number, Byte, Float, Double]", () => {
        let a = [Number.NaN, 0, 3.14f, 1.7976931348623157e+308, ] as [Number, Byte, Float, Double]
        arktest.assertTrue(Number.isNaN(a[0].toDouble()))
        arktest.assertEQ(a[1], 0)
        arktest.assertEQ(a[2], 3.14f)
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
    })

    suite.addTest("nested tuple of strings", () => {
        let a = ['1', '2', ['', '-9223372036854775808', ], ] as [string, string, [string, string]]
        arktest.assertEQ(a[0], '1')
        arktest.assertEQ(a[1], '2')
        arktest.assertEQ(a[2][0], '')
        arktest.assertEQ(a[2][1], '-9223372036854775808')
    })

    suite.addTest("[A, Object|null, A, Object] class objects", () => {
        let a = [{}, null, new A, {} ] as [A, Object|null, A, Object]
        arktest.assertTrue(a[0] instanceof A)
        arktest.assertEQ(a[1], null)
        arktest.assertTrue(a[2] instanceof A)
        arktest.assertTrue(a[3] instanceof Object)
    })

    suite.addTest("nested tuple of [string, Object] and [Double, float]", () => {
        let a = [["a", {}, ], [Double.POSITIVE_INFINITY - 1.0 , 16777215, ], ] as [[string, Object], [Double, float]]
        arktest.assertEQ(a[0][0],  "a")
        arktest.assertTrue(a[0][1] instanceof Object)
        arktest.assertEQ(a[1][0], Number.POSITIVE_INFINITY)
        arktest.assertEQ(a[1][1], 16777215.0f)
    })

    suite.addTest("nested [int, double] and [byte, float, short]", () => {
        let a = [[-2147483647, Double.NEGATIVE_INFINITY], [ 0x7f, 16777215.0f, -32768]] as [[int, double], [byte, float, short]]
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[0][1], Double.NEGATIVE_INFINITY)
        arktest.assertEQ(a[1][0], 127)
        arktest.assertEQ(a[1][1], 16777215.0f)
        arktest.assertEQ(a[1][2], -32768)
    })

    suite.addTest("nested [Long, Double, Number, Int, Long]", () => {
        let a = [9223372036854775807, [9007199254740991, -9007199254740991], [ 2147483647, -9223372036854775807-1]] as [Long, [Double, Number], [Int, Long]]
        arktest.assertEQ(a[0], 9223372036854775807)
        arktest.assertEQ(a[1][0], 9007199254740991)
        arktest.assertEQ(a[1][1], -9007199254740991)
        arktest.assertEQ(a[2][0], 2147483647)
        arktest.assertEQ(a[2][1], -9223372036854775807-1)
    })

    suite.addTest("[Color, Size, string] enums", () => {
        let a = [Color.Green, Size.XL, 'a'] as [Color, Size, string]
        arktest.assertEQ(a[0], Color.Green)
        arktest.assertEQ(a[1], Size.XL)
        arktest.assertEQ(a[2], 'a')
    })

    suite.addTest("[S, N, boolean] function types", () => {
        let a = [(p: string): string => p + p, (p: number): number => p + p, true] as [S, N, boolean]
        arktest.assertEQ(a[0]('a'), 'aa')
        arktest.assertEQ(a[1](42), 84)
        arktest.assertEQ(a[2], true)
    })

    suite.addTest("[byte[], long[], string[]] arrays", () => {
        let a = [[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']] as [byte[], long[], string[]]
        arktest.assertEQ(a[0][0], 0)
        arktest.assertEQ(a[0][1], 15)
        arktest.assertEQ(a[0][2], 127)
        arktest.assertEQ(a[1][0], -9223372036854775807-1)
        arktest.assertEQ(a[1][1], 9223372036854775807)
        arktest.assertEQ(a[2][0], 'a')
        arktest.assertEQ(a[2][1], 'b')
        arktest.assertEQ(a[2][2], 'c')
    })

    suite.addTest("[Array<byte>, Array<long>, Array<string>]", () => {
        let a = [[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']] as [Array<byte>, Array<long>, Array<string>]
        arktest.assertEQ(a[0][0], 0)
        arktest.assertEQ(a[0][1], 15)
        arktest.assertEQ(a[0][2], 127)
        arktest.assertEQ(a[1][0], -9223372036854775807-1)
        arktest.assertEQ(a[1][1], 9223372036854775807)
        arktest.assertEQ(a[2][0], 'a')
        arktest.assertEQ(a[2][1], 'b')
        arktest.assertEQ(a[2][2], 'c')
    })

    suite.addTest("[FixedArray<byte>, FixedArray<long>, FixedArray<string>]", () => {
        let a = [[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']] as [FixedArray<byte>, FixedArray<long>, FixedArray<string>]
        arktest.assertEQ(a[0][0], 0)
        arktest.assertEQ(a[0][1], 15)
        arktest.assertEQ(a[0][2], 127)
        arktest.assertEQ(a[1][0], -9223372036854775807-1)
        arktest.assertEQ(a[1][1], 9223372036854775807)
        arktest.assertEQ(a[2][0], 'a')
        arktest.assertEQ(a[2][1], 'b')
        arktest.assertEQ(a[2][2], 'c')
    })

    return suite.run()
}
