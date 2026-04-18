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
    The type of an array literal can be inferred from the call context.
---*/

enum Color { Red, Green, Blue }
type F = (p: Color) => int

function main(): int {
    let suite = new arktest.ArkTestsuite("arr4_inf_call")

    suite.addTest("FixedArray<Array<int>>", () => {
        let verify = (a: FixedArray<Array<int>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], -2147483647)
            arktest.assertEQ(a[2][2], 2147483647)
        }
        verify([[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ])
    })

    suite.addTest("FixedArray<int[]>", () => {
        let verify = (a: FixedArray<int[]>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], -2147483647)
            arktest.assertEQ(a[2][2], 2147483647)
        }
        verify([[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ])
    })

    suite.addTest("Array<FixedArray<int>>", () => {
        let verify = (a: Array<FixedArray<int>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], -2147483647)
            arktest.assertEQ(a[2][2], 2147483647)
        }
        verify([[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ])
    })

    suite.addTest("FixedArray<int>[]", () => {
        let verify = (a: FixedArray<int>[]) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], -2147483647)
            arktest.assertEQ(a[2][2], 2147483647)
        }
        verify([[-2147483647, 2, ], [3, ], [2 + 2, 5, 2147483646 + 1, ], [], ])
    })

    suite.addTest("FixedArray<Array<number>>", () => {
        let verify = (a: FixedArray<Array<number>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
            arktest.assertEQ(a[2][2], 6.0)
        }
        verify([[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ])
    })

    suite.addTest("FixedArray<number[]>", () => {
        let verify = (a: FixedArray<number[]>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
            arktest.assertEQ(a[2][2], 6.0)
        }
        verify([[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ])
    })

    suite.addTest("Array<FixedArray<number>>", () => {
        let verify = (a: Array<FixedArray<number>>) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
            arktest.assertEQ(a[2][2], 6.0)
        }
        verify([[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ])
    })

    suite.addTest("FixedArray<number>[]", () => {
        let verify = (a: FixedArray<number>[]) => {
            arktest.assertEQ(a.length, 4)
            arktest.assertEQ(a[0][0], 1.7976931348623157e+308)
            arktest.assertEQ(a[2][2], 6.0)
        }
        verify([[1.7976931348623157e+308, 2, ], [3, ], [4, 5, 6, ], [], ])
    })

    suite.addTest("FixedArray<Array<string>>", () => {
        let s = '1'
        let verify = (a: FixedArray<Array<string>>) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '11')
            arktest.assertEQ(a[1][0], '')
            arktest.assertEQ(a[1][1], '-9223372036854775808')
            arktest.assertEQ(a.length, 2)
        }
        verify([[s, s + s,], ['', '-9223372036854775808',], ])
    })

    suite.addTest("FixedArray<string[]>", () => {
        let s = '1'
        let verify = (a: FixedArray<string[]>) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '11')
            arktest.assertEQ(a[1][0], '')
            arktest.assertEQ(a[1][1], '-9223372036854775808')
            arktest.assertEQ(a.length, 2)
        }
        verify([[s, s + s,], ['', '-9223372036854775808',], ])
    })

    suite.addTest("Array<FixedArray<String>>", () => {
        let s = '1'
        let verify = (a: Array<FixedArray<String>>) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '11')
            arktest.assertEQ(a[1][0], '')
            arktest.assertEQ(a[1][1], '-9223372036854775808')
            arktest.assertEQ(a.length, 2)
        }
        verify([[s, s + s,], ['', '-9223372036854775808',], ])
    })

    suite.addTest("FixedArray<String>[]", () => {
        let s = '1'
        let verify = (a: FixedArray<String>[]) => {
            arktest.assertEQ(a[0][0], '1')
            arktest.assertEQ(a[0][1], '11')
            arktest.assertEQ(a[1][0], '')
            arktest.assertEQ(a[1][1], '-9223372036854775808')
            arktest.assertEQ(a.length, 2)
        }
        verify([[s, s + s,], ['', '-9223372036854775808',], ])
    })

    suite.addTest("FixedArray<Array<bigint>>", () => {
        let verify = (a: FixedArray<Array<bigint>>) => {
            arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
            arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]])
    })

    suite.addTest("FixedArray<bigint[]>", () => {
        let verify = (a: FixedArray<bigint[]>) => {
            arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
            arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]])
    })

    suite.addTest("Array<FixedArray<bigint>>", () => {
        let verify = (a: Array<FixedArray<bigint>>) => {
            arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
            arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]])
    })

    suite.addTest("FixedArray<bigint>[]", () => {
        let verify = (a: FixedArray<bigint>[]) => {
            arktest.assertEQ(a[0][0], 11111111111111111111111111111111111111111111111n)
            arktest.assertEQ(a[2][0], 33333333333333333333333333333333333333333333333333n)
            arktest.assertEQ(a.length, 3)
        }
        verify([[11111111111111111111111111111111111111111111111n], [2n], [33333333333333333333333333333333333333333333333333n]])
    })

    suite.addTest("FixedArray<Array<Any>>", () => {
        let verify = (a: FixedArray<Array<Any>>) => {
            arktest.assertEQ(a[0][0] as string,  "a")
            arktest.assertEQ(a[0][1] as int,  1)
            arktest.assertEQ(a[0][2] as boolean,  true)
            arktest.assertEQ(a[0][3] as bigint,  2n)
            arktest.assertEQ(a[0][4], null)
            arktest.assertEQ(a[0][5], undefined)
            arktest.assertEQ(a.length, 1)
        }
        verify([["a", 1, true, 2n, null, undefined]])
    })

    suite.addTest("FixedArray<Any[]>", () => {
        let verify = (a: FixedArray<Any[]>) => {
            arktest.assertEQ(a[0][0] as string,  "a")
            arktest.assertEQ(a[0][1] as int,  1)
            arktest.assertEQ(a[0][2] as boolean,  true)
            arktest.assertEQ(a[0][3] as bigint,  2n)
            arktest.assertEQ(a[0][4], null)
            arktest.assertEQ(a[0][5], undefined)
            arktest.assertEQ(a.length, 1)
        }
        verify([["a", 1, true, 2n, null, undefined]])
    })

    suite.addTest("Array<FixedArray<Any>>", () => {
        let verify = (a: Array<FixedArray<Any>>) => {
            arktest.assertEQ(a[0][0] as string,  "a")
            arktest.assertEQ(a[0][1] as int,  1)
            arktest.assertEQ(a[0][2] as boolean,  true)
            arktest.assertEQ(a[0][3] as bigint,  2n)
            arktest.assertEQ(a[0][4], null)
            arktest.assertEQ(a[0][5], undefined)
            arktest.assertEQ(a.length, 1)
        }
        verify([["a", 1, true, 2n, null, undefined]])
    })

    suite.addTest("FixedArray<Any>[]", () => {
        let verify = (a: FixedArray<Any>[]) => {
            arktest.assertEQ(a[0][0] as string,  "a")
            arktest.assertEQ(a[0][1] as int,  1)
            arktest.assertEQ(a[0][2] as boolean,  true)
            arktest.assertEQ(a[0][3] as bigint,  2n)
            arktest.assertEQ(a[0][4], null)
            arktest.assertEQ(a[0][5], undefined)
            arktest.assertEQ(a.length, 1)
        }
        verify([["a", 1, true, 2n, null, undefined]])
    })

    suite.addTest("FixedArray<Array<Color>>", () => {
        let verify = (a: FixedArray<Array<Color>>) => {
            arktest.assertEQ(a[0][0], Color.Red)
            arktest.assertEQ(a[0][1], Color.Blue)
            arktest.assertEQ(a.length, 1)
        }
        verify([[Color.Red, Color.Blue]])
    })

    suite.addTest("FixedArray<Color[]>", () => {
        let verify = (a: FixedArray<Color[]>) => {
            arktest.assertEQ(a[0][0], Color.Red)
            arktest.assertEQ(a[0][1], Color.Blue)
            arktest.assertEQ(a.length, 1)
        }
        verify([[Color.Red, Color.Blue]])
    })

    suite.addTest("Array<FixedArray<Color>>", () => {
        let verify = (a: Array<FixedArray<Color>>) => {
            arktest.assertEQ(a[0][0], Color.Red)
            arktest.assertEQ(a[0][1], Color.Blue)
            arktest.assertEQ(a.length, 1)
        }
        verify([[Color.Red, Color.Blue]])
    })

    suite.addTest("FixedArray<Color>[]", () => {
        let verify = (a: FixedArray<Color>[]) => {
            arktest.assertEQ(a[0][0], Color.Red)
            arktest.assertEQ(a[0][1], Color.Blue)
            arktest.assertEQ(a.length, 1)
        }
        verify([[Color.Red, Color.Blue]])
    })

    suite.addTest("FixedArray<Array<F>>", () => {
        let verify = (a: FixedArray<Array<F>>) => {
            arktest.assertEQ(a[0][0](Color.Red), 0)
            arktest.assertEQ(a[1][0](Color.Red), 1)
            arktest.assertEQ(a[2][0](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]])
    })

    suite.addTest("FixedArray<F[]>", () => {
        let verify = (a: FixedArray<F[]>) => {
            arktest.assertEQ(a[0][0](Color.Red), 0)
            arktest.assertEQ(a[1][0](Color.Red), 1)
            arktest.assertEQ(a[2][0](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]])
    })

    suite.addTest("Array<FixedArray<F>>", () => {
        let verify = (a: Array<FixedArray<F>>) => {
            arktest.assertEQ(a[0][0](Color.Red), 0)
            arktest.assertEQ(a[1][0](Color.Red), 1)
            arktest.assertEQ(a[2][0](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]])
    })

    suite.addTest("FixedArray<F>[]", () => {
        let verify = (a: FixedArray<F>[]) => {
            arktest.assertEQ(a[0][0](Color.Red), 0)
            arktest.assertEQ(a[1][0](Color.Red), 1)
            arktest.assertEQ(a[2][0](Color.Blue), 4)
            arktest.assertEQ(a.length, 3)
        }
        verify([[(p: Color): int => p.valueOf()], [(p: Color): int => p.valueOf() + 1], [(p: Color): int => p.valueOf() + 2]])
    })

    suite.addTest("FixedArray<Array<boolean|int|string|null>>", () => {
        let verify = (a: FixedArray<Array<boolean|int|string|null>>) => {
            arktest.assertEQ(a[0][0], true)
            arktest.assertEQ(a[0][1], 1)
            arktest.assertEQ(a[0][2], 'a')
            arktest.assertEQ(a[0][3], null)
            arktest.assertEQ(a.length, 1)
        }
        verify([[true, 1, 'a', null]])
    })

    suite.addTest("FixedArray<(boolean|int|string|null)[]>", () => {
        let verify = (a: FixedArray<(boolean|int|string|null)[]>) => {
            arktest.assertEQ(a[0][0], true)
            arktest.assertEQ(a[0][1], 1)
            arktest.assertEQ(a[0][2], 'a')
            arktest.assertEQ(a[0][3], null)
            arktest.assertEQ(a.length, 1)
        }
        verify([[true, 1, 'a', null]])
    })

    suite.addTest("Array<FixedArray<boolean|int|string|null>>", () => {
        let verify = (a: Array<FixedArray<boolean|int|string|null>>) => {
            arktest.assertEQ(a[0][0], true)
            arktest.assertEQ(a[0][1], 1)
            arktest.assertEQ(a[0][2], 'a')
            arktest.assertEQ(a[0][3], null)
            arktest.assertEQ(a.length, 1)
        }
        verify([[true, 1, 'a', null]])
    })

    suite.addTest("FixedArray<boolean|int|string|null>[]", () => {
        let verify = (a: FixedArray<boolean|int|string|null>[]) => {
            arktest.assertEQ(a[0][0], true)
            arktest.assertEQ(a[0][1], 1)
            arktest.assertEQ(a[0][2], 'a')
            arktest.assertEQ(a[0][3], null)
            arktest.assertEQ(a.length, 1)
        }
        verify([[true, 1, 'a', null]])
    })

    suite.addTest("FixedArray<Array<Number|Error|((p: string)=>string)|Boolean>>", () => {
        let e = new Error
        let verify = (a: FixedArray<Array<Number|Error|((p: string)=>string)|Boolean>>) => {
            arktest.assertEQ(a[0][0] as Error, e)
            arktest.assertEQ(a[1][0] as Boolean, false)
            arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
            arktest.assertEQ(a[3][0] as Number, 42)
            arktest.assertEQ(a.length, 4)
        }
        verify([[e], [false], [(p: string): string => p + p], [42]])
    })

    suite.addTest("FixedArray<(Number|Error|((p: string)=>string)|Boolean)[]>", () => {
        let e = new Error
        let verify = (a: FixedArray<(Number|Error|((p: string)=>string)|Boolean)[]>) => {
            arktest.assertEQ(a[0][0] as Error, e)
            arktest.assertEQ(a[1][0] as Boolean, false)
            arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
            arktest.assertEQ(a[3][0] as Number, 42)
            arktest.assertEQ(a.length, 4)
        }
        verify([[e], [false], [(p: string): string => p + p], [42]])
    })

    suite.addTest("Array<FixedArray<Number|Error|((p: string)=>string)|Boolean>>", () => {
        let e = new Error
        let verify = (a: Array<FixedArray<Number|Error|((p: string)=>string)|Boolean>>) => {
            arktest.assertEQ(a[0][0] as Error, e)
            arktest.assertEQ(a[1][0] as Boolean, false)
            arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
            arktest.assertEQ(a[3][0] as Number, 42)
            arktest.assertEQ(a.length, 4)
        }
        verify([[e], [false], [(p: string): string => p + p], [42]])
    })

    suite.addTest("FixedArray<Number|Error|((p: string)=>string)|Boolean>[]", () => {
        let e = new Error
        let verify = (a: FixedArray<Number|Error|((p: string)=>string)|Boolean>[]) => {
            arktest.assertEQ(a[0][0] as Error, e)
            arktest.assertEQ(a[1][0] as Boolean, false)
            arktest.assertEQ((a[2][0] as ((p: string) => string))('a'), 'aa')
            arktest.assertEQ(a[3][0] as Number, 42)
            arktest.assertEQ(a.length, 4)
        }
        verify([[e], [false], [(p: string): string => p + p], [42]])
    })

    suite.addTest("FixedArray<Number>|FixedArray<String>|Array<Boolean>|null as Array<boolean>", () => {
        let verify = (a: FixedArray<Number>|FixedArray<String>|Array<Boolean>|null) => {
            let x = a as Array<boolean>
            arktest.assertEQ(x[0], false)
            arktest.assertEQ(x[1], true)
            arktest.assertEQ(x[2], false)
            arktest.assertEQ(x.length, 3)
        }
        verify([false, true, false])
    })

    suite.addTest("FixedArray<Number>|FixedArray<String>|Boolean[]|null as boolean[]", () => {
        let verify = (a: FixedArray<Number>|FixedArray<String>|Boolean[]|null) => {
            let x = a as boolean[]
            arktest.assertEQ(x[0], false)
            arktest.assertEQ(x[1], true)
            arktest.assertEQ(x[2], false)
            arktest.assertEQ(x.length, 3)
        }
        verify([false, true, false])
    })

    suite.addTest("Array<number>|FixedArray<String>|FixedArray<Boolean>|null as number[]", () => {
        let verify = (a: Array<number>|FixedArray<String>|FixedArray<Boolean>|null) => {
            let x = a as number[]
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 43)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 43])
    })

    suite.addTest("number[]|FixedArray<String>|FixedArray<Boolean>|null as Array<number>", () => {
        let verify = (a: number[]|FixedArray<String>|FixedArray<Boolean>|null) => {
            let x = a as Array<number>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 43)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 43])
    })

    suite.addTest("Array<boolean>|FixedArray<String>|FixedArray<number>|null as FixedArray<number>", () => {
        let verify = (a: Array<boolean>|FixedArray<String>|FixedArray<number>|null) => {
            let x = a as FixedArray<number>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 43)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 43])
    })

    suite.addTest("FixedArray<number>|FixedArray<String>|Array<Boolean>|null as FixedArray<number>", () => {
        let verify = (a: FixedArray<number>|FixedArray<String>|Array<Boolean>|null) => {
            let x = a as FixedArray<number>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 43)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 43])
    })

    suite.addTest("FixedArray<Number>|Array<string>|FixedArray<Boolean>|null as string[]", () => {
        let s = '4'
        let verify = (a: FixedArray<Number>|Array<string>|FixedArray<Boolean>|null) => {
            let x = a as string[]
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '43')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', s + '3'])
    })

    suite.addTest("FixedArray<Number>|string[]|Array<Boolean>|null as Array<String>", () => {
        let s = '4'
        let verify = (a: FixedArray<Number>|string[]|Array<Boolean>|null) => {
            let x = a as Array<String>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '43')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', s + '3'])
    })

    suite.addTest("FixedArray<string>|FixedArray<Boolean>|null as FixedArray<string>", () => {
        let s = '4'
        let verify = (a: FixedArray<string>|FixedArray<Boolean>|null) => {
            let x = a as FixedArray<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '43')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', s + '3'])
    })

    suite.addTest("FixedArray<boolean>|string|FixedArray<string>|null as FixedArray<string>", () => {
        let s = '4'
        let verify = (a: FixedArray<boolean>|string|FixedArray<string>|null) => {
            let x = a as FixedArray<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '43')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', s + '3'])
    })

    suite.addTest("Array<string>|Array<int> as Array<int>", () => {
        let verify = (a: Array<string>|Array<int>) => {
            let x = a as Array<int>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("Array<string>|FixedArray<int> as FixedArray<int>", () => {
        let verify = (a: Array<string>|FixedArray<int>) => {
            let x = a as FixedArray<int>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("FixedArray<string>|Array<int> as int[]", () => {
        let verify = (a: FixedArray<string>|Array<int>) => {
            let x = a as int[]
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("FixedArray<string>|FixedArray<int> as FixedArray<int>", () => {
        let verify = (a: FixedArray<string>|FixedArray<int>) => {
            let x = a as FixedArray<int>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("Array<int>|Array<string> as Array<int>", () => {
        let verify = (a: Array<int>|Array<string>) => {
            let x = a as Array<int>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("FixedArray<int>|Array<string> as FixedArray<int>", () => {
        let verify = (a: FixedArray<int>|Array<string>) => {
            let x = a as FixedArray<int>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("Array<int>|FixedArray<string> as int[]", () => {
        let verify = (a: Array<int>|FixedArray<string>) => {
            let x = a as int[]
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("FixedArray<int>|FixedArray<string> as FixedArray<int>", () => {
        let verify = (a: FixedArray<int>|FixedArray<string>) => {
            let x = a as FixedArray<int>
            arktest.assertEQ(x[0], 1)
            arktest.assertEQ(x[1], 2)
            arktest.assertEQ(x[2], 3)
            arktest.assertEQ(x.length, 3)
        }
        verify([1, 2, 3])
    })

    suite.addTest("Array<string>|Array<int> as string[]", () => {
        let verify = (a: Array<string>|Array<int>) => {
            let x = a as string[]
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("Array<string>|FixedArray<int> as Array<string>", () => {
        let verify = (a: Array<string>|FixedArray<int>) => {
            let x = a as Array<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("FixedArray<string>|Array<int> as FixedArray<string>", () => {
        let verify = (a: FixedArray<string>|Array<int>) => {
            let x = a as FixedArray<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("FixedArray<string>|FixedArray<int> as FixedArray<string>", () => {
        let verify = (a: FixedArray<string>|FixedArray<int>) => {
            let x = a as FixedArray<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("Array<int>|Array<string> as string[]", () => {
        let verify = (a: Array<int>|Array<string>) => {
            let x = a as string[]
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("FixedArray<int>|Array<string> as Array<string>", () => {
        let verify = (a: FixedArray<int>|Array<string>) => {
            let x = a as Array<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("Array<int>|FixedArray<string> as FixedArray<string>", () => {
        let verify = (a: Array<int>|FixedArray<string>) => {
            let x = a as FixedArray<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    suite.addTest("FixedArray<int>|FixedArray<string> as FixedArray<string>", () => {
        let verify = (a: FixedArray<int>|FixedArray<string>) => {
            let x = a as FixedArray<string>
            arktest.assertEQ(x[0], '1')
            arktest.assertEQ(x[1], '2')
            arktest.assertEQ(x[2], '3')
            arktest.assertEQ(x.length, 3)
        }
        verify(['1', '2', '3'])
    })

    return suite.run()
}
