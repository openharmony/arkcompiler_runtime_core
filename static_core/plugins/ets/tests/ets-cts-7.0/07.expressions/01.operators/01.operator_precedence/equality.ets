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
desc: equality and inequality operators precedence with other operators
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Equality operator precedence")

    suite.addTest("equality has lower precedence than additive", () => {
        arktest.assertTrue(1 + 1 == 2)
        arktest.assertFalse(1 + 1 != 2)
    })

    suite.addTest("equality has lower precedence than shift", () => {
        arktest.assertTrue(1 << 3 == 8)
        arktest.assertFalse(1 << 3 != 8)
    })

    suite.addTest("equality has lower precedence than relational", () => {
        arktest.assertTrue(3 < 5 == 4 < 6)
        arktest.assertTrue(3 < 5 != 4 > 6)
    })

    suite.addTest("equality has higher precedence than logical AND", () => {
        arktest.assertFalse(true != false && false)
    })

    suite.addTest("equality has higher precedence than logical OR", () => {
        arktest.assertTrue(false == true || true)
    })

    suite.addTest("equality is left-to-right associative", () => {
        arktest.assertFalse(true == false != false)
    })

    suite.addTest("inequality with multiplicative", () => {
        arktest.assertTrue(3 * 4 != 10)
        arktest.assertFalse(3 * 4 != 12)
    })

    return suite.run()
}
