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
    Floating-point calculations must not be naively reordered because they
    are unlikely to be computationally associative. The order of evaluation
    is indicated by parentheses and implicitly by the precedence of operators.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Floating-point evaluation order")

    suite.addTest("left-to-right float addition preserves precision", () => {
        let a: double = 1e308
        let b: double = -1e308
        let c: double = 1.0
        let result: double = a + b + c
        arktest.assertEQ(result, 1.0)
    })

    suite.addTest("parentheses can change float evaluation result", () => {
        let a: double = 1e308
        let b: double = -1e308
        let c: double = 1.0
        let result: double = a + (b + c)
        arktest.assertEQ(result, 0.0)
    })

    suite.addTest("integer addition is associative", () => {
        let a: int = 1000000000
        let b: int = -1000000000
        let c: int = 7
        arktest.assertEQ(a + b + c, 7)
        arktest.assertEQ(a + (b + c), 7)
    })

    suite.addTest("infinity follows left-to-right evaluation", () => {
        let inf: double = 1.0 / 0.0
        let a: double = 1e308
        let result: double = inf - inf + a
        arktest.assertTrue(result != result)
    })

    suite.addTest("parentheses change infinity result", () => {
        let inf: double = 1.0 / 0.0
        let a: double = 1e308
        let result: double = inf - (inf + a)
        arktest.assertTrue(result != result)
    })

    suite.addTest("float multiplication is not reordered", () => {
        let a: double = 1e200
        let b: double = 1e200
        let c: double = 1e-200
        let left_to_right: double = a * b * c
        let right_grouped: double = a * (b * c)
        arktest.assertTrue(left_to_right != right_grouped)
    })

    return suite.run()
}
