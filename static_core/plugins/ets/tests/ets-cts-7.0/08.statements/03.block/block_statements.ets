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
  A sequence of statements enclosed in balanced braces forms a block.
  Block statements are executed one after another in textual order,
  except type declarations, until an error is thrown or a return occurs.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Block")

    suite.addTest("Block with numeric expression", () => {
        { 5; }
    })

    suite.addTest("Block with arithmetic expression", () => {
        { 1 + 2; }
    })

    suite.addTest("Block with variable declaration (no init)", () => {
        { let a: int; }
    })

    suite.addTest("Block with variable declaration (with init)", () => {
        { let b = 1; }
    })

    suite.addTest("Statements in block execute in textual order", () => {
        let result = ""
        {
            result += "A"
            result += "B"
            result += "C"
        }
        arktest.assertEQ(result, "ABC")
    })

    suite.addTest("Empty block is valid", () => {
        {}
    })

    suite.addTest("Nested blocks", () => {
        let x = 1
        {
            x += 1
            {
                x += 1
                {
                    x += 1
                }
            }
        }
        arktest.assertEQ(x, 4)
    })

    suite.addTest("Variable declared in block is scoped to block", () => {
        let x = 10
        {
            let x = 20
            arktest.assertEQ(x, 20)
        }
        arktest.assertEQ(x, 10)
    })

    suite.addTest("Block with declarations and expressions", () => {
        let result = 0
        {
            let a = 3
            let b = 7
            result = a + b
        }
        arktest.assertEQ(result, 10)
    })

    suite.addTest("Block execution stops on error", () => {
        let result = ""
        try {
            {
                result += "A"
                if (true) throw new Error()
                result += "B"
            }
        } catch (e) {
            result += "C"
        }
        arktest.assertEQ(result, "AC")
    })

    suite.addTest("Variable shadowing in nested blocks", () => {
        let item: number = 123
        {
            const item: string = "123"
            arktest.assertEQ(item, "123")
        }
        arktest.assertEQ(item, 123)
    })

    return suite.run()
}
