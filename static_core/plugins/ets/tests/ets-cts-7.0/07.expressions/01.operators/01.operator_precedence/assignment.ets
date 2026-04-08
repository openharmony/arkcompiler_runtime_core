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
desc: assignment operator precedence and right-to-left associativity
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Assignment operator precedence")

    suite.addTest("assignment has lower precedence than ternary", () => {
        let a: int = 0
        a = true ? 10 : 20
        arktest.assertEQ(a, 10)
    })

    suite.addTest("assignment has lower precedence than logical OR", () => {
        let a: boolean = false
        a = true || false
        arktest.assertTrue(a)
    })

    suite.addTest("assignment has lower precedence than logical AND", () => {
        let a: boolean = true
        a = false && true
        arktest.assertFalse(a)
    })

    suite.addTest("assignment is right-to-left associative", () => {
        let a: int = 0
        let b: int = 0
        let c: int = 0
        a = b = c = 5
        arktest.assertEQ(a, 5)
        arktest.assertEQ(b, 5)
        arktest.assertEQ(c, 5)
    })

    suite.addTest("compound assignment has lower precedence than arithmetic", () => {
        let a: int = 10
        a += 3 * 2
        arktest.assertEQ(a, 16)
    })

    suite.addTest("compound assignment has lower precedence than shift", () => {
        let a: int = 3
        a += 1 << 2
        arktest.assertEQ(a, 7)
    })

    suite.addTest("compound assignment has lower precedence than relational via ternary", () => {
        let a: int = 0
        a = 3 > 2 ? 100 : 200
        arktest.assertEQ(a, 100)
    })

    return suite.run()
}
