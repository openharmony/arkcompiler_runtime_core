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
  If type used in a context is a value array type, and type of each expression
  is assignable to an array element type, then an array literal is of the
  specified type. Array literal type inferred from declaration context.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("ValueArray inference from declaration context")

    suite.addTest("ValueArray<int> from int literals", () => {
        let a: ValueArray<int> = [1, 2, 3]
        arktest.assertEQ(a[0], 1)
        arktest.assertEQ(a[1], 2)
        arktest.assertEQ(a[2], 3)
        arktest.assertEQ(a.length, 3)
    })

    suite.addTest("ValueArray<double> from numeric literals with widening", () => {
        let a: ValueArray<double> = [1, 2]
        arktest.assertEQ(a[0], 1.0)
        arktest.assertEQ(a[1], 2.0)
        arktest.assertEQ(a.length, 2)
    })

    suite.addTest("ValueArray<double> from double literals", () => {
        let a: ValueArray<double> = [3.14, 2.71]
        arktest.assertEQ(a[0], 3.14)
        arktest.assertEQ(a[1], 2.71)
    })

    suite.addTest("ValueArray<boolean> from boolean literals", () => {
        let a: ValueArray<boolean> = [true, false, true]
        arktest.assertEQ(a[0], true)
        arktest.assertEQ(a[1], false)
        arktest.assertEQ(a[2], true)
    })

    suite.addTest("ValueArray<byte> from small int literals", () => {
        let a: ValueArray<byte> = [1, 2, 3]
        arktest.assertEQ(a[0], (1).toByte())
        arktest.assertEQ(a[1], (2).toByte())
        arktest.assertEQ(a[2], (3).toByte())
    })

    suite.addTest("ValueArray<short> from int literals", () => {
        let a: ValueArray<short> = [10, 20, 30]
        arktest.assertEQ(a[0], (10).toShort())
        arktest.assertEQ(a[1], (20).toShort())
        arktest.assertEQ(a[2], (30).toShort())
    })

    suite.addTest("ValueArray<long> from int literals", () => {
        let a: ValueArray<long> = [100, 200, 300]
        arktest.assertEQ(a[0], (100).toLong())
        arktest.assertEQ(a[1], (200).toLong())
        arktest.assertEQ(a[2], (300).toLong())
    })

    suite.addTest("ValueArray<float> from numeric literals", () => {
        let a: ValueArray<float> = [1, 2, 3]
        arktest.assertEQ(a[0], (1).toFloat())
        arktest.assertEQ(a[1], (2).toFloat())
        arktest.assertEQ(a[2], (3).toFloat())
    })

    suite.addTest("ValueArray<char> from char literals", () => {
        let a: ValueArray<char> = [c'a', c'b', c'c']
        arktest.assertEQ(a[0], c'a')
        arktest.assertEQ(a[1], c'b')
        arktest.assertEQ(a[2], c'c')
    })

    return suite.run()
}
