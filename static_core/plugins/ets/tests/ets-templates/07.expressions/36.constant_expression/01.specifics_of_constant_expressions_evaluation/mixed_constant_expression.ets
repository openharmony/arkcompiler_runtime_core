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
    In case of mixed constant expression, each numeric subexpression is evaluated
    independently according to the type promotion rules. Numeric subexpressions
    within logical operators are evaluated separately.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Mixed constant expression evaluation")

    suite.addTest("spec example: const c = 3; c > 1 && c*2 < 8", () => {
        const c = 3
        let r: boolean = c > 1 && c * 2 < 8
        arktest.assertTrue(r)
    })

    suite.addTest("mixed: numeric subexpressions evaluated independently", () => {
        const c = 5
        let r: boolean = c + 1 > 3 && c - 2 < 10
        arktest.assertTrue(r)
    })

    suite.addTest("mixed: false when left subexpression is false", () => {
        const c = 1
        let r: boolean = c > 10 && c * 2 < 8
        arktest.assertTrue(!r)
    })

    suite.addTest("mixed: false when right subexpression is false", () => {
        const c = 5
        let r: boolean = c > 1 && c * 2 < 3
        arktest.assertTrue(!r)
    })

    suite.addTest("mixed with OR: true when one side is true", () => {
        const c = 10
        let r: boolean = c < 1 || c * 2 > 15
        arktest.assertTrue(r)
    })

    suite.addTest("mixed: equality subexpressions", () => {
        const c = 4
        let r: boolean = c * 2 == 8 && c + 1 != 4
        arktest.assertTrue(r)
    })

    suite.addTest("mixed: ternary with numeric subexpressions", () => {
        const c = 3
        const r = c > 1 ? c + 10 : c - 10
        let i: int = r
        arktest.assertEQ(i, 13)
    })

    return suite.run()
}
