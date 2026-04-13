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
  The type of a tuple literal can be inferred from the call context.
---*/

class A {}

enum Color { Red, Green, Blue }
enum Size { S, L, M, XL }

type S = (p: string) => string
type N = (p: number) => number

function sum(p: [number, number, number]): number {
    return p[0] + p[1] + p[2]
}

function main(): int {
    let suite = new arktest.ArkTestsuite("tup_inf_call")

    suite.addTest("[byte, int, long]", () => {
        let verify = (a: [byte, int, long]) => {
            arktest.assertEQ(a[0], 3)
            arktest.assertEQ(a[1], 42)
            arktest.assertEQ(a[2], -9223372036854775807-1)
        }
        verify([0b11, 42, -9223372036854775807-1])
    })

    suite.addTest("[float, short, double, number]", () => {
        let verify = (a: [float, short, double, number]) => {
            arktest.assertEQ(a[0], 111e11f)
            arktest.assertEQ(a[1], 33)
            arktest.assertTrue(Number.isNaN(a[2]))
            arktest.assertEQ(a[3], 1.7976931348623157e+308)
        }
        verify([111e11f, 33, Number.NaN, 1.7976931348623157e+308, ])
    })

    suite.addTest("[Byte, Short, Int, Long]", () => {
        let verify = (a: [Byte, Short, Int, Long]) => {
            arktest.assertEQ(a[0], 1)
            arktest.assertEQ(a[1], 2)
            arktest.assertEQ(a[2], 3)
            arktest.assertEQ(a[3], -9223372036854775807-1)
        }
        verify([1, 2, 3, -9223372036854775807-1,])
    })

    suite.addTest("[Number, Float, Int]", () => {
        let verify = (a: [Number, Float, Int]) => {
            arktest.assertTrue(Number.isNaN(a[0]))
            arktest.assertEQ(a[1], 0.0f)
            arktest.assertEQ(a[2], -2147483647)
        }
        verify([Number.NaN, 0.0f, -2147483647])
    })

    suite.addTest("nested tuple of strings", () => {
        let verify = (a: [[string, string], string, string]) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '2')
            arktest.assertEQ(a[1], '')
            arktest.assertEQ(a[2], '-9223372036854775808')
        }
        verify([['1', '2',], '', '-9223372036854775808', ])
    })

    suite.addTest("[A, Object, A] class objects", () => {
        let verify = (a: [A, Object, A]) => {
            arktest.assertTrue(a[0] instanceof A)
            arktest.assertTrue(a[1] instanceof Object)
            arktest.assertTrue(a[2] instanceof A)
        }
        verify([{}, {}, new A])
    })

    suite.addTest("nested tuple of mixed types", () => {
        let verify = (a: [[string, number], [[number], [int, boolean]]]) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], 1)
            arktest.assertEQ(a[1][0][0], 42)
            arktest.assertEQ(a[1][1][0], 33)
            arktest.assertEQ(a[1][1][1], true)
        }
        verify([['1', 1], [[42], [33, true]]])
    })

    suite.addTest("nested [byte, number] tuples", () => {
        let verify = (a: [[byte, number], [number, byte]]) => {
            arktest.assertEQ(a[0][0], 3)
            arktest.assertEQ(a[0][1], 222e22)
            arktest.assertEQ(a[1][0], 333e33)
            arktest.assertEQ(a[1][1], 2)
        }
        verify([[0x03, 222e22], [333e33, 0x02]])
    })

    suite.addTest("nested [Float, Long, Short] tuples", () => {
        let verify = (a: [[Float, Float], [Long, Long], [Short]]) => {
            arktest.assertEQ(a[0][0], -16777215.0f)
            arktest.assertEQ(a[0][1], 16777215.0f)
            arktest.assertEQ(a[1][0], -9223372036854775807-1)
            arktest.assertEQ(a[1][1], 9223372036854775807)
            arktest.assertEQ(a[2][0], 32767)
        }
        verify([[-16777215, 16777215], [-9223372036854775807-1, 9223372036854775807], [32767]])
    })

    suite.addTest("sum of [number, number, number]", () => {
        arktest.assertEQ( sum([1, 2, 3]), 6.0 )
        arktest.assertEQ( sum([1.1, 2.2, 3.3, ]), 6.6 )
        arktest.assertEQ( sum([0, 0, -1]), -1.0 )
    })

    suite.addTest("[Color, Size, string] enums", () => {
        let verify = (a: [Color, Size, string]) => {
            arktest.assertEQ(a[0], Color.Green)
            arktest.assertEQ(a[1], Size.XL)
            arktest.assertEQ(a[2], 'a')
        }
        verify([Color.Green, Size.XL, 'a'])
    })

    suite.addTest("[S, N, boolean] function types", () => {
        let verify = (a: [S, N, boolean]) => {
            arktest.assertEQ(a[0]('a'), 'aa')
            arktest.assertEQ(a[1](42), 84)
            arktest.assertEQ(a[2], true)
        }
        verify([(p: string): string => p + p, (p: number): number => p + p, true])
    })

    suite.addTest("[byte[], long[], string[]] arrays", () => {
        let verify = (a: [byte[], long[], string[]]) => {
            arktest.assertEQ(a[0][0], 0)
            arktest.assertEQ(a[0][1], 15)
            arktest.assertEQ(a[0][2], 127)
            arktest.assertEQ(a[1][0], -9223372036854775807-1)
            arktest.assertEQ(a[1][1], 9223372036854775807)
            arktest.assertEQ(a[2][0], 'a')
            arktest.assertEQ(a[2][1], 'b')
            arktest.assertEQ(a[2][2], 'c')
        }
        verify([[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']])
    })

    suite.addTest("[Array<byte>, Array<long>, Array<string>]", () => {
        let verify = (a: [Array<byte>, Array<long>, Array<string>]) => {
            arktest.assertEQ(a[0][0], 0)
            arktest.assertEQ(a[0][1], 15)
            arktest.assertEQ(a[0][2], 127)
            arktest.assertEQ(a[1][0], -9223372036854775807-1)
            arktest.assertEQ(a[1][1], 9223372036854775807)
            arktest.assertEQ(a[2][0], 'a')
            arktest.assertEQ(a[2][1], 'b')
            arktest.assertEQ(a[2][2], 'c')
        }
        verify([[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']])
    })

    suite.addTest("[FixedArray<byte>, FixedArray<long>, FixedArray<string>]", () => {
        let verify = (a: [FixedArray<byte>, FixedArray<long>, FixedArray<string>]) => {
            arktest.assertEQ(a[0][0], 0)
            arktest.assertEQ(a[0][1], 15)
            arktest.assertEQ(a[0][2], 127)
            arktest.assertEQ(a[1][0], -9223372036854775807-1)
            arktest.assertEQ(a[1][1], 9223372036854775807)
            arktest.assertEQ(a[2][0], 'a')
            arktest.assertEQ(a[2][1], 'b')
            arktest.assertEQ(a[2][2], 'c')
        }
        verify([[0x00, 0x0f, 0x7f], [-9223372036854775807-1, 9223372036854775807], ['a', 'b', 'c']])
    })

    return suite.run()
}
