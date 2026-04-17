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
desc: Array type is correctly resolved from a literal expression for all primitive and reference element types.
---*/

interface MyInterface {}
class MyClass {}

function main(): int {
    let suite = new arktest.ArkTestsuite("array_create_from_lit1")

    suite.addTest("byte[] from literal", () => {
        let instantiateEmptyArray: () => byte[] = (): byte[] => [] as byte[]
        let instantiateArrayWithValues: () => byte[] = (): byte[] => [0, 5, 7, 10] as byte[]
        let a: byte[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: byte[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 4)
        arktest.assertTrue(b[0] instanceof byte)
        arktest.assertEQ(b[0], 0)
        arktest.assertEQ(b[1], 5)
        arktest.assertEQ(b[2], 7)
        arktest.assertEQ(b[3], 10)
    })

    suite.addTest("short[] from literal", () => {
        let instantiateEmptyArray: () => short[] = (): short[] => [] as short[]
        let instantiateArrayWithValues: () => short[] = (): short[] => [9, 15, 13] as short[]
        let a: short[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: short[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 3)
        arktest.assertTrue(b[0] instanceof short)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 15)
        arktest.assertEQ(b[2], 13)
    })

    suite.addTest("int[] from literal", () => {
        let instantiateEmptyArray: () => int[] = (): int[] => [] as int[]
        let instantiateArrayWithValues: () => int[] = (): int[] => [9, 13, 123, 17, 10] as int[]
        let a: int[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: int[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 5)
        arktest.assertTrue(b[0] instanceof int)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 13)
        arktest.assertEQ(b[2], 123)
        arktest.assertEQ(b[3], 17)
        arktest.assertEQ(b[4], 10)
    })

    suite.addTest("number[] from literal", () => {
        let instantiateEmptyArray: () => number[] = (): number[] => [] as number[]
        let instantiateArrayWithValues: () => number[] = (): number[] => [9, 13, 1, 5.7, 10, 12.4] as number[]
        let a: number[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: number[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof number)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 13)
        arktest.assertEQ(b[2], 1)
        arktest.assertEQ(b[3], 5.7)
        arktest.assertEQ(b[4], 10)
        arktest.assertEQ(b[5], 12.4)
    })

    suite.addTest("double[] from literal", () => {
        let instantiateEmptyArray: () => double[] = (): double[] => [] as double[]
        let instantiateArrayWithValues: () => double[] = (): double[] => [9, 13, 1, 5.7, 10, 12.4] as double[]
        let a: double[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: double[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof double)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 13)
        arktest.assertEQ(b[2], 1)
        arktest.assertEQ(b[3], 5.7)
        arktest.assertEQ(b[4], 10)
        arktest.assertEQ(b[5], 12.4)
    })

    suite.addTest("long[] from literal", () => {
        let instantiateEmptyArray: () => long[] = (): long[] => [] as long[]
        let instantiateArrayWithValues: () => long[] = (): long[] => [9, 13, 9223372036854775807, 5, 10, 12] as long[]
        let a: long[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: long[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof long)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 13)
        arktest.assertEQ(b[2], 9223372036854775807)
        arktest.assertEQ(b[3], 5)
        arktest.assertEQ(b[4], 10)
        arktest.assertEQ(b[5], 12)
    })

    suite.addTest("float[] from literal", () => {
        let instantiateEmptyArray: () => float[] = (): float[] => [] as float[]
        let instantiateArrayWithValues: () => float[] = (): float[] => [9, 13, 1, 5.7f, 10, 12.4f] as float[]
        let a: float[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: float[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof float)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 13)
        arktest.assertEQ(b[2], 1)
        arktest.assertEQ(b[3], 5.7f)
        arktest.assertEQ(b[4], 10)
        arktest.assertEQ(b[5], 12.4f)
    })

    suite.addTest("boolean[] from literal", () => {
        let instantiateEmptyArray: () => boolean[] = (): boolean[] => [] as boolean[]
        let instantiateArrayWithValues: () => boolean[] = (): boolean[] => [true, false, false, true, false, true] as boolean[]
        let a: boolean[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: boolean[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof boolean)
        arktest.assertEQ(b[0], true)
        arktest.assertEQ(b[1], false)
        arktest.assertEQ(b[2], false)
        arktest.assertEQ(b[3], true)
        arktest.assertEQ(b[4], false)
        arktest.assertEQ(b[5], true)
    })

    suite.addTest("char[] from literal", () => {
        let instantiateEmptyArray: () => char[] = (): char[] => [] as char[]
        let instantiateArrayWithValues: () => char[] = (): char[] => [c'\u2028', c'\u005c', c'\u0027', c'x', c'Z'] as char[]
        let a: char[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: char[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 5)
        arktest.assertTrue(b[0] instanceof char)
        arktest.assertEQ(b[0], c'\u2028')
        arktest.assertEQ(b[1], c'\u005c')
        arktest.assertEQ(b[2], c'\u0027')
        arktest.assertEQ(b[3], c'x')
        arktest.assertEQ(b[4], c'Z')
    })

    suite.addTest("String[] from literal", () => {
        let instantiateEmptyArray: () => String[] = (): String[] => [] as String[]
        let instantiateArrayWithValues: () => String[] = (): String[] => ["ab", "cd", "efg"] as String[]
        let a: String[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: String[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 3)
        arktest.assertTrue(b[0] instanceof String)
        arktest.assertEQ(b[0], "ab")
        arktest.assertEQ(b[1], "cd")
        arktest.assertEQ(b[2], "efg")
    })

    suite.addTest("object[] from literal", () => {
        let instantiateEmptyArray: () => object[] = (): object[] => [] as object[]
        let instantiateArrayWithValues: () => object[] = (): object[] => [new Object, new Object, new Object] as object[]
        let a: object[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: object[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 3)
        arktest.assertTrue(b[0] instanceof object)
    })

    suite.addTest("Number[] from literal", () => {
        let instantiateEmptyArray: () => Number[] = (): Number[] => [] as Number[]
        let instantiateArrayWithValues: () => Number[] = (): Number[] => [9, 13, 1, 5.7, 10, 12.4] as Number[]
        let a: Number[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: Number[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof Number)
        arktest.assertEQ(b[0], 9)
        arktest.assertEQ(b[1], 13)
        arktest.assertEQ(b[2], 1)
        arktest.assertEQ(b[3], 5.7)
        arktest.assertEQ(b[4], 10)
        arktest.assertEQ(b[5], 12.4)
    })

    suite.addTest("Boolean[] from literal", () => {
        let instantiateEmptyArray: () => Boolean[] = (): Boolean[] => [] as Boolean[]
        let instantiateArrayWithValues: () => Boolean[] = (): Boolean[] => [true, false, false, true, false, true] as Boolean[]
        let a: Boolean[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: Boolean[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 6)
        arktest.assertTrue(b[0] instanceof Boolean)
        arktest.assertEQ(b[0], true)
        arktest.assertEQ(b[1], false)
        arktest.assertEQ(b[2], false)
        arktest.assertEQ(b[3], true)
        arktest.assertEQ(b[4], false)
        arktest.assertEQ(b[5], true)
    })

    suite.addTest("MyClass[] from literal", () => {
        let instantiateEmptyArray: () => MyClass[] = (): MyClass[] => [] as MyClass[]
        let instantiateArrayWithValues: () => MyClass[] = (): MyClass[] => [new MyClass, new MyClass, new MyClass] as MyClass[]
        let a: MyClass[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: MyClass[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 3)
        arktest.assertTrue(b[0] instanceof MyClass)
    })

    suite.addTest("MyInterface[] from literal", () => {
        let instantiateEmptyArray: () => MyInterface[] = (): MyInterface[] => [] as MyInterface[]
        let instantiateArrayWithValues: () => MyInterface[] = (): MyInterface[] => [{}, {}, {}] as MyInterface[]
        let a: MyInterface[] = instantiateEmptyArray()
        arktest.assertEQ(a.length, 0)

        let b: MyInterface[] = instantiateArrayWithValues()
        arktest.assertEQ(b.length, 3)
        arktest.assertTrue(b[0] instanceof MyInterface)
    })

    return suite.run()
}
