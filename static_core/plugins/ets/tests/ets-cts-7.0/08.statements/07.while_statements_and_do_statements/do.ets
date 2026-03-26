/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
  A do statement evaluates an expression and executes the statement repeatedly
  till the expression value is true. doStatement starts from executing the
  statement.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("do Statements")

    suite.addTest("Basic counting", () => {
        let i = 1
        do i++; while (i < 10)
        arktest.assertEQ(i, 10)
    })

    suite.addTest("Expression in condition with side effect", () => {
        let i = 1
        let res = ""
        do res += "L"; while ((i += 2) < 7)
        arktest.assertEQ(res, "LLL")
    })

    suite.addTest("Pre-increment in condition, empty block", () => {
        let i = 1
        do {} while (++i < 10)
        arktest.assertEQ(i, 10)
    })

    suite.addTest("Triple nested do-while", () => {
        let i = 1
        do do do i += 1; while (i < 3) while (i < 5) while (i < 7)
        arktest.assertEQ(i, 7)
    })

    suite.addTest("With nested while inside body", () => {
        let i = 1
        do { while (i < 7) i += 1 } while (i < 5)
        arktest.assertEQ(i, 7)
    })

    suite.addTest("Variable shadowing inside body", () => {
        let i = 1
        let j = 1
        do {
            let i: int = j;
            j = i * 2;
            i = j;
        } while (j < 3)
        arktest.assertEQ(i, 1)
        arktest.assertEQ(j, 4)
    })

    suite.addTest("Numeric expression as condition (addition)", () => {
        let i = 1
        let j = 1
        do {i--} while (i+j)
        arktest.assertEQ(i, -1)
    })

    suite.addTest("Numeric expression as condition (bitwise and)", () => {
        let i = 1
        let j = 1
        do {i--} while (i & j)
        arktest.assertEQ(i, 0)
    })

    suite.addTest("Always executes body at least once even when condition is false", () => {
        let count = 0
        do count++; while (false)
        arktest.assertEQ(count, 1)
    })

    suite.addTest("Break exits loop", () => {
        let i = 0
        do {
            if (i >= 3) break
            i++
        } while (true)
        arktest.assertEQ(i, 3)
    })

    suite.addTest("Continue skips rest of iteration", () => {
        let sum = 0
        let i = 0
        do {
            i++
            if (i == 3) continue
            sum += i
        } while (i < 6)
        arktest.assertEQ(sum, 18)
    })

    return suite.run()
}
