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
desc: Integer declarations behave as value types and preserve independent values.
assertion: GAP-03-0013 / GRC-03-0030 / CI-03-06-001-01
---*/
// TEST-UNIT-03-0007; EV-03-06-001-01-01 is extended with an independence oracle.
function main(): int {
    let suite = new arktest.ArkTestsuite('integer value type semantics');
    suite.addTest('integer_values_are_independent', () => {
        let first: int = 41;
        let second: int = first;
        second += 1;
        arktest.assertEQ(first, 41, 'copying an integer must preserve the original value');
        arktest.assertEQ(second, 42, 'the copied integer must be independently mutable');
    });
    return suite.run();
}
