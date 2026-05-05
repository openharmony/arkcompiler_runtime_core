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

/*--- desc: The type of an array literal can be inferred from the call context. ---*/

function bar(i: Int): Char {
    let v: Int = i + 1
    return v.toChar()
}

function sum(p: Array<Long>): long {
    let s: long = 0
    for (let v of p)
        s += v
    return s
}

class PlainClass {}

class GenericClass<T> {}

class ClassWithMethod {
    foo(p: Array<Object>): int {
        let s = p.length
        for (let v of p) {
            s += v instanceof ClassWithMethod ? 1 : 0
        }
        return s
    }
}

function verifyGenericClass<T>(a: Array<GenericClass<T>>) {
    arktest.assertEQ(a.length, 2)
}

enum Color { Red, Green, Blue }

enum Size { S, M, L, XL }

type F = (p: Color) => int

function main(): int {
    let suite = new arktest.ArkTestsuite("Array inference from call context")

    suite.addTest("Array<boolean>", () => {
        let verify = (a: Array<boolean>) => {
            arktest.assertEQ(a[0], false)
            arktest.assertEQ(a[1], true)
            arktest.assertEQ(a[2], true)
            arktest.assertEQ(a.length, 3)
        }
        verify([!true, true, true])
    })

    suite.addTest("Array<long>", () => {
        let verify = (a: Array<long>) => {
            arktest.assertEQ(a[3], -9223372036854775808)
            arktest.assertEQ(a.length, 4)
        }
        verify([1, 2, 3, -9223372036854775808])
    })

    suite.addTest("Array<Array<int>>", () => {
        let verify = (a: Array<Array<int>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], -2147483647)
            arktest.assertEQ(a[2][2], 2147483647)
        }
        verify([[-2147483647, 2, ], [3, ], [4, 5, 2147483647, ], [], ])
    })

    suite.addTest("Array<number>", () => {
        let verify = (a: Array<number>) => {
            arktest.assertTrue(Number.isNaN(a[0]))
            arktest.assertEQ(a[3], 1.7976931348623157e+308)
            arktest.assertEQ(a.length, 4)
        }
        verify([Number.NaN, 0, 3.14, 1.7976931348623157e+308, ])
    })

    suite.addTest("Array<Number>", () => {
        let verify = (a: Array<Number>) => {
            arktest.assertTrue(Number.isNaN(a[0]))
            arktest.assertEQ(a[3], 1.7976931348623157e+308)
            arktest.assertEQ(a.length, 4)
        }
        verify([Number.NaN, 0, 3.14, 1.7976931348623157e+308, ])
    })

    suite.addTest("Array<Boolean>", () => {
        let verify = (a: Array<Boolean>) => {
            arktest.assertEQ(a[0], false)
            arktest.assertEQ(a[1], true)
            arktest.assertEQ(a[2], true)
            arktest.assertEQ(a.length, 3)
        }
        verify([false, true, true])
    })

    suite.addTest("Array<Long>", () => {
        let verify = (a: Array<Long>) => {
            arktest.assertEQ(a[3], -9223372036854775808)
            arktest.assertEQ(a.length, 4)
        }
        verify([1, 2, 3, -9223372036854775808,])
    })

    suite.addTest("Array<string>", () => {
        let verify = (a: Array<string>) => {
            arktest.assertEQ(a[3], '-9223372036854775808')
            arktest.assertEQ(a.length, 4)
        }
        verify(['1', '2', '', '-9223372036854775808', ])
    })

    suite.addTest("Array<string> concat", () => {
        let verify = (a: Array<string>) => {
            arktest.assertEQ(a[0], 'x')
            arktest.assertEQ(a[1], 'x1')
            arktest.assertEQ(a[2], 'x2')
            arktest.assertEQ(a[3], 'x3')
            arktest.assertEQ(a.length, 4)
        }
        verify(['x', 'x' + '1', 'x' + '2', 'x' + '3', ])
    })

    suite.addTest("Array<Char>", () => {
        let verify = (a: Array<Char>) => {
            arktest.assertEQ(a[0], c'+')
            arktest.assertEQ(a[1], c',')
            arktest.assertEQ(a[2], c'-')
            arktest.assertEQ(a.length, 3)
        }
        verify([bar(42), bar(43), bar(44), ])
    })

    suite.addTest("Array<Array<String>>", () => {
        let verify = (a: Array<Array<String>>) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '11')
            arktest.assertEQ(a[1][0], '')
            arktest.assertEQ(a[1][1], '-9223372036854775808')
            arktest.assertEQ(a.length, 2)
        }
        let s = '1'
        verify([[s, s + s,], ['', '-9223372036854775808',], ])
    })

    suite.addTest("Array<bigint>", () => {
        let verify = (a: Array<bigint>) => {
            arktest.assertEQ(a[1], 2n)
            arktest.assertEQ(a[2], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([1n, 2n, 33333333333333333333333333333333333333333333333333n])
    })

    suite.addTest("Array<Array<bigint>>", () => {
        let verify = (a: Array<Array<bigint>>) => {
            arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
            arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]])
    })

    suite.addTest("Array<PlainClass>", () => {
        let verify = (a: Array<PlainClass>) => {
            arktest.assertTrue(a[0] instanceof PlainClass)
            arktest.assertEQ(a.length, 1)
        }
        verify([{}])
    })

    suite.addTest("Array<GenericClass<T>>", () => {
        let v = new GenericClass<Number>
        let q = new GenericClass<Number>
        verifyGenericClass<Number>([v, q])
    })

    suite.addTest("Array<Array<Number>>", () => {
        let verify = (a: Array<Array<Number>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
            arktest.assertEQ(a[2][2], 6.0)
        }
        verify([[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ])
    })

    suite.addTest("Array<Array<Object>>", () => {
        let verify = (a: Array<Array<Object>>) => {
            arktest.assertEQ(a[0][0] as string,  "a")
            arktest.assertEQ(a[0][1] as int,  1)
            arktest.assertEQ(a[0][2] as boolean,  true)
            arktest.assertEQ(a[0][3] as bigint,  2n)
            arktest.assertEQ(a.length, 1)
        }
        verify([["a", 1, true, 2n]])
    })

    suite.addTest("sum Array<Long>", () => {
        arktest.assertEQ( sum([1, 2, 3]),  6 )
        arktest.assertEQ( sum([]),  0 )
        arktest.assertEQ( sum([0x0f0f0f0f, 0b10110011001, 42, (8).toByte()]),  252646618 )
    })

    suite.addTest("ClassWithMethod foo Array<Object>", () => {
        arktest.assertEQ( new ClassWithMethod().foo([new Object(), new Long(), new Int(), ]),  3 )
        arktest.assertEQ( new ClassWithMethod().foo([new ClassWithMethod(), new ClassWithMethod(), new Object()]),  5 )
        arktest.assertEQ( new ClassWithMethod().foo(["aaa", new ClassWithMethod(), new Object(), ]),  4 )
    })

    suite.addTest("Array<Color>", () => {
        let verify = (a: Array<Color>) => {
            arktest.assertEQ(a[0], Color.Red)
            arktest.assertEQ(a[1], Color.Blue)
            arktest.assertEQ(a.length, 2)
        }
        verify([Color.Red, Color.Blue])
    })

    suite.addTest("Array<Array<Color>>", () => {
        let verify = (a: Array<Array<Color>>) => {
            arktest.assertEQ(a[0][0], Color.Red)
            arktest.assertEQ(a[0][1], Color.Blue)
            arktest.assertEQ(a.length, 1)
        }
        verify([[Color.Red, Color.Blue]])
    })

    suite.addTest("Array<F>", () => {
        let verify = (a: Array<F>) => {
            arktest.assertEQ(a[0](Color.Red), 0)
            arktest.assertEQ(a[1](Color.Red), 1)
            arktest.assertEQ(a[2](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([(p: Color): int => p.valueOf(), (p: Color): int => p.valueOf() + 1, (p: Color): int => p.valueOf() + 2])
    })

    suite.addTest("Array<Array<F>>", () => {
        let verify = (a: Array<Array<F>>) => {
            arktest.assertEQ(a[0][0](Color.Red), 0)
            arktest.assertEQ(a[1][0](Color.Red), 1)
            arktest.assertEQ(a[2][0](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]])
    })

    suite.addTest("Array<null|undefined|Object>", () => {
        let u: undefined = undefined
        let n: null = null
        let o: Object = {}
        let verify = (a: Array<null|undefined|Object>) => {
            arktest.assertEQ(a[0], null)
            arktest.assertEQ(a[1] as Object, o)
            arktest.assertEQ(a[2] as Object, o)
            arktest.assertEQ(a[3], undefined)
            arktest.assertEQ(a.length, 4)
        }
        verify([n, o, o, u])
    })

    suite.addTest("Array<boolean|int|string|null>", () => {
        let verify = (a: Array<boolean|int|string|null>) => {
            arktest.assertEQ(a[0] as boolean, true)
            arktest.assertEQ(a[1] as int, 1)
            arktest.assertEQ(a[2] as string, 'a')
            arktest.assertEQ(a[3], null)
            arktest.assertEQ(a.length, 4)
        }
        verify([true, 1, 'a', null])
    })

    suite.addTest("Array<Array<boolean|int|string|null>>", () => {
        let verify = (a: Array<Array<boolean|int|string|null>>) => {
            arktest.assertEQ(a[0][0] as boolean, true)
            arktest.assertEQ(a[0][1] as int, 1)
            arktest.assertEQ(a[0][2] as string, 'a')
            arktest.assertEQ(a[0][3], null)
            arktest.assertEQ(a.length, 1)
        }
        verify([[true, 1, 'a', null]])
    })

    suite.addTest("Array<Number|Error|fn|Boolean>", () => {
        let e = new Error
        let verify = (a: Array<Number|Error|((p: string)=>string)|Boolean>) => {
            arktest.assertEQ(a[0] as Error, e)
            arktest.assertEQ(a[1] as Boolean, false)
            arktest.assertEQ((a[2] as ((p: string) => string))('a'), 'aa')
            arktest.assertEQ(a[3] as Number, 42)
            arktest.assertEQ(a.length, 4)
        }
        verify([e, false, (p: string): string => p + p, 42])
    })

    suite.addTest("Array<Array<Number|Error|fn|Boolean>>", () => {
        let e = new Error
        let verify = (a: Array<Array<Number|Error|((p: string)=>string)|Boolean>>) => {
            arktest.assertEQ(a[0][0] as Error, e)
            arktest.assertEQ(a[1][0] as Boolean, false)
            arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
            arktest.assertEQ(a[3][0] as Number, 42)
            arktest.assertEQ(a.length, 4)
        }
        verify([[e], [false], [(p: string): string => p + p], [42]])
    })

    suite.addTest("union of arrays Boolean", () => {
        let verify = (a: Array<Number>|Array<String>|Array<Boolean>) => {
            arktest.assertEQ((a as Array<Boolean>)[0], false)
            arktest.assertEQ((a as Array<Boolean>)[1], true)
            arktest.assertEQ((a as Array<Boolean>)[2], false)
            arktest.assertEQ((a as Array<Boolean>).length, 3)
        }
        verify([!true, true, !true])
    })

    suite.addTest("union of arrays Number", () => {
        let verify = (a: Array<Number>|Array<String>|Array<Boolean>) => {
            arktest.assertEQ((a as Array<Number>)[0], 1)
            arktest.assertEQ((a as Array<Number>)[1], 2)
            arktest.assertEQ((a as Array<Number>)[2], 43)
            arktest.assertEQ((a as Array<Number>).length, 3)
        }
        verify([1, 2, 40 + 3.0])
    })

    suite.addTest("union of arrays String", () => {
        let verify = (a: Array<Number>|Array<String>|Array<Boolean>) => {
            arktest.assertEQ((a as Array<String>)[0], '1')
            arktest.assertEQ((a as Array<String>)[1], '2')
            arktest.assertEQ((a as Array<String>)[2], '43')
            arktest.assertEQ((a as Array<String>).length, 3)
        }
        verify(['1', '2', '43'])
    })

    suite.addTest("Array<Color|Size>", () => {
        let verify = (a: Array<Color|Size>) => {
            arktest.assertEQ(a[0] as Color, Color.Red)
            arktest.assertEQ(a[1] as Color, Color.Green)
            arktest.assertEQ(a[2] as Size, Size.XL)
            arktest.assertEQ(a.length, 3)
        }
        verify([Color.Red, Color.Green, Size.XL])
    })

    return suite.run()
}
