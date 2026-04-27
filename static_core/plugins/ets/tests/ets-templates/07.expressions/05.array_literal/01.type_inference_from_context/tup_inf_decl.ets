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
  The type of a tuple literal can be inferred from the declaration context.
---*/

class A_tup {}

enum Color_tup { Red, Green, Blue }
enum Size_tup { S, L, M, XL }

type S_tup = (p: string) => string
type N_tup = (p: number) => number

function main(): int {
    let suite = new arktest.ArkTestsuite("tup_inf_decl")

    suite.addTest("declaration context, tuple of value types (short/int/int/long)", () => {
        let a: [short, int, int, long] = [1, 2, 3, -9223372036854775807-1]
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[3], -9223372036854775807-1)
    })

    suite.addTest("declaration context, tuple of value types (number/byte/float/number)", () => {
        let a: [number, byte, float, number] = [Number.NaN, 0, 3.14f, 1.7976931348623157e+308, ]
        arktest.assertTrue(Number.isNaN(a[0].toDouble()))
        arktest.assertEQ(a[1], 0)
        arktest.assertEQ(a[2], 3.14f)
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
    })

    suite.addTest("declaration context, tuple of value types (Short/Int/Byte/Long)", () => {
        let a: [Short, Int, Byte, Long] = [0b1, 2, 0x7f, -9223372036854775807-1,]
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 127)
        arktest.assertEQ(a[3], -9223372036854775807-1)
    })

    suite.addTest("declaration context, tuple of value types (Number/Byte/Float/Double)", () => {
        let a: [Number, Byte, Float, Double] = [Number.NaN, 0, 3.14f, 1.7976931348623157e+308, ]
        arktest.assertTrue(Number.isNaN(a[0].toDouble()))
        arktest.assertEQ(a[1], 0)
        arktest.assertEQ(a[2], 3.14f)
        arktest.assertEQ(a[3], 1.7976931348623157e+308)
    })

    suite.addTest("declaration context, tuple of strings", () => {
        let a: [string, string, [string, string]] = ['1', '2', ['', '-9223372036854775808', ], ]
        arktest.assertEQ(a[0], '1')
        arktest.assertEQ(a[1], '2')
        arktest.assertEQ(a[2][0], '')
        arktest.assertEQ(a[2][1], '-9223372036854775808')
    })

    suite.addTest("declaration context, tuple of class objects", () => {
        let a: [A_tup, Object|null, A_tup, Object] = [{}, null, new A_tup, {} ]
        arktest.assertTrue(a[0] instanceof A_tup)
        arktest.assertEQ(a[1], null)
        arktest.assertTrue(a[2] instanceof A_tup)
        arktest.assertTrue(a[3] instanceof Object)
    })

    suite.addTest("declaration context, tuple of tuple", () => {
        let a: [[string, Object], [Double, float]] = [["a", {}, ], [Double.POSITIVE_INFINITY - 1.0 , 16777215, ], ]
        arktest.assertEQ(a[0][0],  "a")
        arktest.assertTrue(a[0][1] instanceof Object)
        arktest.assertEQ(a[1][0], Number.POSITIVE_INFINITY)
        arktest.assertEQ(a[1][1], 16777215.0f)
    })

    suite.addTest("declaration context, tuple of tuple of value types (int/double, byte/float/short)", () => {
        let a: [[int, double], [byte, float, short]] = [[-2147483647, Double.NEGATIVE_INFINITY], [ 0x7f, 16777215.0f, -32768]]
        arktest.assertEQ(a[0][0], -2147483647)
        arktest.assertEQ(a[0][1], Double.NEGATIVE_INFINITY)
        arktest.assertEQ(a[1][0], 127)
        arktest.assertEQ(a[1][1], 16777215.0f)
        arktest.assertEQ(a[1][2], -32768)
    })

    suite.addTest("declaration context, tuple of tuple of value types (Long, Double/Number, Int/Long)", () => {
        let a: [Long, [Double, Number], [Int, Long]] = [9223372036854775807, [9007199254740991, -9007199254740991], [ 2147483647, -9223372036854775807-1]]
        arktest.assertEQ(a[0], 9223372036854775807)
        arktest.assertEQ(a[1][0], 9007199254740991)
        arktest.assertEQ(a[1][1], -9007199254740991)
        arktest.assertEQ(a[2][0], 2147483647)
        arktest.assertEQ(a[2][1], -9223372036854775807-1)
    })

    suite.addTest("declaration context, tuple of enums", () => {
        let a: [Color_tup, Size_tup, string] = [Color_tup.Green, Size_tup.XL, 'a']
        arktest.assertEQ(a[0], Color_tup.Green)
        arktest.assertEQ(a[1], Size_tup.XL)
        arktest.assertEQ(a[2], 'a')
    })

    suite.addTest("declaration context, tuple of functions", () => {
        let a: [S_tup, N_tup, boolean] = [(p: string): string => p + p, (p: number): number => p + p, true]
        arktest.assertEQ(a[0]('a'), 'aa')
        arktest.assertEQ(a[1](42), 84)
        arktest.assertEQ(a[2], true)
    })

    suite.addTest("declaration context, tuple of arrays (T[])", () => {
        let a: [byte[], long[], string[]] = [[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']]
        arktest.assertEQ(a[0][0], 0)
        arktest.assertEQ(a[0][1], 15)
        arktest.assertEQ(a[0][2], 127)
        arktest.assertEQ(a[1][0], -9223372036854775807-1)
        arktest.assertEQ(a[1][1], 9223372036854775807)
        arktest.assertEQ(a[2][0], 'a')
        arktest.assertEQ(a[2][1], 'b')
        arktest.assertEQ(a[2][2], 'c')
    })

    suite.addTest("declaration context, tuple of arrays (Array<T>)", () => {
        let a: [Array<byte>, Array<long>, Array<string>] = [[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']]
        arktest.assertEQ(a[0][0], 0)
        arktest.assertEQ(a[0][1], 15)
        arktest.assertEQ(a[0][2], 127)
        arktest.assertEQ(a[1][0], -9223372036854775807-1)
        arktest.assertEQ(a[1][1], 9223372036854775807)
        arktest.assertEQ(a[2][0], 'a')
        arktest.assertEQ(a[2][1], 'b')
        arktest.assertEQ(a[2][2], 'c')
    })

    suite.addTest("declaration context, tuple of arrays (FixedArray<T>)", () => {
        let a: [FixedArray<byte>, FixedArray<long>, FixedArray<string>] = [[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']]
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
