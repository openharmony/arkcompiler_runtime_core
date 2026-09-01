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
desc: if statement accepts boolean expressions as conditions
assertion: CI-08-05-001-01
tags: []
---*/

function getObjectOrNull(): Object | null {
    return {}
}

function main(): int {
    let suite = new arktest.ArkTestsuite('if boolean conditions')
    suite.addTest('comparison_condition_selects_then', () => {
        let numValue: number = 5
        let selected = false
        if (numValue > 0) {
            selected = true
        }
        arktest.assertTrue(selected)
    })
    suite.addTest('relational_condition_selects_then', () => {
        let stringValue: string = 'hello'
        let selected = false
        if (stringValue.length > 0) {
            selected = true
        }
        arktest.assertTrue(selected)
    })
    suite.addTest('equality_condition_selects_then', () => {
        let objValue: Object | null = getObjectOrNull()
        let selected = false
        if (objValue != null) {
            selected = true
        }
        arktest.assertTrue(selected)
    })
    return suite.run()
}
