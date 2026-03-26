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
desc: array indexing, function call, and new expression precedence with other operators
---*/

class Counter {
    value: int
    constructor(v: int) {
        this.value = v
    }
}

function five(): int {
    return 5
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Access and call operator precedence")

    suite.addTest("array indexing has higher precedence than unary minus", () => {
        let arr: int[] = [10, 20, 30]
        arktest.assertEQ(-arr[1], -20)
    })

    suite.addTest("array indexing has higher precedence than postfix increment", () => {
        let arr: int[] = [10, 20, 30]
        let old = arr[0]++
        arktest.assertEQ(old, 10)
        arktest.assertEQ(arr[0], 11)
    })

    suite.addTest("array indexing has higher precedence than postfix decrement", () => {
        let arr: int[] = [10, 20, 30]
        let old = arr[2]--
        arktest.assertEQ(old, 30)
        arktest.assertEQ(arr[2], 29)
    })

    suite.addTest("function call has higher precedence than unary minus", () => {
        arktest.assertEQ(-five(), -5)
    })

    suite.addTest("new expression has higher precedence than member access chain", () => {
        arktest.assertEQ(new Counter(15).value, 15)
    })

    suite.addTest("array indexing combined with arithmetic", () => {
        let arr: int[] = [2, 3, 4]
        arktest.assertEQ(arr[0] * arr[1] + arr[2], 10)
    })

    suite.addTest("nested array indexing", () => {
        let matrix: int[][] = [[1, 2], [3, 4]]
        arktest.assertEQ(matrix[1][0], 3)
    })

    return suite.run()
}
