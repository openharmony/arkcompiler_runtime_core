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
desc: postfix ensure-not-nullish operator precedence with other operators
---*/

class NullishBox {
    value: number
    constructor(v: number) {
        this.value = v
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Postfix ensure-not-nullish operator precedence")

    suite.addTest("postfix ! has higher precedence than prefix logical NOT", () => {
        let x: boolean | null = true
        let result: boolean = !x!
        arktest.assertEQ(result, false)
    })

    suite.addTest("postfix ! has higher precedence than unary minus", () => {
        let x: number | null = 5.0
        let result: number = -x!
        arktest.assertEQ(result, -5.0)
    })

    suite.addTest("postfix ! has higher precedence than bitwise complement", () => {
        let x: int | null = 0
        let result: int = ~x!
        arktest.assertEQ(result, -1)
    })

    suite.addTest("postfix ! with member access on nullable object", () => {
        let b: NullishBox | null = new NullishBox(7.0)
        arktest.assertEQ(b!.value, 7.0)
    })

    suite.addTest("postfix ! with indexing on nullable array", () => {
        let arr: number[] | null = [10.0, 20.0, 30.0]
        arktest.assertEQ(arr![1], 20.0)
    })

    suite.addTest("postfix ! combined with arithmetic", () => {
        let x: number | null = 8.0
        let y: number | null = 3.0
        arktest.assertEQ(x! + y!, 11.0)
        arktest.assertEQ(x! * y! - 1, 23.0)
    })

    return suite.run()
}
