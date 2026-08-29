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
desc: if condition false executes else statement
assertion: CI-08-05-003-01
tags: []
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite('if else execution');
    suite.addTest('else_branch_executes_when_false', () => {
        let executed = false;
        if (false) {
            executed = false;
        } else {
            executed = true;
        }
        arktest.assertTrue(executed, 'else branch should execute when condition is false');
    });
    return suite.run();
}
