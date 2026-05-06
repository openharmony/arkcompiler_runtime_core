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
desc: Spread expression always copies values from original arrays. A callee changes elements of its own copy, but not elements of arrays used in the call.
---*/

function fooIntPModify(...p: int[]) {
    p[1] = 4
}

function fooNumberMultiMod(...p: number[]) {
    p[0] = 99.0
    p[2] = 88.0
}

function fooStringReturn(...p: string[]): string[] {
    p[1] = "modified"
    return p
}

function fooByteArr(...p: byte[]) {
    p[0] = (99).toByte()
}

function fooLongArr(...p: long[]) {
    p[1] = (777).toLong()
}

function fooCharArr(...p: char[]) {
    p[0] = c'Z'
}

function fooFixedArrInt(...p: FixedArray<int>) {
    p[0] = 999
}

function fooFixedArrWithK(k: int, ...p: FixedArray<int>) {
    for (let i = 0; i < p.length; i++) {
        p[i] = p[i] + k
    }
    k = k + 1
}

function fooTupleArr(...p: [int, int][]) {
    p[0][0] = 999
}

function fooIntPush(...p: int[]) {
    p.push(999)
}

function fooIntMul(...p: int[]) {
    for (let i = 0; i < p.length; i++) {
        p[i] = p[i] * 10
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Spread expression copies values from original arrays")

    suite.addTest("Callee modifies its copy, original array unchanged", () => {
        let a: int[] = [1, 2, 3]
        fooIntPModify(...a)
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 3)
    })

    suite.addTest("Multiple element modifications don't affect original", () => {
        let a: number[] = [1.0, 2.0, 3.0]
        fooNumberMultiMod(...a)
        arktest.assertEQ(a[0], 1.0)
        arktest.assertEQ(a[1], 2.0)
        arktest.assertEQ(a[2], 3.0)
    })

    suite.addTest("Function returns modified copy, original unchanged", () => {
        let a: string[] = ["a", "b", "c"]
        let result = fooStringReturn(...a)
        arktest.assertEQ(result[1], "modified")
        arktest.assertEQ(a[1], "b")
    })

    suite.addTest("Array literal spread creates new array", () => {
        let original: int[] = [1, 2, 3]
        let copy = [...original]
        copy[0] = 999
        arktest.assertEQ(original[0], 1)
        arktest.assertEQ(copy[0], 999)
    })

    suite.addTest("Multiple spreads create independent arrays", () => {
        let a: int[] = [1, 2]
        let b = [...a]
        let c = [...a]
        b[0] = 10
        c[0] = 20
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(b[0], 10)
        arktest.assertEQ(c[0], 20)
    })

    suite.addTest("Value type array spread creates copy", () => {
        let a: byte[] = [(1).toByte(), (2).toByte(), (3).toByte()]
        fooByteArr(...a)
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 3)
    })

    suite.addTest("Long array spread creates copy", () => {
        let a: long[] = [(100).toLong(), (200).toLong(), (300).toLong()]
        fooLongArr(...a)
        arktest.assertEQ(a[0], 100)
        arktest.assertEQ(a[1], 200)
        arktest.assertEQ(a[2], 300)
    })

    suite.addTest("Char array spread creates copy", () => {
        let a: char[] = [c'a', c'b', c'c']
        fooCharArr(...a)
        arktest.assertEQ(a[0], c'a')
        arktest.assertEQ(a[1], c'b')
        arktest.assertEQ(a[2], c'c')
    })

    suite.addTest("FixedArray spread creates copy", () => {
        let a: FixedArray<int> = [1, 2, 3]
        fooFixedArrInt(...a)
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 3)
    })

    suite.addTest("FixedArray literal spread creates copy", () => {
        let original: FixedArray<number> = [1.0, 2.0, 3.0]
        let copy: number[] = [...original]
        copy[1] = 999.0
        arktest.assertEQ(original[1], 2.0)
        arktest.assertEQ(copy[1], 999.0)
    })

    suite.addTest("FixedArray spread with extra param", () => {
        let a: FixedArray<int> = [1, 2, 3]
        let k: int = 10
        fooFixedArrWithK(k, ...a)
        fooFixedArrWithK(k, ...[...a])
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 3)
        arktest.assertEQ(k, 10)
    })

    suite.addTest("Tuple array spread - outer array is copied, inner tuples are references", () => {
        let a: [int, int][] = [[1, 2], [3, 4]]
        fooTupleArr(...a)
        arktest.assertEQ(a[0][0], 999)
        arktest.assertEQ(a[0][1], 2)
    })

    suite.addTest("Push to spread copy doesn't affect original", () => {
        let a: int[] = [1, 2, 3]
        fooIntPush(...a)
        arktest.assertEQ(a.length, 3)
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[2], 3)
    })

    suite.addTest("Array literal spread, then push", () => {
        let original: string[] = ["a", "b"]
        let copy = [...original]
        copy.push("c")
        arktest.assertEQ(original.length, 2)
        arktest.assertEQ(copy.length, 3)
        arktest.assertEQ(copy[2], "c")
    })

    suite.addTest("All elements modified in copy, original unchanged", () => {
        let a: int[] = [1, 2, 3, 4, 5]
        fooIntMul(...a)
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[2], 3)
        arktest.assertEQ(a[4], 5)
    })

    return suite.run()
}
