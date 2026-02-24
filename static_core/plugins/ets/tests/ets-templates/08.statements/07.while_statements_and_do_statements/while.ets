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
  A while statement evaluates an expression and executes the statement repeatedly
  till the expression value is true. whileStatement starts from evaluating and
  checking the expression value.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("while Statements")

    suite.addTest("Basic counting", () => {
        let i = 1
        while (i < 10) i++
        arktest.assertEQ(i, 10)
    })

    suite.addTest("Expression in condition with side effect", () => {
        let i = 1
        let res = ""
        while ((i += 2) < 7) res += "L"
        arktest.assertEQ(res, "LL")
    })

    suite.addTest("Pre-increment in condition, empty block", () => {
        let i = 1
        while (++i < 10) {}
        arktest.assertEQ(i, 10)
    })

    suite.addTest("Nested while statements", () => {
        let i = 1
        let j = 1
        while (++i < 7) while (++i < 5) while (++i < 3) j = i
        arktest.assertEQ(j, 1)
        arktest.assertEQ(i, 8)
    })

    suite.addTest("With nested do-while", () => {
        let i = 1
        let j = 1
        while (i < 7) do j = ++i; while (i < 5)
        arktest.assertEQ(j, 7)
    })

    suite.addTest("Empty statement body (semicolon)", () => {
        let i = 1
        while(++i < 10); { i-- }
        arktest.assertEQ(i, 9)
    })

    suite.addTest("Numeric expression as condition (addition)", () => {
        let i = 1
        let j = 1
        while (i+j) { i-- }
        arktest.assertEQ(i, -1)
    })

    suite.addTest("Numeric expression as condition (bitwise and)", () => {
        let i = 1
        let j = 1
        while (i & j) {i--}
        arktest.assertEQ(i, 0)
    })

    suite.addTest("False condition never executes body", () => {
        let count = 0
        while (false) count++
        arktest.assertEQ(count, 0)
    })

    suite.addTest("Break exits loop", () => {
        let i = 0
        while (true) {
            if (i >= 3) break
            i++
        }
        arktest.assertEQ(i, 3)
    })

    suite.addTest("Continue skips rest of iteration", () => {
        let sum = 0
        let i = 0
        while (i < 6) {
            i++
            if (i == 3) continue
            sum += i
        }
        arktest.assertEQ(sum, 18)
    })

    return suite.run()
}
