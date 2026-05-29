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

function sum(p: FixedArray<Long>): long {
    let s: long = 0
    for (let v of p)
        s += v
    return s
}

class PlainClass {}

class GenericClass<T> {}

class ClassWithMethod {
    foo(p: FixedArray<Object>): int {
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
    let suite = new arktest.ArkTestsuite("arr3_inf_call")

    suite.addTest("boolean_array", () => {
        let verify = (a: FixedArray<boolean>) => {
            arktest.assertEQ(a[0], false)
            arktest.assertEQ(a[1], true)
            arktest.assertEQ(a[2], true)
            arktest.assertEQ(a.length, 3)
        }
        verify([!true, true, true])
    })

    suite.addTest("long_array", () => {
        let verify = (a: FixedArray<long>) => {
            arktest.assertEQ(a[3], -9223372036854775808)
            arktest.assertEQ(a.length, 4)
        }
        verify([1, 2, 3, -9223372036854775808])
    })

    suite.addTest("int_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<int>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], -2147483647)
            arktest.assertEQ(a[2][2], 2147483647)
        }
        verify([[-2147483647, 2, ], [3, ], [4, 5, 2147483647, ], [], ])
    })

    suite.addTest("number_array", () => {
        let verify = (a: FixedArray<number>) => {
            arktest.assertTrue(Number.isNaN(a[0]))
            arktest.assertEQ(a[3], 1.7976931348623157e+308)
            arktest.assertEQ(a.length, 4)
        }
        verify([Number.NaN, 0, 3.14, 1.7976931348623157e+308, ])
    })

    suite.addTest("Number_array", () => {
        let verify = (a: FixedArray<Number>) => {
            arktest.assertTrue(Number.isNaN(a[0]))
            arktest.assertEQ(a[3], 1.7976931348623157e+308)
            arktest.assertEQ(a.length, 4)
        }
        verify([Number.NaN, 0, 3.14, 1.7976931348623157e+308, ])
    })

    suite.addTest("Boolean_array", () => {
        let verify = (a: FixedArray<Boolean>) => {
            arktest.assertEQ(a[0], false)
            arktest.assertEQ(a[1], true)
            arktest.assertEQ(a[2], true)
            arktest.assertEQ(a.length, 3)
        }
        verify([false, true, true])
    })

    suite.addTest("Long_array", () => {
        let verify = (a: FixedArray<Long>) => {
            arktest.assertEQ(a[3], -9223372036854775808)
            arktest.assertEQ(a.length, 4)
        }
        verify([1, 2, 3, -9223372036854775808,])
    })

    suite.addTest("string_array", () => {
        let verify = (a: FixedArray<string>) => {
            arktest.assertEQ(a[3], '-9223372036854775808')
            arktest.assertEQ(a.length, 4)
        }
        verify(['1', '2', '', '-9223372036854775808', ])
    })

    suite.addTest("string_concat_array", () => {
        let verify = (a: FixedArray<string>) => {
            arktest.assertEQ(a[0], 'x')
            arktest.assertEQ(a[1], 'x1')
            arktest.assertEQ(a[2], 'x2')
            arktest.assertEQ(a[3], 'x3')
            arktest.assertEQ(a.length, 4)
        }
        verify(['x', 'x' + '1', 'x' + '2', 'x' + '3', ])
    })

    suite.addTest("char_array_via_bar", () => {
        let verify = (a: FixedArray<Char>) => {
            arktest.assertEQ(a[0], c'+')
            arktest.assertEQ(a[1], c',')
            arktest.assertEQ(a[2], c'-')
            arktest.assertEQ(a.length, 3)
        }
        verify([bar(42), bar(43), bar(44), ])
    })

    suite.addTest("String_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<String>>) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '11')
            arktest.assertEQ(a[1][0], '')
            arktest.assertEQ(a[1][1], '-9223372036854775808')
            arktest.assertEQ(a.length, 2)
        }
        let s = '1'
        verify([[s, s + s,], ['', '-9223372036854775808',], ])
    })

    suite.addTest("bigint_array", () => {
        let verify = (a: FixedArray<bigint>) => {
            arktest.assertEQ(a[1], 2n)
            arktest.assertEQ(a[2], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([1n, 2n, 33333333333333333333333333333333333333333333333333n])
    })

    suite.addTest("bigint_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<bigint>>) => {
            arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
            arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]])
    })

    suite.addTest("PlainClass_array", () => {
        let verify = (a: FixedArray<PlainClass>) => {
            arktest.assertTrue(a[0] instanceof PlainClass)
            arktest.assertEQ(a.length, 1)
        }
        verify([{}])
    })

    suite.addTest("GenericClass_array", () => {
        let v = new GenericClass<Number>
        let q = new GenericClass<Number>
        verifyGenericClass<Number>([v, q])
    })

    suite.addTest("Number_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<Number>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
            arktest.assertEQ(a[2][2], 6.0)
        }
        verify([[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ])
    })

    suite.addTest("Object_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<Object>>) => {
            arktest.assertEQ(a[0][0] as string,  "a")
            arktest.assertEQ(a[0][1] as int,  1)
            arktest.assertEQ(a[0][2] as boolean,  true)
            arktest.assertEQ(a[0][3] as bigint,  2n)
            arktest.assertEQ(a.length, 1)
        }
        verify([["a", 1, true, 2n]])
    })

    suite.addTest("sum_Long_array", () => {
        arktest.assertEQ( sum([1, 2, 3]),  6 )
        arktest.assertEQ( sum([]),  0 )
        arktest.assertEQ( sum([0x0f0f0f0f, 0b10110011001, 42, (8).toByte()]),  252646618 )
    })

    suite.addTest("ClassWithMethod_foo", () => {
        arktest.assertEQ( new ClassWithMethod().foo([new Object(), new Long(), new Int(), ]),  3 )
        arktest.assertEQ( new ClassWithMethod().foo([new ClassWithMethod(), new ClassWithMethod(), new Object()]),  5 )
        arktest.assertEQ( new ClassWithMethod().foo(["aaa", new ClassWithMethod(), new Object(), ]),  4 )
    })

    suite.addTest("Color_array", () => {
        let verify = (a: FixedArray<Color>) => {
            arktest.assertEQ(a[0], Color.Red)
            arktest.assertEQ(a[1], Color.Blue)
            arktest.assertEQ(a.length, 2)
        }
        verify([Color.Red, Color.Blue])
    })

    suite.addTest("Color_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<Color>>) => {
            arktest.assertEQ(a[0][0], Color.Red)
            arktest.assertEQ(a[0][1], Color.Blue)
            arktest.assertEQ(a.length, 1)
        }
        verify([[Color.Red, Color.Blue]])
    })

    suite.addTest("F_array", () => {
        let verify = (a: Array<F>) => {
            arktest.assertEQ(a[0](Color.Red), 0)
            arktest.assertEQ(a[1](Color.Red), 1)
            arktest.assertEQ(a[2](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([(p: Color): int => p.valueOf(), (p: Color): int => p.valueOf() + 1, (p: Color): int => p.valueOf() + 2])
    })

    suite.addTest("F_array_of_arrays", () => {
        let verify = (a: Array<Array<F>>) => {
            arktest.assertEQ(a[0][0](Color.Red), 0)
            arktest.assertEQ(a[1][0](Color.Red), 1)
            arktest.assertEQ(a[2][0](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]])
    })

    suite.addTest("null_undefined_Object_union", () => {
        let u: undefined = undefined
        let n: null = null
        let o: Object = {}
        let verify = (a: FixedArray<null|undefined|Object>) => {
            arktest.assertEQ(a[0], null)
            arktest.assertEQ(a[1] as Object, o)
            arktest.assertEQ(a[2] as Object, o)
            arktest.assertEQ(a[3], undefined)
            arktest.assertEQ(a.length, 4)
        }
        verify([n, o, o, u])
    })

    suite.addTest("primitive_union", () => {
        let verify = (a: FixedArray<boolean|int|string|null>) => {
            arktest.assertEQ(a[0] as boolean, true)
            arktest.assertEQ(a[1] as int, 1)
            arktest.assertEQ(a[2] as string, 'a')
            arktest.assertEQ(a[3], null)
            arktest.assertEQ(a.length, 4)
        }
        verify([true, 1, 'a', null])
    })

    suite.addTest("primitive_union_array_of_arrays", () => {
        let verify = (a: FixedArray<FixedArray<boolean|int|string|null>>) => {
            arktest.assertEQ(a[0][0] as boolean, true)
            arktest.assertEQ(a[0][1] as int, 1)
            arktest.assertEQ(a[0][2] as string, 'a')
            arktest.assertEQ(a[0][3], null)
            arktest.assertEQ(a.length, 1)
        }
        verify([[true, 1, 'a', null]])
    })

    suite.addTest("error_func_value_union", () => {
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

    suite.addTest("error_func_value_union_arrays", () => {
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

    suite.addTest("union_of_arrays_boolean", () => {
        let verify = (a: FixedArray<Number>|FixedArray<String>|FixedArray<Boolean>) => {
            arktest.assertEQ((a as FixedArray<Boolean>)[0], false)
            arktest.assertEQ((a as FixedArray<Boolean>)[1], true)
            arktest.assertEQ((a as FixedArray<Boolean>)[2], false)
            arktest.assertEQ((a as FixedArray<Boolean>).length, 3)
        }
        verify([!true, true, !true])
    })

    suite.addTest("union_of_arrays_number", () => {
        let verify = (a: FixedArray<Number>|FixedArray<String>|FixedArray<Boolean>) => {
            arktest.assertEQ((a as FixedArray<Number>)[0], 1)
            arktest.assertEQ((a as FixedArray<Number>)[1], 2)
            arktest.assertEQ((a as FixedArray<Number>)[2], 43)
            arktest.assertEQ((a as FixedArray<Number>).length, 3)
        }
        verify([1, 2, 40 + 3.0])
    })

    suite.addTest("union_of_arrays_string", () => {
        let verify = (a: FixedArray<Number>|FixedArray<String>|FixedArray<Boolean>) => {
            arktest.assertEQ((a as FixedArray<String>)[0], '1')
            arktest.assertEQ((a as FixedArray<String>)[1], '2')
            arktest.assertEQ((a as FixedArray<String>)[2], '43')
            arktest.assertEQ((a as FixedArray<String>).length, 3)
        }
        verify(['1', '2', '43'])
    })

    suite.addTest("Color_Size_union", () => {
        let verify = (a: FixedArray<Color|Size>) => {
            arktest.assertEQ(a[0] as Color, Color.Red)
            arktest.assertEQ(a[1] as Color, Color.Green)
            arktest.assertEQ(a[2] as Size, Size.XL)
            arktest.assertEQ(a.length, 3)
        }
        verify([Color.Red, Color.Green, Size.XL])
    })

    return suite.run()
}
