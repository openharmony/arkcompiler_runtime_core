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
desc: while and do statements accept boolean condition expressions
assertion: CI-08-06-01-001-01
tags: []
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('while and do boolean conditions')
    suite.addTest('while_checks_boolean_condition', () => {
        let count: number = 5
        let iterations = 0
        while (count > 0) {
            count--
            iterations++
        }
        arktest.assertEQ(iterations, 5)
        arktest.assertEQ(count, 0)
    })
    suite.addTest('do_executes_before_boolean_condition_check', () => {
        let count: number = 5
        let iterations = 0
        do {
            count++
            iterations++
        } while (count < 5)
        arktest.assertEQ(iterations, 1)
        arktest.assertEQ(count, 6)
    })
    suite.addTest('while_accepts_boolean_member_expression', () => {
        let text: string = 'test'
        while (text.length > 0) {
            text = ''
        }
        arktest.assertEQ(text.length, 0)
    })
    return suite.run()
}
