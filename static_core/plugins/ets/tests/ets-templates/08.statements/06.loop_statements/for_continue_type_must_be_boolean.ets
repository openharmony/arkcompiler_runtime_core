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
desc: for statement accepts a boolean condition expression
assertion: CI-08-06-02-001-01
tags: []
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('for boolean condition')
    suite.addTest('for_condition_controls_iteration', () => {
        let iterations = 0
        for (let i = 0; i < 5; i++) {
            iterations++
        }
        arktest.assertEQ(iterations, 5)
    })
    suite.addTest('for_condition_can_use_mutated_state', () => {
        let remaining: number = 3
        let iterations = 0
        for (let i: number = 0; remaining > 0; i++) {
            iterations++
            remaining--
        }
        arktest.assertEQ(iterations, 3)
        arktest.assertEQ(remaining, 0)
    })
    return suite.run()
}
