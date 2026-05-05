/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
  Spec examples for array literal type inference from context: variable
  declaration, assignment, call, return, casting, fixed array, readonly,
  union, and tuple contexts.
---*/

function min(x: number[]): number {
    let m = x[0]
    for (let v of x)
        if (v < m)
            m = v
    return m
}

type Matrix = number[][]

function takeNumberArray(array: Number[]) {
    arktest.assertEQ(array[0], 1.0)
    arktest.assertEQ(array[1], 2.0)
    arktest.assertEQ(array[2], 3.0)
}

function takeNumberBooleanTuple(tuple: [Number, Boolean]) {
    arktest.assertEQ(tuple[0], 1.0)
    arktest.assertEQ(tuple[1], true)
}

function takeReadonlyTuple(tuple: readonly [Number, Boolean]) {
    arktest.assertEQ(tuple[0], 1.0)
    arktest.assertEQ(tuple[1], true)
}

function takeReadonlyNumberArray(ar: readonly number[]) {
    arktest.assertEQ(ar[0], 1.0)
}

class AClass {}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spec examples for array literal type inference from context")

    suite.addTest("variable declaration, assignment, call, matrix, Object[]", () => {
        let a: number[] = [1, 2, 3]
        a = [4, 5]
        arktest.assertEQ(min([1., 3.14, 0.99]), 0.99)

        let m: Matrix = [[1, 2], [3, 4], [5, 6]]
        let value: number = 2
        let list: Object[] = [1, value, "hello", new Error()]
    })

    suite.addTest("[number, string] tuple", () => {
        let tuple: [number, string] = [1, "hello"]
        arktest.assertTrue(tuple[0] == 1 && tuple[1] == "hello")
    })

    suite.addTest("Number[] in assignment, call, casting", () => {
        let array: Number[] = [1, 2, 3]
        arktest.assertEQ(array[0], 1.0)
        arktest.assertEQ(array[1], 2.0)
        arktest.assertEQ(array[2], 3.0)

        takeNumberArray([1, 2, 3])

        let n: Number[] = [1, 2, 3] as Number[]
        arktest.assertEQ(n[0], 1.0)
        arktest.assertEQ(n[1], 2.0)
        arktest.assertEQ(n[2], 3.0)
    })

    suite.addTest("[Number, Boolean] tuple and readonly contexts", () => {
        let tuple: [Number, Boolean] = [1, true]
        arktest.assertEQ(tuple[0], 1.0)
        arktest.assertEQ(tuple[1], true)

        takeNumberBooleanTuple([1, true])

        let t: [Number, Boolean] = [1, true] as [Number, Boolean]
        arktest.assertEQ(t[0], 1.0)
        arktest.assertEQ(t[1], true)

        let readonly_tuple: readonly [number, boolean] = [1, true]
        takeReadonlyTuple(readonly_tuple)

        let readonly_array: readonly number[] = [1, 2, 3]
        takeReadonlyNumberArray(readonly_array)
        takeReadonlyNumberArray([1.0, 2.0])
    })

    suite.addTest("union of arrays resolves to single matching type", () => {
        let union_of_arrays: number[] | string[] = [1.0, 2.0]
        arktest.assertEQ((union_of_arrays as number[])[0], 1.0)
    })

    suite.addTest("FixedArray<number> from literal", () => {
        let farray: FixedArray<number> = [1, 2]
        arktest.assertEQ(farray[0], 1.0)
        arktest.assertEQ(farray[1], 2.0)
    })

    suite.addTest("Array<AClass>, Array<Number>, FixedArray<number>", () => {
        let b1: Array<AClass> = [new AClass, new AClass]
        let b2: Array<Number> = [1, 2, 3]
        let b3: FixedArray<number> = [1, 2]

        arktest.assertTrue(b1[0] instanceof AClass)
        arktest.assertEQ(b2[2], 3)
        arktest.assertEQ(b3[1], 2)
    })

    return suite.run()
}
