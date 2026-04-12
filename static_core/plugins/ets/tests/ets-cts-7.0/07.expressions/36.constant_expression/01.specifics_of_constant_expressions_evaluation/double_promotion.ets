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
    If any of operands are of type double or the operator is exponentiation operator,
    other operands are converted to type double before operator evaluation and the
    result type is double.
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Double promotion in constant expressions")

    suite.addTest("double + int literal promotes to double", () => {
        const r = 3.0 + 5
        let d: double = r
        arktest.assertEQ(d, 8.0)
    })

    suite.addTest("int literal + double promotes to double", () => {
        const r = 5 + 3.0
        let d: double = r
        arktest.assertEQ(d, 8.0)
    })

    suite.addTest("int * double promotes to double", () => {
        const r = 2 * 3.5
        let d: double = r
        arktest.assertEQ(d, 7.0)
    })

    suite.addTest("double - int promotes to double", () => {
        const r = 10.0 - 3
        let d: double = r
        arktest.assertEQ(d, 7.0)
    })

    suite.addTest("double / int promotes to double", () => {
        const r = 15.0 / 3
        let d: double = r
        arktest.assertEQ(d, 5.0)
    })

    suite.addTest("exponentiation of ints produces double", () => {
        const r = 2 ** 3
        let d: double = r
        arktest.assertEQ(d, 8.0)
    })

    suite.addTest("exponentiation with double operand produces double", () => {
        const r = 2.0 ** 3
        let d: double = r
        arktest.assertEQ(d, 8.0)
    })

    suite.addTest("exponentiation int ** double produces double", () => {
        const r = 3 ** 2.0
        let d: double = r
        arktest.assertEQ(d, 9.0)
    })

    suite.addTest("double wins over float in binary operation", () => {
        const r = 3.0f + 5.0
        let d: double = r
        arktest.assertEQ(d, 8.0)
    })

    return suite.run()
}
