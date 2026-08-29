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
desc: while evaluates expression before executing statement
assertion: CI-08-06-01-002-01
tags: []
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('while condition first');
    suite.addTest('while_checks_before_execution', () => {
        let executed = false;
        while (false) {
            executed = true;
        }
        arktest.assertFalse(executed, 'while should check condition before executing body');
    });
    return suite.run();
}
