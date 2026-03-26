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
  If a type used in a context is a union type, then Array Literal Type
  Inference from Context is taken repetitively trying to use each type of
  the union type as the context. If only a single type is inferred, then
  this single type is used as the type of the literal.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Array literal type inference from union context")

    suite.addTest("union with single matching type: [int, int]", () => {
        let union_1: string[] | [int, int] = [1, 2]
        // instanceof [int, int] is erased per 15.12; verify via length and elements
        arktest.assertEQ((union_1 as [int, int]).length, 2)
        arktest.assertEQ((union_1 as [int, int])[0], 1)
        arktest.assertEQ((union_1 as [int, int])[1], 2)
    })

    suite.addTest("union with single matching resizable array type", () => {
        let union_4: (number | string)[] | [(number | string), number, boolean] | string = [5, 5, 5]
        let arr = union_4 as (number | string)[]
        arktest.assertEQ(arr.length, 3)
        arktest.assertEQ(arr[0], 5)
    })

    suite.addTest("union with single matching array type", () => {
        let union_5: (number | string)[] | [(number | string), number, boolean] | string = [5, 5]
        // instanceof (number | string)[] is erased to Array per 15.12; verify via elements
        let arr = union_5 as (number | string)[]
        arktest.assertEQ(arr.length, 2)
        arktest.assertEQ(arr[0], 5)
        arktest.assertEQ(arr[1], 5)
    })

    return suite.run()
}
